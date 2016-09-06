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

#include <pa/symbols.h>
#include <sstream>

static pa::Symbols g_symbols;
static std::map<pa::Symbols::idx_type, std::string> g_arg_names;

pa::ExprSym pa::symbol(const char* name)
{
	return g_symbols.symbol(name);
}

pa::ExprSym pa::arg_symbol(Symbols::idx_type const n)
{
    return Symbols::arg_symbol(n);
}

pa::Symbols* pa::symbols()
{
	return &g_symbols;
}

const char* pa::Symbols::name(ExprSym const& e) const
{
	const idx_type idx = e.idx();
	if ((idx & mask_is_arg) == mask_is_arg) {
		const idx_type n = idx & mask_arg_idx;
		auto it = g_arg_names.find(n);
		if (it == g_arg_names.end()) {
			std::stringstream ss;
			ss << "_" << n;
			auto ret_ins = g_arg_names.insert(std::make_pair(n, ss.str()));
			it = ret_ins.first;
		}
		return it->second.c_str();
	}
	if (idx >= _names.size()) {
		return nullptr;
	}
	return _names[idx];
}
