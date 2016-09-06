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

#ifndef PETANQUE_ANALYSES_H
#define PETANQUE_ANALYSES_H

#include <pa/app.h>
#include <pa/compat.h>
#include <pa/exports.h>

#include <algorithm>
#include <string>

namespace pa {

class Vector;

namespace analyses {

class PA_API UnknownSymbol: public std::exception
{
public:
	UnknownSymbol(Expr const& s);

public:
	const char* what() const noexcept override { return _err.c_str(); }

private:
	std::string _err;
};

PA_API App vectorial_decomp(Vector const& symbols, Vector const& v);

template <typename Iterator>
Iterator find_expr(Iterator begin, Iterator end, Expr const& s)
{
	static_assert(std::is_same<typename Iterator::value_type, Expr>::value, "Iterator type must reference Expr objects");
	return std::find(begin, end, s);
}

template <typename Iterator>
ssize_t find_expr_idx(Iterator begin, Iterator end, Expr const& s)
{
	Iterator ret = find_expr(begin, end, s);
	if (ret == end) {
		return -1;
	}
	return std::distance(begin, ret);
}

PA_API Vector::const_iterator find_expr(Vector const& v, Expr const& e);
PA_API ssize_t find_expr_idx(Vector const& v, Expr const& e);

} // analyses

} // pa

#endif
