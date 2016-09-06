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

#ifndef PETANQUE_SUBS_H
#define PETANQUE_SUBS_H

#include <pa/exports.h>
#include <pa/algos.h>
#include <pa/bitfield.h>
#include <pa/exprs.h>
#include <pa/vector.h>
#include <pa/matrix.h>

namespace pa {

class Vector;
class Matrix;

namespace __impl {

template <class SC, class VC>
bool vec_syms_vec_bits_to_bitfields(bitfield& bf_syms, bitfield& bf_values, SC const& syms, VC const& values)
{
	if (array_size(syms) != array_size(values)) {
		return false;
	}
	bf_syms.reserve(array_size(syms));
	bf_values.reserve(array_size(syms));
	bf_syms.clear_all();
	bf_values.clear_all();

	auto it_sym = std::begin(syms);
	auto it_sym_end = std::end(syms);
	auto it_values = std::begin(values);
	for (; it_sym != it_sym_end; ++it_sym, ++it_values) {
		it_sym->call_if(
			[&bf_syms, &bf_values, it_values] (ExprSym const& s_)
			{
				const auto idx = s_.idx();
				bf_syms.set_bit(idx);
				if (*it_values) {
					bf_values.set_bit(idx);
				}
			});
	}
	return true;
}

template <class VecC, class ValC>
bool list_vec_syms_list_values_to_bitfields(bitfield& bf_syms, bitfield& bf_values, VecC const& vecs, ValC const& vec_values)
{
	if (array_size(vecs) != array_size(vec_values)) {
		return false;
	}
	size_t nsyms = 0;
	for (auto const& vec: vecs) {
		nsyms += array_size(vec);
	}
	bf_syms.reserve(nsyms);
	bf_values.reserve(nsyms);
	bf_syms.clear_all();
	bf_values.clear_all();

	auto it_vec = std::begin(vecs);
	auto it_vec_end = std::end(vecs);
	auto it_vec_values = std::begin(vec_values);
	for (; it_vec != it_vec_end; ++it_vec, ++it_vec_values) {
		auto const& vec = *it_vec;
		const auto value = *it_vec_values;

		size_t i = 0;
		for (Expr const& e: vec) {
			e.call_if(
				[&bf_syms, &bf_values, value, i] (ExprSym const& s_)
				{
					const auto idx = s_.idx();
					bf_syms.set_bit(idx);
					if (value & ((decltype(value)(1))<<i)) {
						bf_values.set_bit(idx);
					}
				});
			i++;
		}
	}

	return true;
}

} // __impl

PA_API void subs(Expr& e, bitfield const& syms, bitfield const& values);
PA_API void subs(Vector& e, bitfield const& syms, bitfield const& values);
PA_API void subs(Matrix& e, bitfield const& syms, bitfield const& values);

template <class O, class SC, class VC>
bool subs(O& o, SC const& syms, VC const& values)
{
	bitfield bf_syms, bf_values;
	if (!__impl::vec_syms_vec_bits_to_bitfields(bf_syms, bf_values, syms, values)) {
		return false;
	}
	subs(o, bf_syms, bf_values);
	return true;
}

template <class O, class VecC, class ValC>
bool subs_vectors(O& o, VecC const& vecs, ValC const& vec_values)
{
	bitfield bf_syms, bf_values;
	if (!__impl::list_vec_syms_list_values_to_bitfields(bf_syms, bf_values, vecs, vec_values)) {
		return false;
	}
	subs(o, bf_syms, bf_values);
	return true;
}


template <class MapExprs>
void subs_exprs(Expr& e, MapExprs const& map)
{
	const auto it = map.find(e);
	if (it != map.end()) {
		e = it->second;
		return;
	}

	if (e.has_args()) {
		ExprArgs& args = e.args();
		for (Expr& a: args) {
			subs_exprs(a, map);
		}
		std::sort(args.begin(), args.end());
	}
}

template <class MapExprs>
void subs_exprs(Vector& v, MapExprs const& map)
{
	for (Expr& e: v) {
		subs_exprs(e, map);
	}
}

template <class MapExprs>
void subs_exprs(Matrix& m, MapExprs const& map)
{
	for (Expr& e: m) {
		subs_exprs(e, map);
	}
}

} // pa

#endif
