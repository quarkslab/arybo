// This file is part of pector (https://github.com/aguinet/pector).
// Copyright (C) 2014-2015 Adrien Guinet <adrien@guinet.me> 
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA


#ifndef PECTOR_PECTOR_H
#define PECTOR_PECTOR_H

#include <memory>
#include <type_traits>
#include <limits>
#include <cstring>
#include <cassert>
#include <iterator>

#include <pector/enhanced_allocators.h>
#include <pector/recommended_size.h>
#include <pector/pector_internals.h>

namespace pt {

template <class T, class Alloc = std::allocator<T>, class SizeType = size_t, class RecommendedSize = default_recommended_size, bool check_size_overflow = true>
class pector
{
public:
	typedef T value_type;
	typedef typename Alloc::template rebind<T>::other allocator_type;
	typedef SizeType size_type;
	typedef std::ptrdiff_t difference_type;
	typedef value_type& reference;
	typedef value_type const& const_reference;
	typedef RecommendedSize recommended_size_type;
	static constexpr bool is_pod = std::is_pod<value_type>::value;

	static_assert(std::numeric_limits<size_type>::is_signed == false, "SizeType must be an unsigned integer type!");

private:
	typedef internals::pector_storage<value_type, Alloc, allocator_type, size_type, is_pod, recommended_size_type, check_size_overflow> storage_type;

public:
	typedef typename storage_type::pointer pointer;
	typedef typename storage_type::const_pointer const_pointer;

public:
	typedef pointer iterator;
	typedef const_pointer  const_iterator;
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

public:
	pector() { }

	pector(allocator_type const& alloc): 
		_storage(alloc)
	{ }

	pector(pector const& o):
		_storage(o._storage)
	{ }

	pector(pector&& o):
		_storage(std::move(o._storage))
	{ }

	pector(std::initializer_list<value_type> const& il)
	{
		_storage.force_allocate(il.size());
		_storage.force_size(il.size());
		size_type i = 0;
		for (value_type const& v: il) {
			_storage.construct(_storage.at(i), v);
			i++;
		}
	}

public:
	void reserve(const size_type n)
	{
		_storage.allocate_if_needed(n);
	}

	void resize(const size_type n)
	{
		size_type old_size = size();
		_storage.allocate_if_needed(n);
		if (n > old_size) {
			_storage.construct(_storage.begin() + old_size, _storage.begin() + n);
		}
		_storage.set_size(n);
	}

	void resize(const size_type n, value_type const& v)
	{
		size_type old_size = size();
		_storage.allocate_if_needed(n);
		if (n > old_size) {
			_storage.construct(_storage.begin() + old_size, _storage.begin() + n, v);
		}
		_storage.set_size(n);
	}

	void reserve_fit(const size_type n)
	{
		_storage.force_allocate(n);
	}

	void resize_no_construct(const size_type n)
	{
		_storage.allocate_if_needed(n);
		_storage.set_size(n);
	}

	void resize_fit(const size_type n)
	{
		size_type old_size = size();
		_storage.force_allocate(n);
		if (n > old_size) {
			_storage.construct(_storage.begin() + old_size, _storage.begin() + n);
		}
		_storage.set_size(n);
	}

	void shrink_to_fit() { _storage.shrink_to_fit(); }

	size_type size() const { return _storage.size(); }
	size_type max_size() const
	{
		return _storage.max_size();
	}

	size_type capacity() const { return _storage.storage_size(); }
	bool empty() const { return _storage.size() == 0; }

	template <class InputIterator>
	void assign(InputIterator first, InputIterator last)
	{
		const size_type n = std::distance(first, last);
		clear();
		_storage.allocate_if_needed(n);
		size_type i = 0;
		for (; first != last; ++first) {
			_storage.construct_copy_from_iterator(_storage.at(i), first);
			i++;
		}
		_storage.set_size(n);
	}

	void assign(std::initializer_list<value_type> const& li)
	{
		assign(li.begin(), li.end());
	}

	void assign(size_type const n, value_type const& v)
	{
		clear();
		_storage.allocate_if_needed(n);
		_storage.set_size(n);
		_storage.construct(_storage.begin(), _storage.end(), v);
	}
	
	void push_back(value_type const& v)
	{
		const size_type new_size = _storage.grow_if_needed(1);
		_storage.construct(_storage.end(), v);
		_storage.set_size(new_size);
	}

	void pop_back()
	{
		_storage.destroy(_storage.last());
		_storage.force_size(size()-1);
	}

	void swap(pector& x)
	{
		std::swap(_storage, x._storage);
	}

	void clear()
	{
		_storage.clear();
	}

	template <class... Args>
	void emplace_back(Args&& ... args)
	{
		const size_type new_size = _storage.grow_if_needed(1);
		//_storage.allocate_if_needed(1);
		_storage.construct_args(_storage.end(), std::forward<Args>(args)...);
		_storage.force_size(new_size);
	}


	template< class InputIt>
	iterator insert(typename std::enable_if<std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>::value, const_iterator>::type pos, InputIt first, InputIt last)
	{
		return insert_gen(pos, std::distance(first, last),
			[&first, this](pointer start, pointer end)
			{
				for (; start != end; start++) {
					this->_storage.construct_copy_from_iterator(start, first++);
				}
			});
	}

	iterator insert(const_iterator pos, size_type const count, value_type const& v)
	{
		return insert_gen(pos, count,
			[this, &v](pointer start, pointer end)
			{
				this->_storage.construct(start, end, v);
			});
	}

	template<class... Args>
	iterator emplace(const_iterator pos, Args&& ... args)
	{
#if (__GNUC__ == 4 && __GNUC_MINOR__ >= 9) || __GNUC__ >= 5
		return insert_gen(pos, 1,
			[this, &args...](pointer start, pointer)
			{
				this->_storage.construct_args(start, std::forward<Args>(args)...);
			});
#else
		// AG: lambda with variadic arguments only works starting with GCC 4.9.
		// Copy/paste the code otherwise... :/
		assert(pos <= cend());
		const size_type npos = std::distance(cbegin(), pos);
		const size_type new_size = compute_size_grow(1);
		pointer buf;
		size_type alloc_size = 0;
		const size_type cap = _storage.storage_size();
		if (_storage.begin() == nullptr ||
			cap < new_size) {
			alloc_size = recommended_size_type::recommended(max_size(), cap, new_size);
			buf = _storage.allocate_buffer(alloc_size);
			_storage.construct_move(buf, _storage.begin(), _storage.begin()+npos);
		}
		else {
			buf = _storage.begin();
		}
		// Make room for the new objects
		_storage.construct_move_alias_reverse(buf+npos+1, _storage.begin()+npos, _storage.end());

		// Insert objects
		_storage.construct_args(&buf[npos], std::forward<Args>(args)...);

		if (alloc_size > 0) {
			_storage.destroy(_storage.begin(), _storage.end());
			_storage.deallocate(_storage.begin(), _storage.storage_size());
			_storage.set_buffer(buf, alloc_size);
		}
		_storage.force_size(new_size);
		return iterator(_storage.begin()+npos);
#endif
	}

	iterator insert(const_iterator pos, value_type&& v)
	{
		return emplace(pos, std::move(v));
	}

	iterator insert(const_iterator pos, value_type const& v)
	{
		return insert(pos, 1, v);
	}

	iterator insert(const_iterator pos, std::initializer_list<value_type> const& il)
	{
		return insert(pos, il.begin(), il.end());
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		assert(first <= last);
		assert(last <= cend());
		assert(first < cend());
		if (first == last) {
			return begin() + std::distance(cbegin(), first);
		}
		_storage.move_alias(begin() + std::distance(cbegin(), first),
                        begin() + std::distance(cbegin(), last),
                        end());
		_storage.set_size(size() - std::distance(first,last));
		return iterator(first);
	}

	inline iterator erase(const_iterator pos)
	{
		assert(pos < cend());
		return erase(pos, pos+1);
	}

	allocator_type get_allocator() const { return _storage.allocator(); }

	reference at(const size_type i)
	{
		return *_storage.at(i);
	}

	const_reference at(const size_type i) const
	{
		return *_storage.at(i);
	}

	reference operator[](const size_type i) { return at(i); }
	const_reference operator[](const size_type i) const { return at(i); }

	reference front() { return *_storage.begin(); }
	const_reference front() const { return *_storage.begin(); }

	reference back() { return *_storage.last(); }
	const_reference back() const { return *_storage.last(); }

	pointer data() noexcept { return _storage.begin(); }
	const_pointer data() const noexcept { return _storage.begin(); }

	iterator begin() { return iterator(_storage.begin()); }
	iterator end()   { return iterator(_storage.end()); }

	const_iterator cbegin() { return const_iterator(_storage.begin()); }
	const_iterator cend()   { return const_iterator(_storage.end()); }

	const_iterator begin() const { return const_iterator(_storage.begin()); }
	const_iterator end()   const { return const_iterator(_storage.end()); }

	const_iterator cbegin() const { return const_iterator(_storage.begin()); }
	const_iterator cend()   const { return const_iterator(_storage.end()); }

	reverse_iterator rbegin() { return reverse_iterator(end()); }
	reverse_iterator rend()   { return reverse_iterator(begin()); }

	const_reverse_iterator crbegin() { return const_reverse_iterator(end()); }
	const_reverse_iterator crend()   { return const_reverse_iterator(begin()); }

	const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
	const_reverse_iterator rend()   const { return const_reverse_iterator(begin()); }

	const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
	const_reverse_iterator crend()   const { return const_reverse_iterator(begin()); }

public:
	inline bool operator==(pector const& o) const
	{
		return storage_type::equals(_storage.begin(), size(), o._storage.begin(), o.size());
	}

	inline bool operator<(pector const& o) const
	{
		return std::lexicographical_compare(begin(), end(), o.begin(), o.end());
	}

public:
	pector& operator=(pector const& o)
	{
		if (&o != this) {
			_storage = o._storage;
		}
		return *this;
	}

	pector& operator=(pector&& o)
	{
		if (&o != this) {
			_storage = std::move(o._storage);
		}
		return *this;
	}

private:
	template <class F>
	inline iterator insert_gen(const_iterator pos, size_type const count, F const& fins)
	{
		assert(pos <= cend());
		if (count == 0) {
			return iterator(pos);
		}
		const size_type npos = std::distance(cbegin(), pos);
		const size_type new_size = compute_size_grow(count);
		pointer buf;
		size_type alloc_size = 0;
		const size_type cap = _storage.storage_size();
		if (_storage.begin() == nullptr ||
			cap < new_size) {
			alloc_size = recommended_size_type::recommended(max_size(), cap, new_size);
			buf = _storage.allocate_buffer(alloc_size);
			_storage.construct_move(buf, _storage.begin(), _storage.begin()+npos);
		}
		else {
			buf = _storage.begin();
		}
		// Make room for the new objects
		_storage.construct_move_alias_reverse(buf+npos+count, _storage.begin()+npos, _storage.end());

		// Insert objects
		fins(&buf[npos], &buf[npos+count]);

		if (alloc_size > 0) {
			_storage.destroy(_storage.begin(), _storage.end());
			_storage.deallocate(_storage.begin(), _storage.storage_size());
			_storage.set_buffer(buf, alloc_size);
		}
		_storage.force_size(new_size);
		return iterator(_storage.begin()+npos);
	}

	inline size_type compute_size_grow(size_type const grow_by) const
	{
		return _storage.compute_size_grow(grow_by);
	}

private:
	storage_type _storage;
};

} // pector

namespace std {

template <class T, class Alloc, class SizeType, class RS, bool co> 
void swap(pt::pector<T, Alloc, SizeType, RS, co>& a, pt::pector<T, Alloc, SizeType, RS, co>& b)
{
	a.swap(b);
}

} // std

#endif
