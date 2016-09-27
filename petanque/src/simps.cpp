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

#include <pa/algos.h>
#include <pa/bitfield.h>
#include <pa/cast.h>
#include <pa/compat.h>
#include <pa/exprs.h>
#include <pa/simps.h>
#include <pa/vector.h>
#include <pa/matrix.h>
#include <pa/prettyprinter.h>
#include <pa/config.h>

#ifdef PA_USE_TBB
#include <tbb/parallel_sort.h>
#endif

#include <algorithm>
#include <iostream>

static bool flatten_no_rec_once(pa::Expr& expr)
{
	if (!expr.has_args()) { 
		return false;
	}
	// This could have been created by the user or any passes
	if (expr.nargs() == 0) {
		expr.set<pa::ExprImm>(false);
		return true;
	}
	if (expr.is_esf()) {
		return false;
	}
	// Same as for the 0 case
	if (expr.nargs() == 1) {
		expr = std::move(expr.args()[0]);
		return true;
	}

	const pa::expr_type_id type = expr.type();
	bool has_candidates = false;
	size_t new_size = 0;
	for (pa::Expr const& a: expr.args()) {
		if (a.type() == type) {
			new_size += a.nargs();
			has_candidates = true;
		}
		else {
			new_size++;
		}
	}
	if (!has_candidates) {
		return false;
	}
	if (expr.is_mul() || expr.is_or()) {
		pa::ExprArgs new_args;
		new_args.reserve(new_size);
		// TODO: hint on the position to use for insert
		for (pa::Expr& a: expr.args()) {
			if (a.type() == type) {
				new_args.insert(std::move(a.args()));
			}
			else {
				new_args.insert(std::move(a));
			}
		}
		expr.args() = std::move(new_args);
		assert(expr.nargs() > 1);
	}
	else {
		assert(expr.is_add());
		pa::ExprAdd ret;
		// TODO: hint on the position to use for emplace_arg
		for (pa::Expr& a: expr.args()) {
			if (a.type() == type) {
				ret.extend_args(std::move(a.args()));
			}
			else {
				ret.emplace_arg(std::move(a));
			}
		}
		expr = std::move(ret);
		expr.fix_unary();
	}
	return true;
}

bool pa::simps::flatten_no_rec(Expr& expr)
{
	bool changed = false;
	bool ret;
	do {
		ret = flatten_no_rec_once(expr);
		changed |= ret;
	}
	while (ret);
	return changed;
}

static void resize_args(pa::Expr& expr, pa::ExprArgs& args, pa::ExprArgs::iterator const it)
{
	const size_t new_size = std::distance(args.begin(), it);
	if (new_size == 0) {
		expr.set<pa::ExprImm>(0);
	}
	else {
		args.resize(new_size);
		args.shrink_to_fit();
	}
}

static void sort_args(pa::ExprArgs& args)
{
#ifdef PA_USE_TBB
	tbb::parallel_sort(args.begin(), args.end());
#else
	std::sort(args.begin(), args.end());
#endif
}

static bool unique_args(pa::Expr& expr, pa::ExprArgs& args)
{
	auto it = std::unique(args.begin(), args.end());
	const bool changed = it != args.end();
	resize_args(expr, args, it);
	return changed;
}

static bool remove_dead_ops_once(pa::Expr& expr)
{
	if (!expr.has_args()) {
		return false;
	}

	if (expr.is_esf()) {
		return false;
	}

	pa::ExprArgs& args = expr.args();
	bool ret;

	// Multiplication or OR implies only a unique operation
	if (expr.is_mul() || expr.is_or()) {
		ret = unique_args(expr, args);
		if (args.size() > 1) {
			pa::Expr const& last_arg = args.back();
			// 0 is useless for or, 1 is useless for mul
			const bool useless_imm = expr.is_mul();
			// Remove the last immediate if there it is equal to last_v
			pa::ExprImm const* last_imm = pa::expr_dynamic_cast<pa::ExprImm const*>(&last_arg);
			if (last_imm && last_imm->value() == useless_imm) {
				args.resize(args.size()-1);
				ret = true;
			}
		}
	}
	else {
		// For additions, remove two consecutive equals expressions
		auto it = pa::remove_consecutives(args.begin(), args.end(),
			[] (pa::Expr const& e)
			{
				bool ret = true;
				e.call_if([&ret] (pa::ExprImm const& e_)
					{
						if (e_.value() == 0) {
							ret = false;
						}
					});
				return ret;
			});
		ret = (it != args.end());
		if (ret) {
			resize_args(expr, args, it);
		}
	}

	// If we only have one argument left, set it as the expression
	if (args.size() == 1) {
		expr = std::move(args[0]);
		ret = true;
	}
	return ret;
}

bool pa::simps::remove_dead_ops_no_rec(Expr& expr)
{
	return remove_dead_ops_once(expr);
}

bool pa::simps::remove_dead_ops(Expr& e)
{
	bool changed = false;
	if (e.has_args()) {
		for (Expr& a: e.args()) {
			changed |= remove_dead_ops(a);
		}
	}

	changed |= constants_prop_no_rec(e);
	changed |= remove_dead_ops_no_rec(e);
	return changed;
}

bool pa::simps::flatten(Expr& e)
{
	if (!e.has_args()) {
		return false;
	}

	bool changed = false;
	for (Expr& a: e.args()) {
		changed |= flatten(a);
	}

	changed |= flatten_no_rec(e);
	return changed;
}

bool pa::simps::constants_prop_sorted_no_rec(pa::Expr& expr)
{
	pa::expr_type_id const type = expr.type();
	if (type != pa::expr_type_id::esf_type) {
		return false;
	}

	pa::ExprArgs& args = expr.args();
	for (auto it = args.rbegin(); it != args.rend(); ++it) {
		pa::Expr& a = *it;
		if (a.type() != pa::expr_type_id::imm_type) {
			break;
		}

		if (pa::expr_static_cast<pa::ExprImm&>(a).value() == false) {
			auto rbegin_zero = it;
			++it;
			auto rend_zero = it;
			for (; it != args.rend(); ++it) {
				if ((it->type() != pa::expr_type_id::imm_type) ||
				    (pa::expr_static_cast<pa::ExprImm&>(*it).value() != false)) {
					rend_zero = it;
					break;
				}
			}
			const auto begin_zero = rend_zero.base();
			const auto end_zero = rbegin_zero.base();
			if ((begin_zero == args.begin()) &&
				(end_zero == args.end())) {
				expr.set<ExprImm>(false);
			}
			else {
				args.erase(begin_zero, end_zero);
				const size_t size = args.size();
				const size_t deg = pa::expr_static_cast<pa::ExprESF const&>(expr).degree();
				if (size == deg) {
					expr.set_type(pa::expr_type_id::mul_type);
				}
				else
				if (size < deg) {
					expr.set<ExprImm>(false);
				}
			}
			return true;
		}
	}

	return false;
}

bool pa::simps::constants_prop_no_rec(Expr& expr)
{
	expr_type_id const type = expr.type();
	if (type == expr_type_id::mul_type) {
		const bool value_prop = false;

		ExprArgs& args = expr.args();
		for (Expr& a: args) {
			if (a.type() == expr_type_id::imm_type) {
				bool v;
				a.call_assert([&v] (ExprImm const& e_) { v = e_.value(); });
				if (v == value_prop) {
					expr.set<ExprImm>(value_prop);
					return true;
				}
			}
		}
	}
	return false;
}

bool pa::simps::constants_prop(Expr& e)
{
	bool changed = false;
	if (e.has_args()) {
		for (Expr& a: e.args()) {
			changed |= constants_prop(a);
		}
	}

	changed |= constants_prop_no_rec(e);
	return changed;
}

static void expand_mul_add_add(pa::Expr& ea, pa::Expr& eb)
{
	pa::ExprArgs ret;
	ret.reserve(ea.nargs() * eb.nargs());
	for (pa::Expr& a: ea.args()) {
		for (pa::Expr& b: eb.args()) {
			pa::Expr ne = a*b;
			if (!ne.is_zero()) {
				auto it = ret.insert(std::move(ne));
				if (!it.second) {
					ret.erase(it.first);
				}
			}
		}
	}
	ea.args() = std::move(ret);
}

bool pa::simps::expand_no_rec(Expr& e)
{
	// Transform as mucch as possible an ExprMul(ExprAdd) into a ExprAdd(ExprMul)
	// ExprMul(ExprAdd(a, b), ExprAdd(c, d), e)
	// Suppose arguments are sorted.
	if (e.type() != expr_type_id::mul_type) {
		return false;
	}

	ExprArgs& args = e.args();
	const size_t n = args.size();

	assert(n > 0);
	assert((args[0].type() >= expr_type_id::add_type) || args[0].is_or_esf());
	assert(args[n-1].type() <= expr_type_id::symbol_type);

	if (args[0].type() == expr_type_id::symbol_type) {
		// It means that there is only symbols in this expression
		return false;
	}

	if ((n == 2) && args[0].is_or_esf() && args[1].is_add()) {
		// We are in the state ExprMul(Expr(Or|ESF)(a, b, c), ExprAdd(d, e, f)), and
		// we can't do nothing more until ORs are replaced.
		return false;
	}

	// There, we have a mix of ORs, SEFs, additions and symbols. We are in this state:
	// ExprMul(ExprOr(a, b), ExprESF(e, f), ExprAdd(a, b), ExprAdd(c, d), e)
	
	size_t add_start;
	for (add_start = 0; add_start < n; add_start++) {
		if (args[add_start].is_add()) {
			break;
		}
	}

	if (add_start >= n) {
		return false;
	}

	// Let's compute the final ExprAdd!
	Expr final_add = std::move(args[add_start]);
	assert(final_add.type() == expr_type_id::add_type);
	size_t i;
	for (i = add_start+1; i < n; i++) {
		Expr& a = args[i];
		if (a.type() == expr_type_id::add_type) {
			expand_mul_add_add(final_add, a);
		}
		else {
			// Break here as it meens that we only have left ExprAdd(...)*symbols
			// Just need to propagate this into our expradd
			break;
		}
	}

	if (i < n) {
		pa::ExprMul mul_sym;
		mul_sym.as<pa::ExprMul>().extend_args(args.begin()+i, args.end());
		mul_sym.fix_unary();
		for (Expr& a: final_add.args()) {
			a *= mul_sym;
		}
	}

	std::sort(final_add.args().begin(), final_add.args().end());
	if (add_start == 0) {
		e.set<ExprAdd>();
		e.args() = std::move(final_add.args());
	}
	else {
		ExprMul& e_ = expr_static_cast<ExprMul&>(e);
		// First arguments (ORs and/or ESFs) are still valid, just resize and
		// move the final addition at the end.
		e_.args().resize(add_start);
		e_.emplace_arg(std::move(final_add));
		e_.args().shrink_to_fit();
	}

	return true;
}

bool pa::simps::expand(Expr& e)
{
	if (!e.has_args()) {
		return false;
	}

	bool changed = false;
	for (Expr& a: e.args()) {
		changed |= expand(a);
	}

	changed |= expand_no_rec(e);
	return changed;
}

static bool simplify_no_rec(pa::Expr& e)
{
	bool changed = false;
	bool exp_changed;
	do {
		changed |= pa::simps::constants_prop_no_rec(e);
		changed |= pa::simps::flatten_no_rec(e);
		changed |= pa::simps::constants_prop_sorted_no_rec(e);
		changed |= pa::simps::remove_dead_ops_no_rec(e);
		changed |= pa::simps::flatten_no_rec(e);
		exp_changed = pa::simps::expand_no_rec(e);
		changed |= exp_changed;
	}
	while (exp_changed);
	return changed;
}

static bool expand_esf_rec(pa::Expr& e)
{
	if (!e.has_args()) {
		return false;
	}

	bool ret = false;
	for (pa::Expr& a: e.args()) {
		ret |= expand_esf_rec(a);
	}

	if (!e.is_esf()) {
		return ret;
	}

	pa::ExprESF& esf = pa::expr_static_cast<pa::ExprESF&>(e);
	esf.expand();
	simplify_no_rec(esf);

	return true;
}

static bool or_to_esf_no_rec(pa::Expr& e)
{
	if (!e.is_or()) {
		return false;
	}

	// a|b|c|..|n = SEF(1,a,b,..,n) + ... + SEF(#n, a, b, c, n)
	const size_t n = e.nargs();
	pa::ExprAdd ret;
	ret.resize_args(n);

	for (size_t i = 0; i < n; i++) {
		ret.args()[i] = pa::ExprESF(i+1, e.args());
	}

	e = std::move(ret);
	return true;
}

bool pa::simps::or_to_esf(pa::Expr& e)
{
	bool ret = false;
	if (e.has_args()) {
		for (Expr& a: e.args()) {
			ret |= or_to_esf(a);
		}
	}

	return or_to_esf_no_rec(e);
}

static bool simplify_rec(pa::Expr& e)
{
	if (!e.has_args()) {
		return false;
	}

	assert(std::is_sorted(e.args().cbegin(), e.args().cend()));

	bool changed = false;
	for (pa::Expr& a: e.args()) {
		changed |= simplify_rec(a);
	}

	assert(changed || (!changed && std::is_sorted(e.args().cbegin(), e.args().cend())));
	if (changed) {
		sort_args(e.args());
	}
	changed |= simplify_no_rec(e);
	return changed;
}

#if 0
static void simplify_rec_no_expand(pa::Expr& e, int tab)
{
	if (!e.has_args()) {
		return;
	}

	for (pa::Expr& a: e.args()) {
		simplify_rec_no_expand(a, tab+1);
	}

	do {
		pa::simps::remove_dead_ops(e);
		pa::simps::constants_prop(e);
	}
	while (pa::simps::flatten_no_rec(e));
}
#endif

pa::Expr& pa::simps::expand_esf(Expr& e)
{
	expand_esf_rec(e);
	return e;
}

pa::Vector& pa::simps::expand_esf(Vector& v)
{
	for (Expr& e: v) {
		expand_esf(e);
	}
	return v;
}

pa::Expr& pa::simps::simplify(Expr& e)
{
	sort(e);
	simplify_rec(e);
	//expand_esf_rec(e);
	//simplify_rec(e);
	return e;
}

/*pa::Expr& pa::simps::simplify_and_expand(Expr& e)
{
	simplify_rec(e);
	expand_esf_rec(e);
	simplify_rec(e);
	return e;
}*/

pa::Vector& pa::simps::simplify(Vector& v)
{
#ifdef PA_USE_TBB
  tbb::parallel_for(size_t{0}, v.size(), [&v](size_t i) {
		simplify(v[i]);
	});
#else
  for (Expr& e: v) {
    simplify(e);
  }
#endif
	return v;
}

pa::Matrix& pa::simps::simplify(Matrix& m)
{
//#pragma omp parallel for
	//for (Expr& e: v) {
	for (size_t i = 0; i < m.nelts(); i++) {
		simplify(m.elt_at(i));
	}
	return m;
}

void pa::simps::sort_no_rec(Expr& e)
{
	if (e.has_args()) {
		sort_args(e.args());
	}
}

void pa::simps::sort(Expr& e)
{
	if (!e.has_args()) {
		return;
	}

	for (Expr& a: e.args()) {
		sort(a);
	}

	sort_no_rec(e);
}

bool pa::simps::identify_ors_no_rec(Expr& e)
{
	// Expression should be sorted!
	// Created expression will be sorted.
	
	if (!e.is_add()) {
		return false;
	}

	assert(std::is_sorted(e.args().begin(), e.args().end()));

	size_t sym_start = -1;
	size_t mul_start = -1;
	size_t esf_start = -1;

	size_t n = e.nargs();
	ExprArgs& eargs = e.args();
	size_t i;
	for (i = 0; i < n; i++) {
		if (eargs[i].is_esf()) {
			esf_start = i++;
			break;
		}
	}
	if (esf_start == (size_t)-1) {
		i = 0;
	}
	for (; i < n; i++) {
		if (eargs[i].is_mul()) {
			mul_start = i++;
			break;
		}
	}
	if (mul_start == (size_t)-1) {
		return false;
	}
	for (; i < n; i++) {
		if (eargs[i].is_sym()) {
			sym_start = i++;
			break;
		}
	}
	if (sym_start == (size_t)-1) {
		return false;
	}
	size_t nsym = 1;
	for (; i < n; i++) {
		if (!eargs[i].is_sym()) {
			break;
		}
		++nsym;
	}

	if (nsym <= 1) {
		return false;
	}

	ExprArgs new_ors;

	// Select i symbols among new_ors, for i from 2 to nsym. This gives the arguments to check for the existance of an OR operation!
search_begin:
	for (size_t k = nsym; k >= 2; k--) {
		bool go_again = false;
		draw_without_replacement<size_t>(k, nsym,
			[&](size_t const* idxes, size_t const n)
			{
				ExprArgs or_args;
				or_args.resize(n);
				for (size_t i = 0; i < n; i++) {
					or_args[i] = eargs[sym_start+idxes[i]];
				}

				const size_t or_degree = n;
				if (or_degree == 2) {
					for (size_t mul = mul_start; mul < sym_start; mul++) {
						if (eargs[mul].args() == or_args) {
							new_ors.insert_dup(ExprOr(std::move(or_args)));
							// This is an (a|b) operation
							eargs.erase(std::begin(eargs)+sym_start+idxes[1]);
							eargs.erase(std::begin(eargs)+sym_start+idxes[0]);
							eargs.erase(std::begin(eargs)+mul);
							--sym_start;
							nsym -= 2;
							go_again = true;
							return false;
						}
					}
					// Nothing here, go to the next one.
					return true;
				}

				if (esf_start == (size_t)-1) {
					// No ESF, go to the next possibility.
					return true;
				}

				assert(or_degree >= 2);

				// For size arguments bigger than 2, check ESFs 
				size_t cur_degree = 2;
				std::vector<size_t> idx_to_remove;
				idx_to_remove.reserve(or_degree-1);
				for (size_t esf = esf_start; esf < mul_start; esf++) {
					ExprESF const& eesf = expr_assert_cast<ExprESF const&>(eargs[esf]);
					if (eesf.degree() > cur_degree) {
						break;
					}
					if ((eesf.degree() == cur_degree) && (eesf.args() == or_args)) {
						idx_to_remove.push_back(esf);
						cur_degree++;
						if (cur_degree == or_degree) {
							break;
						}
					}
				}
				if (cur_degree != or_degree) {
					return true;
				}
				for (size_t mul = mul_start; mul < sym_start; mul++) {
					ExprMul const& emul = expr_assert_cast<ExprMul const&>(eargs[mul]);
					if (emul.args() == or_args) {
						idx_to_remove.push_back(mul);
						// Got an OR here
						// Remove every invloved expressions
						new_ors.insert_dup(ExprOr(std::move(or_args)));
						for (ssize_t i = n-1; i >= 0; i--) {
							eargs.erase(std::begin(eargs)+sym_start+idxes[i]);
						}
						for (auto it = idx_to_remove.rbegin(); it != idx_to_remove.rend(); it++) {
							eargs.erase(std::begin(eargs)+*it);
						}
						nsym -= n;
						mul_start -= idx_to_remove.size()-1;
						sym_start -= idx_to_remove.size();
						go_again = true;
						return false;
					}
				}
				return true;
			});
		if (go_again) {
			goto search_begin;
		}
	}

	// Preprend the new ORs at the beggining
	if (new_ors.size() > 0) {
		if (eargs.size() == 0 && new_ors.size() == 1) {
			e = std::move(new_ors[0]);
			return true;
		}
		for (size_t i = 0; i < new_ors.size(); i++) {
			eargs.insert_dup(std::move(new_ors[i]));
		}
		return true;
	}
	
	return false;
}

bool pa::simps::identify_ors(Expr& e)
{
	if (!e.has_args()) {
		return false;
	}

	bool ret = identify_ors_no_rec(e);

	for (Expr& a: e.args()) {
		ret |= identify_ors(a);
	}
	return ret;
}

#if 0
bool pa::simps::identify_ors_no_rec(Expr& e)
{
	// Expression should be sorted!
	// Created expression will be sorted.
	
	if (!e.is_add()) {
		return false;
	}

	size_t add_start = -1;
	size_t mul_start = -1;
	size_t esf_start = -1;

	size_t n = e.nargs();
	ExprArgs& a = e.args();
	size_t i;
	for (i = 0; i < n; i++) {
		if (a[i].is_esf()) {
			esf_start = i++;
			break;
		}
	}
	for (; i < n; i++) {
		if (a[i].is_mul()) {
			mul_start = i++;
			break;
		}
	}
	if (mul_start == (size_t)-1) {
		return false;
	}
	for (; i < n; i++) {
		if (a[i].is_add()) {
			add_start = i++;
			break;
		}
	}
	if (add_start == (size_t)-1) {
		return false;
	}

	ExprArgs new_ors;

	for (size_t add = add_start; add < n; add++) {
		if (!a[add].is_add()) {
			break;
		}
		ExprArgs& or_args = a[add].args();
		const size_t or_degree = or_args.size();
		for (size_t mul = mul_start; mul < add_start; mul++) {
			if (a[mul].args() == or_args) {
				if (or_degree == 2) {
					new_ors.emplace_back(ExprOr(std::move(or_args)));
					// This is an OR
					a.erase(std::begin(a)+add);
					--add; --n;
					a.erase(std::begin(a)+mul);
					--n;
					--add_start;
					goto next_args;
				}

				if (esf_start == (size_t)-1) {
					// No ESF, go to the next possibility.
					goto next_args;
				}

				assert(or_degree > 2);

				// For size arguments bigger than 2, check ESFs 
				size_t cur_degree = 3;
				std::vector<size_t> idx_to_remove;
				idx_to_remove.reserve(or_degree-2);
				for (size_t esf = esf_start; esf < mul_start; esf++) {
					ExprESF const& eesf = expr_assert_cast<ExprESF const&>(a[esf]);
					if (eesf.degree() > cur_degree) {
						goto next_args;
					}
					if ((eesf.degree() == cur_degree) && (eesf.args() == or_args)) {
						idx_to_remove.push_back(esf);
						if (cur_degree == or_degree) {
							// Got on OR here
							// Remove every invloved expressions
							new_ors.emplace_back(ExprOr(std::move(or_args)));
							a.erase(std::begin(a)+add);
							--add; --n;
							a.erase(std::begin(a)+mul);
							--n;
							--add_start;
							for (size_t idx: idx_to_remove) {
								a.erase(std::begin(a)+idx);
							}
							n -= a.size();
							mul_start -= a.size();
							goto next_args;
						}
						cur_degree++;
					}
				}
			}
		}
next_args:
		continue;
	}

	// Preprend the new ORs at the beggining
	if (new_ors.size() > 0) {
		a.insert(std::begin(a), new_ors.size(), Expr{});
		for (size_t i = 0; i < new_ors.size(); i++) {
			a[i] = std::move(new_ors[i]);
		}
		return true;
	}
	
	return false;
}
#endif
