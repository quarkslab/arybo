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
#include <pa/exprs.h>
#include <pa/syms_hist.h>

bool pa::SymbolsHist::compute(Expr const& e)
{
	if (!e.is_anf()) {
		return false;
	}

	_hist.clear();
	for (Expr const& a: e.args()) {
		if (auto const* mul = expr_dynamic_cast<ExprMul const*>(&a)) {
			for (Expr const& s: mul->args()) {
				_hist[s.as<ExprSym>().idx()]++;
			}
		}
	}
	return true;
}

bool pa::SymbolsHist::compute(Expr const& e, unsigned args_mul)
{
	if (!e.is_anf()) {
		return false;
	}

	_hist.clear();
	const auto it_end = e.args().rend();
	for (auto it = e.args().rbegin(); it != it_end; ++it) {
		if (auto const* mul = expr_dynamic_cast<ExprMul const*>(&*it)) {
			const size_t nargs = mul->args().size();
			if (nargs < args_mul) {
				break;
			}
			if (nargs != args_mul) {
				continue;
			}
			for (Expr const& s: mul->args()) {
				_hist[s.as<ExprSym>().idx()]++;
			}
		}
	}
	return true;
}

pa::SymbolsHist::count_type pa::SymbolsHist::count(ExprSym const& sym) const
{
	auto it = _hist.find(sym.idx());
	if (it == _hist.end()) {
		return 0;
	}
	return it->second;
}

pa::SymbolsHist::count_type pa::SymbolsHist::count(Expr const& sym) const
{
	if (auto* S = expr_dynamic_cast<ExprSym const*>(&sym)) {
		return count(*S);
	}
	return 0;
}
