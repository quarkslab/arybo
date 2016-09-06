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

#ifndef PETANQUE_SYMBOLS_H
#define PETANQUE_SYMBOLS_H

#include <pa/exports.h>
#include <pa/exprs.h>
#include <pa/cast.h>

#include <vector>
#include <map>
#include <string>

namespace pa {

class PA_API Symbols
{
public:
	typedef Expr::ExprSymStorage::idx_type idx_type;

private:
	static constexpr idx_type mask_is_arg  = 0xF0000000;
	static constexpr idx_type mask_arg_idx = 0x0FFFFFFF;

public:
	ExprSym symbol(std::string const& name)
	{
		auto it = _map_symbols.find(name);
		idx_type idx;
		if (it == _map_symbols.end()) {
			idx = _map_symbols.size();
			auto it = _map_symbols.insert(std::make_pair(name, idx));
			assert(idx == _names.size());
			_names.push_back(it.first->first.c_str());
		}
		else {
			idx = it->second;
		}
		return ExprSym(idx);
	}

	const char* name(ExprSym const& e) const;

	inline const char* name(Expr const& e) const
	{
		return name(expr_assert_cast<ExprSym const&>(e));
	}

	static inline ExprSym arg_symbol(idx_type const n)
	{
		assert((n & mask_is_arg) == 0);
		return ExprSym(n | mask_is_arg);
	}

private:
	std::map<std::string, ExprSym::idx_type> _map_symbols;
	std::vector<const char*> _names;
};

// Use a global Symbols *non-thread safe* storage object
PA_API ExprSym symbol(const char* name);
PA_API ExprSym arg_symbol(Symbols::idx_type const n);
PA_API Symbols* symbols();

} // pa

#endif
