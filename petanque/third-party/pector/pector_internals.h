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


#ifndef PECTOR_INTERNALS_H
#define PECTOR_INTERNALS_H

#include <algorithm>

namespace pt {

template <class T, class Alloc, class SizeType, class RecommendedSize, bool check_size_overflow>
class pector;

namespace internals {

template <class Alloc, class SizeType, bool is_size_aware>
struct size_storage_impl;

template <class Alloc, class SizeType>
struct size_storage_impl<Alloc, SizeType, false>
{
	typedef typename std::allocator_traits<Alloc>::const_pointer const_pointer;
	typedef SizeType size_type;

public:
	size_storage_impl():
		_n(0),
		_alloc(0)
	{ }

	size_storage_impl(size_storage_impl const& o):
		_n(o._n),
		_alloc(o._alloc)
	{ }

	size_storage_impl(size_type const n, size_type const alloc):
		_n(n),
		_alloc(alloc)
	{ }

public:
	void set_size(size_type const n) { _n = n; }
	void set_storage_size(size_type const n) { _alloc = n; }
	void set_zero()
	{
		_n = 0;
		_alloc = 0;
	}

	size_type size() const { return _n; }
	size_type storage_size(Alloc const&, const_pointer) const { return _alloc; }

private:
	size_type _n;
	size_type _alloc;
};

template <class Alloc, class SizeType>
struct size_storage_impl<Alloc, SizeType, true>
{
	typedef typename std::allocator_traits<Alloc>::const_pointer const_pointer;
	typedef SizeType size_type;

public:
	size_storage_impl():
		_n(0)
	{ }

	size_storage_impl(size_storage_impl const& o):
		_n(o._n)
	{ }

	size_storage_impl(size_type const n, size_type const):
		_n(n)
	{ }

public:
	void set_size(size_type const n) { _n = n; }
	void set_storage_size(size_type const) { }
	void set_zero() { _n = 0; }

	size_type size() const { return _n; }
	size_type storage_size(Alloc const& a, const_pointer p) const { return a.usable_size(p); }

private:
	size_type _n;
};

template <class Alloc, class SizeType>
using size_storage = size_storage_impl<Alloc, SizeType, is_size_aware_allocator<Alloc>::value>;

template <bool is_reallocable>
struct pod_reallocate_impl;

template <>
struct pod_reallocate_impl<true>
{
	template <class PS, class Pointer, class SizeType>
	static inline Pointer reallocate(PS& ps, Pointer buf, const SizeType, const SizeType, const SizeType new_size)
	{
		return ps.allocator().realloc(buf, new_size);
	}
};

template <>
struct pod_reallocate_impl<false>
{
	template <class PS, class Pointer, class SizeType>
	static inline Pointer reallocate(PS& ps, Pointer buf, const SizeType old_size, const SizeType nobjs, const SizeType new_size)
	{
		return static_cast<typename PS::base_type&>(ps).reallocate(buf, old_size, nobjs, new_size);
	}
};


template <class T, class AllocOrg, class Alloc, class SizeType, class RecommendedSize, bool check_size_overflow, class Storage>
class pector_storage_base: public Alloc
{
	friend class pector<T, AllocOrg, SizeType, RecommendedSize, check_size_overflow>;

	template <bool IR>
	friend struct pod_reallocate_impl;

protected:
	typedef Storage storage_type;
	typedef Alloc allocator_type;
	typedef T value_type;
	typedef typename std::allocator_traits<allocator_type>::pointer pointer;
	typedef typename std::allocator_traits<allocator_type>::const_pointer const_pointer;
	typedef SizeType size_type;
	typedef RecommendedSize recommended_size_type;

protected:
	pector_storage_base(allocator_type const& alloc = allocator_type()):
		allocator_type(alloc),
		_buf(nullptr),
		_sizes(0, 0)
	{
	};

	pector_storage_base(pector_storage_base const& o, allocator_type const& alloc = allocator_type()):
		Alloc(alloc),
		_buf(nullptr)
	{
		force_allocate(o.size());
		force_size(o.size());
		storage_().construct_copy(begin(), o.begin(), size());
	}

	pector_storage_base(pector_storage_base&& o, allocator_type const& alloc = allocator_type()):
		Alloc(alloc),
		_buf(o._buf),
		_sizes(o._sizes)
	{
		o._buf = nullptr;
		o._sizes.set_zero();
	}

	~pector_storage_base()
	{
		free();
	}

public:
	storage_type& storage_() { return static_cast<storage_type&>(*this); }

public:
	pector_storage_base& operator=(pector_storage_base const& o)
	{
		// self-assignment already checked!
		allocate_if_needed(o.size());
		storage_().copy(begin(), o.begin(), size());
		storage_().construct_copy(at(size()), o.at(size()), o.size()-size());
		force_size(o.size());
		return *this;
	}

	pector_storage_base& operator=(pector_storage_base&& o)
	{
		// self-assignment already checked!
		free();
		_buf = o._buf;
		_sizes = o._sizes;
		o._buf = nullptr;
		o._sizes.set_zero();
		return *this;
	}

protected:
	allocator_type allocator()
	{
		return static_cast<allocator_type&>(*this);
	}

	allocator_type const& allocator() const
	{
		return static_cast<allocator_type const&>(*this);
	}

	void allocate_if_needed(const size_type n)
	{
		if (n <= storage_size()) {
			return;
		}
		force_allocate(n);
	}

	inline size_type compute_size_grow(size_type const grow_by) const
	{
		const size_type size_ = size();
		if (check_size_overflow && (size_ > (max_size()-grow_by))) {
			throw std::length_error("size will overflow");
		}
		return size_+grow_by;
	}

	size_type max_size() const
	{
		return std::min((size_t)allocator().max_size(), (size_t)std::numeric_limits<size_type>::max());
	}

	size_type grow_if_needed(const size_type grow_by)
	{
		const size_type n = compute_size_grow(grow_by);
		const size_type cap = storage_size();
		if (n <= cap) {
			return n;
		}
		const size_type new_alloc = recommended_size_type::recommended(max_size(), cap, n);
		force_allocate(new_alloc);
		return n;
	}

	inline pointer allocate_buffer(const size_type n)
	{
		return allocator().allocate(n);
	}
	
	void force_allocate(const size_type n)
	{
		if (begin() == nullptr) {
			_buf = allocate_buffer(n);
			force_size(0);
		}
		else {
			_buf = storage_().reallocate(_buf, storage_size(), size(), n);
			force_size(std::min(n, size()));
		}

		set_storage_size(n);
	}

	void clear()
	{
		storage_().destroy(begin(), end());
		force_size(0);
	}

	bool shrink_to_fit()
	{
		if (size() == storage_size()) {
			return false;
		}

		force_allocate(size());
		return true;
	}

	void free()
	{
		if (begin()) {
			storage_().destroy(begin(), end());
			deallocate(begin(), storage_size());
		}
	}

	void set_buffer(pointer buf, const size_type size)
	{
		_buf = buf;
		set_storage_size(size);
	}

	inline void deallocate(pointer begin, size_type const n)
	{
		allocator().deallocate(begin, n);
	}

	pointer reallocate(pointer buf, const size_type old_size, const size_type nobjs, const size_type new_size)
	{
		pointer new_buf = allocate_buffer(new_size);
		size_type nmov;
		if (new_size < nobjs) {
			storage_().destroy(buf + new_size, buf + nobjs);
			nmov = new_size;
		}
		else {
			nmov = nobjs;
		}
		storage_().construct_move(new_buf, buf, nmov);
		allocator().deallocate(buf, old_size);
		return new_buf;
	}

	void set_size(size_type const n)
	{
		if (n < size()) {
			storage_().destroy(at(n), at(size()));
		}
		force_size(n);
	}

	void force_size(size_type const n)
	{
		_sizes.set_size(n);
	}

	template <class... Args>
	void construct_args(pointer p, Args&& ... args)
	{
		allocator().construct(p, std::forward<Args>(args)...);
	}

protected:
	inline pointer begin() { return _buf; }
	inline const_pointer begin() const { return _buf; }

	inline pointer at(const size_type i)
	{
		assert(i <= storage_size());
		return begin()+i;
	}
	inline const_pointer at(const size_type i) const
	{
		assert(i <= storage_size());
		return begin()+i;
	}

	inline pointer last() { return end()-1; }
	inline const_pointer last() const { return end()-1; }

	inline pointer end() { return _buf+size(); }
	inline const_pointer end() const { return _buf+size(); }

	inline pointer end_storage() { return _buf+storage_size(); }
	inline const_pointer end_storage() const { return _buf+storage_size(); }

	inline size_type size() const { return _sizes.size(); }
	inline size_type storage_size() const { return _sizes.storage_size(allocator(), begin()); }

	inline void set_storage_size(size_type const n) { _sizes.set_storage_size(n); }

private:
	pointer _buf;
	size_storage<allocator_type, size_type> _sizes;
};

template <class T, class AllocOrg, class Alloc, class SizeType, bool is_pod, class RecommendedSize, bool check_size_overflow>
class pector_storage;

template <class T, class AllocOrg, class Alloc, class SizeType, class RecommendedSize, bool check_size_overflow>
class pector_storage<T, AllocOrg, Alloc, SizeType, true, RecommendedSize, check_size_overflow>: public pector_storage_base<T, AllocOrg, Alloc, SizeType, RecommendedSize, check_size_overflow, pector_storage<T, AllocOrg, Alloc, SizeType, true, RecommendedSize, check_size_overflow>>
{
	friend class pector<T, AllocOrg, SizeType, RecommendedSize, check_size_overflow>;

	template <bool IR>
	friend struct pod_reallocate_impl;

protected:
	typedef pector_storage_base<T, AllocOrg, Alloc, SizeType, RecommendedSize, check_size_overflow, pector_storage<T, AllocOrg, Alloc, SizeType, true, RecommendedSize, check_size_overflow>> base_type;
	friend base_type;

	using typename base_type::const_pointer;
	using typename base_type::pointer;
	using typename base_type::value_type;
	using typename base_type::size_type;
	// MSVC does not accept using typename base_type::allocator_type here!
	typedef typename base_type::allocator_type allocator_type;

	using base_type::base_type;

	static inline void construct(pointer begin, pointer end, value_type const v = value_type())
	{
		for (; begin != end; ++begin) {
			construct(begin, v);
		}
	}

	static inline void construct(pointer p, value_type const v = value_type())
	{
		*p = v;
	}

	static inline void destroy(pointer, pointer) { }
	static inline void destroy(pointer) { };

	static inline void construct_move(pointer dst, pointer src, const size_type n)
	{
		move(dst, src, n);
	}

	static inline void construct_move_alias_reverse(pointer dst, pointer src, const size_type n)
	{
		move_alias(dst, src, n);
	}

	static inline void move(pointer dst, pointer src, const size_type n)
	{
		copy(dst, src, n);
	}

	static inline void move_alias(pointer dst, pointer src, const size_type n)
	{
		memmove(dst, src, n*sizeof(value_type));
	}

	static inline void construct_copy(pointer dst, const_pointer src, const size_type n)
	{
		copy(dst, src, n);
	}

	static inline void copy(pointer dst, const_pointer src, const size_type n)
	{
		memcpy(dst, src, n*sizeof(value_type));
	}

	template <class InputIterator>
	static inline void construct_copy_from_iterator(pointer p, InputIterator it)
	{
		*p = *it;
	}

	static inline void construct_copy(pointer dst, const_pointer src_begin, const_pointer src_end)
	{
		construct_copy(dst, std::distance(src_begin, src_end));
	}

	static inline void construct_move(pointer dst, pointer src_begin, pointer src_end)
	{
		construct_move(dst, src_begin, std::distance(src_begin, src_end));
	}

	static inline void construct_move_alias_reverse(pointer dst, pointer src_begin, pointer src_end)
	{
		construct_move_alias_reverse(dst, src_begin, std::distance(src_begin, src_end));
	}

	static inline void move(pointer dst, pointer src_begin, pointer src_end)
	{
		move(dst, src_begin, std::distance(src_begin, src_end));
	}

	static inline void move_alias(pointer dst, pointer src_begin, pointer src_end)
	{
		move_alias(dst, src_begin, std::distance(src_begin, src_end));
	}

	static inline void copy(pointer dst, const_pointer src_begin, const_pointer src_end)
	{
		copy(dst, src_begin, std::distance(src_begin, src_end));
	}

	inline pointer reallocate(pointer buf, const size_type old_size, const size_type nobjs, const size_type new_size)
	{
		return pod_reallocate_impl<is_reallocable_allocator<allocator_type>::value>::reallocate(*this, buf, old_size, nobjs, new_size);
	}

	inline static bool equals(const_pointer const start_a, size_type const size_a, const_pointer const start_b, size_type const size_b)
	{
		if (size_a != size_b) {
			return false;
		}
		return memcmp(start_a, start_b, size_a*sizeof(value_type)) == 0;
	}
};

template <class T, class AllocOrg, class Alloc, class SizeType, class RecommendedSize, bool check_size_overflow>
class pector_storage<T, AllocOrg, Alloc, SizeType, false, RecommendedSize, check_size_overflow>: public pector_storage_base<T, AllocOrg, Alloc, SizeType, RecommendedSize, check_size_overflow, pector_storage<T, AllocOrg, Alloc, SizeType, false, RecommendedSize, check_size_overflow>>
{
	friend class pector<T, AllocOrg, SizeType, RecommendedSize, check_size_overflow>;

	typedef pector_storage_base<T, AllocOrg, Alloc, SizeType, RecommendedSize, check_size_overflow, pector_storage<T, AllocOrg, Alloc, SizeType, false, RecommendedSize, check_size_overflow>> base_type;
	friend base_type;

	using typename base_type::const_pointer;
	using typename base_type::pointer;
	using typename base_type::value_type;
	using typename base_type::size_type;

protected:
	typedef Alloc allocator_type;

protected:
	using base_type::base_type;

	inline void construct(pointer begin, pointer end)
	{
		for (; begin != end; ++begin) {
			this->allocator().construct(begin);
		}
	}

	inline void construct(pointer p, value_type const& v)
	{
		this->allocator().construct(p, v);
	}

	inline void construct(pointer p, value_type&& v)
	{
		this->allocator().construct(p, std::move(v));
	}

	inline void construct(pointer begin, pointer end, value_type const& v)
	{
		for (; begin != end; ++begin) {
			construct(begin, v);
		}
	}

	inline void destroy(pointer begin, pointer end)
	{
		for (; end != begin; ++begin) {
			destroy(begin);
		}
	}

	inline void destroy(pointer p)
	{
		this->allocator().destroy(p);
	}

	inline void construct_copy(pointer dst, const_pointer src, const size_type n)
	{
		for (size_type i = 0; i < n; i++) {
			construct(&dst[i], src[i]);
		}
	}

	inline void construct_move(pointer dst, pointer src, const size_type n)
	{
		for (size_type i = 0; i < n; i++) {
			construct(&dst[i], std::move(src[i]));
		}
	}

	void construct_move_alias_reverse(pointer dst, pointer src, const size_type n)
	{
		if (n == 0) {
			return;
		}
		for (size_type i = n-1; i > 0; i--) {
			construct(&dst[i], std::move(src[i]));
		}
		new (&dst[0]) value_type(std::move(src[0]));
	}

	inline static void move(pointer dst, pointer src, const size_type n)
	{
		for (size_type i = 0; i < n; i++) {
			dst[i] = std::move(src[i]);
		}
	}

	inline static void move_alias(pointer dst, pointer src, const size_type n)
	{
		move(dst, src, n);
	}

	inline static void copy(pointer dst, const_pointer src, const size_type n)
	{
		for (size_type i = 0; i < n; i++) {
			dst[i] = src[i];
		}
	}

	template <class InputIterator>
	inline void construct_copy_from_iterator(pointer p, InputIterator it)
	{
		construct(p, *it);
	}

	inline void construct_copy(pointer dst, const_pointer src_begin, const_pointer src_end)
	{
		construct_copy(dst, src_begin, std::distance(src_begin, src_end));
	}

	inline void construct_move(pointer dst, pointer src_begin, pointer src_end)
	{
		construct_move(dst, src_begin, std::distance(src_begin, src_end));
	}

	inline void construct_move_alias_reverse(pointer dst, pointer src_begin, pointer src_end)
	{
		construct_move_alias_reverse(dst, src_begin, std::distance(src_begin, src_end));
	}

	inline void move(pointer dst, pointer src_begin, pointer src_end)
	{
		move(dst, src_begin, std::distance(src_begin, src_end));
	}

	inline void move_alias(pointer dst, pointer src_begin, pointer src_end)
	{
		move_alias(dst, src_begin, std::distance(src_begin, src_end));
	}

	inline void copy(pointer dst, const_pointer src_begin, const_pointer src_end)
	{
		copy(dst, src_begin, std::distance(src_begin, src_end));
	}

	inline static bool equals(const_pointer const start_a, size_type const size_a, const_pointer const start_b, size_type const size_b)
	{
#if defined _MSC_VER || __cplusplus > 201103L
		return std::equal(start_a, start_a+size_a, start_b, start_b+size_b);
#else
		if (size_a != size_b) {
			return false;
		}
		// This is deprecated by MSVC 2015!
		return std::equal(start_a, start_a+size_a, start_b);
#endif
	}
};

} // internals

} // pector

#endif
