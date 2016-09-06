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

#ifndef PETANQUE_CAST_H
#define PETANQUE_CAST_H

#include <type_traits>

#include <pa/exprs.h>

namespace pa {

template <class D, class O>
inline D expr_assert_cast(O&& o)
{
#ifndef NDEBUG
	typedef typename std::remove_reference<typename std::remove_cv<D>::type>::type dst_type;
	static constexpr expr_type_id dst_type_id = dst_type::type_id;
	assert(o.type() == dst_type_id);
#endif
	return static_cast<D>(std::forward<O>(o));
}

template <class D, class O>
inline D expr_static_cast(O&& o)
{
	return static_cast<D>(std::forward<O>(o));
}

template <class D, class O>
inline D expr_dynamic_cast(O&& o)
{
	static_assert(std::is_pointer<D>::value && std::is_pointer<O>::value, "expr_dynamic_cast only support pointers!");

	typedef typename std::remove_pointer<typename std::remove_cv<D>::type>::type dst_type;
	static constexpr expr_type_id dst_type_id = dst_type::type_id;

	if (o->type() == dst_type_id) {
		return expr_static_cast<D>(o);
	}
	return nullptr;
}

} // pa

#endif
