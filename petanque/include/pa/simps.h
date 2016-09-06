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

#ifndef PETANQUE_SIMPS_H
#define PETANQUE_SIMPS_H

#include <pa/exports.h>

namespace pa {

class Expr;
class Vector;
class Matrix;

namespace simps {

// return true iif changes have been made
PA_API bool remove_dead_ops_no_rec(Expr& expr);
PA_API bool remove_dead_ops(Expr& expr);
PA_API bool flatten_no_rec(Expr& expr);
PA_API bool flatten(Expr& e);
PA_API bool expand_no_rec(Expr& e);
PA_API bool expand(Expr& e);
PA_API bool constants_prop_no_rec(Expr& e);
PA_API bool constants_prop_sorted_no_rec(Expr& e);
PA_API bool constants_prop(Expr& e);

// Simplification interface
PA_API Expr& simplify(Expr& e);
PA_API Vector& simplify(Vector& v);
PA_API Matrix& simplify(Matrix& m);

// ORs
PA_API bool or_to_esf(Expr& e);
PA_API bool identify_ors_no_rec(Expr& e);
PA_API bool identify_ors(Expr& e);

// ESF
PA_API Expr& expand_esf(Expr& e);
PA_API Vector& expand_esf(Vector& v);

PA_API void sort_no_rec(Expr& e);
PA_API void sort(Expr& e);

} // simps

} // pa

#endif
