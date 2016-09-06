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

#ifndef PETANQUE_ALGOS_H
#define PETANQUE_ALGOS_H

#include <iterator>
#include <vector>
#include <list>

namespace pa {

template <class Array>
auto array_size(Array const& a)
{
	return std::distance(std::begin(a), std::end(a));
}

template <class T, class Alloc>
auto array_size(std::vector<T, Alloc> const& a)
{
	return a.size();
}

template <class T, class Alloc>
auto array_size(std::list<T, Alloc> const& a)
{
	return a.size();
}

template <class Iterator>
Iterator adjacents_find(Iterator begin, Iterator end)
{
	Iterator it_cur = begin;
	Iterator it_check = begin+1;
	while (it_check != end) {
		if (*it_cur != *it_check) {
			break;
		}
		it_check++;
	}
	return it_check;
}

template <class Iterator, class Pred>
Iterator remove_consecutives(Iterator begin, Iterator end, Pred const& should_write)
{
	Iterator it_cur = begin;
	Iterator it_write = begin;
	while (it_cur != end) {
		Iterator it_end_cons = adjacents_find(it_cur, end);
		const size_t d = std::distance(it_cur, it_end_cons);
		if ((d&1) == 1 && should_write(*it_cur)) {
			*it_write = std::move(*it_cur);
			it_write++;
		}
		if (it_end_cons == end) {
			break;
		}
		it_cur = it_end_cons;
	}
	return it_write;
}

template <class Iterator>
Iterator remove_consecutives(Iterator begin, Iterator end)
{
	return remove_consecutives(begin, end, [] (auto const&) { return true; });
}

namespace __impl {

template <typename Int>
static bool plus1(Int k, std::vector<Int>& idxes)
{
	bool is_last = true;
	for (Int i = k+1; i < idxes.size(); i++) {
		if (idxes[i] != (idxes[i-1]+1)) {
			is_last = false;
			break;
		}
	}
	if (is_last) {
		if (k == 0) {
			return false;
		}
		return plus1(k-1, idxes);
	}
	idxes[k]++;
	for (Int i = k+1; i < idxes.size(); i++) {
		idxes[i] = idxes[i-1]+1;
	}
	return true;
}

} // __impl

template <typename Int, class F>
static void draw_without_replacement(const Int n, const Int end, F const& f)
{
	if (n == 1) {
		// Fast case
		for (Int i = 0; i < end; i++) {
			f(&i, 1);
		}
		return;
	}
	std::vector<Int> idxes;
	idxes.resize(n);
	for (Int i = 0; i < n; i++) {
		idxes[i] = i;
	}

	while (true) {
		const Int cur_last = idxes.back();
		for (Int i = cur_last; i < end; i++) {
			idxes.back() = i;
			if (!f(&idxes[0], n)) {
				return;
			}
		}

		if (!__impl::plus1(n-2, idxes)) {
			break;
		}
	}
}


}

#endif
