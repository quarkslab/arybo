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

#include <pa/cast.h>
#include <pa/subs.h>
#include <pa/matrix.h>

void pa::subs(Expr& e, bitfield const& syms, bitfield const& values)
{
	if (auto* S = expr_dynamic_cast<ExprSym*>(&e)) {
		const ExprSym::idx_type idx = S->idx();
		if (syms.get_bit(idx)) {
			e.set<ExprImm>(values.get_bit(idx));
		}
		return;
	}

	if (e.has_args()) {
		ExprArgs& args = e.args();
		for (Expr& a: args) {
			subs(a, syms, values);
		}
		args.sort();
	}
}

void pa::subs(Vector& v, bitfield const& syms, bitfield const& values)
{
	for (Expr& e: v) {
		subs(e, syms, values);
	}
}

void pa::subs(Matrix& m, bitfield const& syms, bitfield const& values)
{
	for (Expr& e: m) {
		subs(e, syms, values);
	}
}
