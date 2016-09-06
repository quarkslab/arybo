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

#ifndef PETANQUE_DYNAMIC_CALLER_H
#define PETANQUE_DYNAMIC_CALLER_H

#include <pa/traits.h>

#include <cassert>
#include <cstdlib>

namespace pa {

namespace __impl {

template <class Int, Int N, class... Types>
struct dynamic_caller_rec;

template <class Int, Int N, class T0, class... Types>
struct dynamic_caller_rec<Int, N, T0, Types...>
{
	template <class F, class B>
	static inline auto call(Int type, B& o, F const& f)
	{
		if (type == N) {
			return f(static_cast<typename pa::propagate_const<T0, B>::type&>(o));
		}
		return dynamic_caller_rec<Int, N+1, Types...>::call(type, o, f);
	}

	template <class F, class B>
	static inline auto& call_ref(Int type, B& o, F const& f)
	{
		if (type == N) {
			return f(static_cast<typename pa::propagate_const<T0, B>::type&>(o));
		}
		return dynamic_caller_rec<Int, N+1, Types...>::call_ref(type, o, f);
	}
};

template <class Int, Int N, class T0>
struct dynamic_caller_rec<Int, N, T0>
{
	template <class F, class B>
	static inline auto call(Int type, B& o, F const& f)
	{
		(void) type;
		assert(type == N);
		return f(static_cast<typename pa::propagate_const<T0, B>::type&>(o));
	}

	template <class F, class B>
	static inline auto& call_ref(Int type, B& o, F const& f)
	{
		(void) type;
		assert(type == N);
		return f(static_cast<typename pa::propagate_const<T0, B>::type&>(o));
	}
};


template <class Int, class... Types>
struct dynamic_caller: public dynamic_caller_rec<Int, 0, Types...>
{
};

} // __impl

template <class... Types, class T, class F, class Int>
inline auto dynamic_call(Int type, T& o, F const& f)
{
	return __impl::dynamic_caller<Int, Types...>::call(type, o, f);
}

template <class... Types, class T, class F, class Int>
inline auto& dynamic_call_ref(Int type, T& o, F const& f)
{
	return __impl::dynamic_caller<Int, Types...>::call_ref(type, o, f);
}

} // pa

#endif
