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

#ifndef PETANQUE_TRAITS_H
#define PETANQUE_TRAITS_H

#include <functional>
#include <type_traits>

#include <boost/type_traits/function_traits.hpp>

namespace pa {

namespace __impl {

// add const to T0 if T1 is
template <class T0, class T1, bool is_t1_const>
struct propagate_const;

template <class T0, class T1>
struct propagate_const<T0, T1, true>
{
	typedef typename std::add_const<T0>::type type;
};

template <class T0, class T1>
struct propagate_const<T0, T1, false>
{
	typedef T0 type;
};

} // __impl

// add const to T0 if T1 is
template <class T0, class T1>
struct propagate_const: public __impl::propagate_const<T0, T1, std::is_const<T1>::value>
{ };

template <typename Function>
struct function_traits: public function_traits<decltype(&Function::operator())>
{ };

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) const>
{
	typedef ReturnType (*pointer)(Args...);
	typedef std::function<ReturnType(Args...)> function;
	typedef ReturnType return_type;
};

template <class F>
struct function_arg_expr
{
private:
	typedef typename std::remove_pointer<typename function_traits<F>::pointer>::type F_;

public:
	typedef typename std::remove_const<typename std::remove_reference<typename boost::function_traits<F_>::arg1_type>::type>::type type;
};

} // pa

#endif
