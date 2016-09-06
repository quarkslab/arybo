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

#include <pa/analyses.h>
#include <pa/exprs.h>
#include <pa/simps.h>
#include <pa/vector.h>
#include <pa/prettyprinter.h>

#include <sstream>

pa::analyses::UnknownSymbol::UnknownSymbol(Expr const& s)
{
	assert(s.type() == pa::expr_type_id::symbol_type);
	std::stringstream ss;
	ss << "Unknown symbol " << pa::pretty_print(s) << ".";
	_err = ss.str();
}

pa::App pa::analyses::vectorial_decomp(Vector const& symbols, Vector const& v)
{
	// Assume vector is sorted and simplified.
	
	//const size_t N = symbols.size();
	const size_t N = v.size();
	const size_t Ns = symbols.size();
	pa::Matrix M(N, Ns, ExprImm(0));
	pa::Vector vimm(N, ExprImm(0));
	pa::Vector vlin(N, ExprAdd());
	pa::Vector vnl(N, ExprAdd());
	size_t i = 0;
	for (Expr const& e: v) {
		switch (e.type()) {
		case pa::expr_type_id::imm_type:
			vimm[i] = e;
			break;

		case pa::expr_type_id::symbol_type:
			vlin[i].args().insert_dup(e);
			break;

		case pa::expr_type_id::add_type:
		{
			pa::ExprArgs::const_reverse_iterator it = e.args().rbegin();
			pa::ExprArgs::const_reverse_iterator it_end = e.args().rend();
			if (it == it_end) {
				break;
			}
			if (it->type() == pa::expr_type_id::imm_type) {
				vimm[i] = *it;
				it++;
			}
			for (; it != it_end; it++) {
				if (it->type() == pa::expr_type_id::symbol_type) {
					vlin[i].args().insert_dup(*it);
				}
				else {
					vnl[i].args().insert_dup(*it);
				}
			}
			break;
		}

		case pa::expr_type_id::mul_type:
		{
			vnl[i].args() = e.args();
			vnl[i].set_type(pa::expr_type_id::mul_type);
			break;
		}

		default:
			break;
		};
		i++;
	}

	i = 0;
	for (Expr const& e: vlin) {
		for (Expr const& s: e.args()) {
			const ssize_t j = find_expr_idx(symbols, s);
			if (j == -1) {
				throw UnknownSymbol(s);
			}
			M.at(i, j) = ExprImm(1);
		}
		i++;
	}

    for (Expr& e: vnl) {
        if (e.nargs() == 0) {
            e = ExprImm(0);
        }
        else
        if (e.nargs() == 1) {
            e = std::move(e.args()[0]);
        }
    }

	pa::AffApp aff(std::move(M), std::move(vimm));
	return pa::App(pa::VectorApp(symbols, vnl), std::move(aff));
}

pa::Vector::const_iterator pa::analyses::find_expr(Vector const& v, Expr const& e)
{
	return find_expr(v.begin(), v.end(), e);
}

ssize_t pa::analyses::find_expr_idx(Vector const& v, Expr const& e)
{
	return find_expr_idx(v.begin(), v.end(), e);
}
