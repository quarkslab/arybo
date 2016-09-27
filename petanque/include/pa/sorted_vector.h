// Copyright (c) 2016 Adrien Guinet <adrien@guinet.me>
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the <organization> nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PETANQUE_SORTED_VEC_H
#define PETANQUE_SORTED_VEC_H

#include <cassert>
#include <algorithm>
#include <tuple>

#include <pa/config.h>
#ifdef PA_USE_TBB
#include <tbb/parallel_sort.h>
#endif

namespace pa {

template <class V, int LinLimit = 8>
class SortedVector
{
public:
	typedef V vector_type;
	typedef typename vector_type::value_type value_type;
	typedef typename vector_type::difference_type difference_type;
	typedef typename vector_type::reference reference;
	typedef typename vector_type::const_reference const_reference;
	typedef typename vector_type::size_type size_type;
	typedef typename vector_type::allocator_type allocator_type;
	typedef typename vector_type::pointer pointer;
	typedef typename vector_type::const_pointer const_pointer;
	typedef typename vector_type::iterator iterator;
	typedef typename vector_type::const_iterator const_iterator;
	typedef typename vector_type::reverse_iterator reverse_iterator;
	typedef typename vector_type::const_reverse_iterator const_reverse_iterator;

public:
	SortedVector()
	{ }

	SortedVector(SortedVector const& o):
		_v(o.vec())
	{ }

	SortedVector(SortedVector&& o):
		_v(std::move(o.vec()))
	{ }

	template <class... Args>
	SortedVector(bool already_sorted, Args&& ... args):
		_v(std::forward<Args>(args)...)
	{
		if (!already_sorted) {
			sort();
		}
	}

public:
	const_iterator begin() const { return _v.begin(); }
	const_iterator cbegin() const { return _v.cbegin(); }
	const_iterator end() const { return _v.end(); }
	const_iterator cend() const { return _v.cend(); }
	const_reverse_iterator rbegin() const { return _v.rbegin(); }
	const_reverse_iterator crbegin() const { return _v.crbegin(); }
	const_reverse_iterator rend() const { return _v.rend(); }
	const_reverse_iterator crend() const { return _v.crend(); }

	vector_type const& vec() const { return _v; }

public:
	size_type size() const { return _v.size(); }
	void shrink_to_fit() { _v.shrink_to_fit(); }

	SortedVector& operator=(SortedVector const& o) { vec() = o.vec(); return *this; }
	SortedVector& operator=(SortedVector&& o) { vec() = std::move(o.vec()); return *this; }

public:
	iterator begin() { return _v.begin(); }
	iterator end() { return _v.end(); }
	reverse_iterator rbegin() { return _v.rbegin(); }
	reverse_iterator rend() { return _v.rend(); }

public:
	void reserve(const size_type n) { vec().reserve(n); }
	void resize(const size_type n) { vec().resize(n); }
	void resize(const size_type n, const_reference value) { vec().resize(n, value); }

	void unique()
	{
		iterator const new_end = std::unique(begin(), end());
		resize(std::distance(begin(), new_end));
	}

public:
	std::pair<const_iterator,bool> insert(value_type&& v) { return emplace(std::move(v), v); }
	std::pair<const_iterator,bool> insert(value_type const& v) { return emplace(v, v); }

	template <class Iterator, class Func>
	void insert(Iterator begin, Iterator end, Func OnExisting)
	{
		assert(std::is_sorted(cbegin(), cend()));
		assert(std::is_sorted(begin, end));
		const_iterator it_ins = cbegin();
		for (; begin != end; ++begin) {
			if (it_ins == cend()) {
				vec().insert(it_ins, begin, end);
				break;
			}

			if (*begin == *it_ins) {
				it_ins = OnExisting(*this, it_ins);
				continue;
			}
			if (*it_ins < *begin) {
				auto lb = lower_bound_(*begin, it_ins);
				it_ins = lb.first;
				if (lb.second || ((it_ins != cend()) && (*begin == *it_ins))) {
					it_ins = OnExisting(*this, it_ins);
					continue;
				}
			}
			it_ins = vec().emplace(it_ins, *begin);
			++it_ins;
		}
	}

	template <class Iterator>
	void insert(Iterator begin, Iterator end)
	{
		insert(begin, end, [](SortedVector&, const_iterator it) { return it; });
	}

	template <class Iterator, class Func>
	void insert_move(Iterator begin, Iterator end, Func OnExisting)
	{
		assert(std::is_sorted(cbegin(), cend()));
		assert(std::is_sorted(begin, end));
		const_iterator it_ins = cbegin();
		for (; begin != end; ++begin) {
			if (it_ins == cend()) {
				for (; begin != end; ++begin) {
					vec().emplace_back(std::move(*begin));
				}
				break;
			}

			if (*begin == *it_ins) {
				it_ins = OnExisting(*this, it_ins);
				continue;
			}
			if (*it_ins < *begin) {
				auto lb = lower_bound_(*begin, it_ins);
				it_ins = lb.first;
				if (lb.second || ((it_ins != cend()) && (*begin == *it_ins))) {
					it_ins = OnExisting(*this, it_ins);
					continue;
				}
			}
			it_ins = vec().emplace(it_ins, std::move(*begin));
			++it_ins;
		}
	}

	template <class Iterator>
	void insert_move(Iterator begin, Iterator end)
	{
		insert_move(begin, end, [](SortedVector&, const_iterator it) { return it; });
	}

	void insert(SortedVector const& v)
	{
		insert(v.cbegin(), v.cend());
	}

	template <class Func>
	void insert(SortedVector const& v, Func OnExisting)
	{
		insert(v.cbegin(), v.cend(), OnExisting);
	}

	void insert_move(SortedVector&& v)
	{
		insert_move(v.begin(), v.end());
	}

	template <class Func>
	void insert_move(SortedVector&& v, Func OnExisting)
	{
		insert_move(v.begin(), v.end(), OnExisting);
	}

	template <class Iterator>
	void insert_unsorted(Iterator begin, Iterator end)
	{
		for (; begin != end; ++begin) {
			insert(*begin);
		}
	}

	void insert_dup(value_type&& v) { emplace_dup(std::move(v), v); }
	void insert_dup(value_type const& v) { emplace_dup(v, v); }

	template <class Iterator>
	void insert_dup(Iterator begin, Iterator end)
	{
		assert(std::is_sorted(cbegin(), cend()));
		assert(std::is_sorted(begin, end));
		const_iterator it_ins = cbegin();
		for (; begin != end; ++begin) {
			if (it_ins == cend()) {
				vec().insert(it_ins, begin, end);
				break;
			}
			if (*it_ins < *begin) {
				auto lb = lower_bound_(*begin, it_ins);
				it_ins = lb.first;
			}
			it_ins = vec().emplace(it_ins, *begin);
			++it_ins;
		}
	}

	template <class Iterator>
	void insert_dup_move(Iterator begin, Iterator end)
	{
		assert(std::is_sorted(cbegin(), cend()));
		assert(std::is_sorted(begin, end));
		const_iterator it_ins = cbegin();
		for (; begin != end; ++begin) {
			if (it_ins != cend()) {
				for (; begin != end; ++begin) {
					vec().emplace_back(std::move(*begin));
				}
				break;
			}
			if (*it_ins < *begin) {
				auto lb = lower_bound_(*begin, it_ins);
				it_ins = lb.first;
			}
			it_ins = vec().emplace(it_ins, std::move(*begin));
			++it_ins;
		}
	}

	void insert_dup(SortedVector const& v)
	{
		insert_dup(v.cbegin(), v.cend());
	}

	void insert_dup(SortedVector&& v)
	{
		insert_dup_move(v.begin(), v.end());
	}

public:
	const_iterator find(value_type const& v) const
	{
		return find(v, cbegin());
	}

	const_iterator find(value_type const& v, const_iterator start) const
	{
		auto lb = lower_bound_(v, start);
		const_iterator it = lb.first;
		if ((lb.second == true) || ((it != cend()) && (*it == v))) {
			return it;
		}
		return cend();
	}

	const_iterator lower_bound(value_type const& v, const_iterator start) const
	{
		return lower_bound_(v, start).first;
	}

	const_iterator lower_bound(value_type const& v) const
	{
		return lower_bound(v, cbegin());
	}

public:
	reference operator[](const size_type n) { return vec()[n]; }
	const_reference operator[](const size_type n) const { return vec()[n]; }

	reference front() { return vec().front(); }
	const_reference front() const { return vec().front(); }
	reference back() { return vec().back(); }
	const_reference back() const { return vec().back(); }

	iterator erase(const_iterator it) { return vec().erase(it); }
	iterator erase(const_iterator first, const_iterator last) { return vec().erase(first, last); }

public:
	bool operator==(SortedVector const& o) const { return vec() == o.vec(); }
	bool operator!=(SortedVector const& o) const { return vec() != o.vec(); }

	bool operator==(vector_type const& o) const { return vec() == o; }
	bool operator!=(vector_type const& o) const { return vec() != o; }
private:
	// Generic function that supports insertion by copy or by moving objects
	// returns true iff an element has been inserted
	template <class C>
	std::pair<const_iterator,bool> emplace(C&& construct, value_type const& v)
	{
		auto lb = lower_bound_(v);
		if (lb.second) {
			return std::make_pair(lb.first,false);
		}
		const_iterator it = lb.first;
		if ((it != cend()) && (*it == v)) {
			return std::make_pair(it,false);
		}
		it = vec().emplace(it, std::forward<C>(construct));
		return std::make_pair(it,true);
	}

	template <class C>
	void emplace_dup(C&& construct, value_type const& v)
	{
		auto lb = lower_bound_(v);
		vec().emplace(lb.first, std::forward<C>(construct));
	}

protected:
	std::pair<const_iterator,bool> lower_bound_(value_type const& v, const_iterator istart) const
	{
		if (istart == end()) {
			return std::make_pair(istart,false);
		}
		if (v == *istart) {
			return std::make_pair(istart,true);
		}
		// Fast path for insertion at the beggining
		if (v < *istart) {
			return std::make_pair(istart,false);
		}
		const_iterator iend;
		if ((istart != begin()) && (v < *istart)) {
			iend = istart;
			istart = begin();
		}
		else {
			iend = end();
		}
		while (std::distance(istart, iend) > LinLimit) {
			const_iterator const middle = istart+(std::distance(istart,iend)/2);
			if (v == *middle) {
				return std::make_pair(middle, true);
			}
			if (v < *middle) {
				iend = middle;
			}
			else {
				istart = middle;
			}
		}
		while ((istart < cend()) &&
				(*istart < v)) {
			++istart;
		}
		return std::make_pair(istart,false);
	}

	std::pair<const_iterator,bool> lower_bound_(value_type const& v) const
	{
		return lower_bound_(v, begin());
	}

public:
	void sort()
	{
#ifdef PA_USE_TBB
		tbb::parallel_sort(begin(), end());
#else
		std::sort(begin(), end());
#endif
	}

private:
	vector_type& vec() { return _v; }

private:
	vector_type _v;
};

} // pa

#endif
