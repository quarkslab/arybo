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

#include <algorithm>

#include <pa/algos.h>
#include <pa/exprs.h>
#include <pa/cast.h>
#include <pa/simps.h>

bool pa::Expr::ExprArgsStorage::args_less_than(pa::ExprArgs const& a, pa::ExprArgs const& b)
{
	if (a.size() < b.size()) {
		return true;
	}
	if (a.size() > b.size()) {
		return false;
	}

	return std::lexicographical_compare(std::begin(a), std::end(a), std::begin(b), std::end(b));
}

bool pa::Expr::ExprArgsStorage::args_equal_with(pa::ExprArgs const& a, pa::ExprArgs const& b)
{
	return a == b;
}

bool pa::Expr::operator==(Expr const& o) const
{
	if (type() != o.type()) {
		return false;
	}
	return call([&o] (auto const& t) { typedef decltype(t) E; return t == expr_assert_cast<E const&>(o); });
}

bool pa::Expr::operator<(Expr const& o) const
{
	if (type() != o.type()) {
		return type() < o.type();
	}

	return call([&o] (auto const& e) { typedef decltype(e) E; return e < expr_assert_cast<E const&>(o); });
}

bool pa::Expr::is_anf() const
{
	auto const* add = expr_dynamic_cast<pa::ExprAdd const*>(this);
	if (!add) {
		return false;
	}

	for (Expr const& a: add->args()) {
		if (auto const* mul = expr_dynamic_cast<pa::ExprMul const*>(&a)) {
			for (Expr const& s: mul->args()) {
				if (!s.is_sym()) {
					return false;
				}
			}
		}
		else
		if (auto const* imm = expr_dynamic_cast<pa::ExprImm const*>(&a)) {
			if (imm->value() != true) {
				return false;
			}
		}
		else
		if (!a.is_sym()) { 
			return false;
		}
	}
	return true;
}

unsigned pa::Expr::anf_esf_max_degree() const
{
	assert(is_anf());
	for (auto it = args().rbegin(); it != args().rend(); ++it) {
		if (it->is_mul()) {
			return it->args().size();
		}
	}
	return 0;
}

bool pa::Expr::contains(Expr const& o) const
{
	if (type() != o.type()) {
		if (has_args()) {
			return args().find(o) != args().cend();
		}
		return false;
	}

	if ((!has_args()) || (o.args().size() == args().size())) {
		return (*this == o);
	}

	// If this is the same type, checks that arguments of 'o' are inside our expression.
	// Assumes than 'o' and ourself are sorted
	assert(std::is_sorted(args().begin(), args().end()));
	assert(std::is_sorted(o.args().begin(), o.args().end()));
	if (o.args().size() > args().size()) {
		return false;
	}
	auto it_o = o.args().begin();
	const auto it_o_end = o.args().end();
	const auto it_end = args().end();
	auto it_find = args().begin();
	while (it_o != it_o_end) {
		it_find = args().find(*it_o, it_find);
		if (it_find == it_end) {
			return false;
		}
		++it_o;
		++it_find;
	}
	return true;
}

uint64_t pa::Expr::hash() const
{
	return call([](auto const& e) { return e.hash(); });
}

void pa::ExprESF::expand()
{
	if (degree() == 1) {
		// Fast path, just transform it into an addition
		set_type(expr_type_id::add_type);
		return;
	}
	ExprAdd ret;
	ExprArgs& args_ret = ret.args();
	ExprArgs const& args_ = args();
	draw_without_replacement<size_t>(degree(), nargs(), 
		[&args_ret,&args_](size_t const* idxes, size_t const n)
		{
			if (n == 0) {
				return true;
			}
			Expr tmp = args_[idxes[0]];
			for (size_t i = 1; i < n; i++) {
				Expr const& cur_a = args_[idxes[i]];
				if (cur_a.is_imm()) {
					ExprImm const& cur_imm = cur_a.as<ExprImm>();
					if (cur_imm.value() == false) {
						return true;
					}
				}
				else {
					tmp *= cur_a;
				}
			}
			args_ret.insert_dup(std::move(tmp));
			return true;
		});
	if (args_ret.size() == 1) {
		static_cast<Expr&>(*this) = std::move(args_ret[0]);
	}
	else {
		set<ExprAdd>(std::move(ret));
	}
}
