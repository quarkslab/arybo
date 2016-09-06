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

#ifndef PETANQUE_VISITORS_H
#define PETANQUE_VISITORS_H

#include <pa/exprs.h>
#include <pa/cast.h>

namespace pa {

template <class T>
struct visitor_inplace
{
	bool visit(Expr& e)
	{
		if (e.has_args()) {
			return this_().with_args(e);
		}

		if (auto* E = expr_dynamic_cast<pa::ExprImm*>(&e)) {
			return this_().imm(*E);
		}
		if (auto* E = expr_dynamic_cast<pa::ExprSym*>(&e)) {
			return this_().sym(*E);
		}
		assert(false);
		return false;
	}

protected:
	// Helper
	bool visit_args(Expr& e)
	{
		assert(e.has_args());
		bool ret = false;
		for (Expr& a: e.args()) {
			ret |= this_().visit(a);
		}
		return ret;
	}

protected:
	bool with_args(Expr& e)
	{
		switch (e.type()) {
			case expr_type_id::add_type:
				return this_().add(expr_static_cast<ExprAdd&>(e));
			case expr_type_id::mul_type:
				return this_().mul(expr_static_cast<ExprMul&>(e));
			case expr_type_id::or_type:
				return this_().or_(expr_static_cast<ExprOr&>(e));
			case expr_type_id::esf_type:
				return this_().esf(expr_static_cast<ExprESF&>(e));
			default:
				assert(false);
		}
		return false;
	}
	bool add(ExprAdd&) { return false; }
	bool mul(ExprMul&) { return false; }
	bool esf(ExprESF&) { return false; }
	bool or_(ExprOr&)  { return false; }
	bool sym(ExprSym&) { return false; }
	bool imm(ExprImm&) { return false; }

private:
	inline T& this_() { return static_cast<T&>(*this); }
};

template <class T>
struct visitor
{
	pa::Expr visit(pa::Expr const& e)
	{
		if (e.has_args()) {
			return this_().with_args(e);
		}

		if (auto* E = expr_dynamic_cast<pa::ExprImm const*>(&e)) {
			return this_().imm(*E);
		}
		if (auto* E = expr_dynamic_cast<pa::ExprSym const*>(&e)) {
			return this_().sym(*E);
		}
		assert(false);
		return e;
	}

protected:
	// Helper
	pa::Expr visit_args(Expr const& e)
	{
		assert(e.has_args());
		pa::ExprArgs args;
		pa::ExprArgs const& org = e.args();
		// TODO: enhance when pector will be back!
		args.resize(org.size());
		auto& my_ = this_();
		std::transform(org.begin(), org.end(), args.begin(),
			[&my_](pa::Expr const& a) { return my_.visit(a); });
		return Expr{e.type(), Expr::ExprArgsStorage{std::move(args)}};
	}

protected:
	pa::Expr with_args(pa::Expr const& e)
	{
		switch (e.type()) {
			case pa::expr_type_id::add_type:
				return this_().add(expr_static_cast<ExprAdd&>(e));
			case pa::expr_type_id::mul_type:
				return this_().mul(expr_static_cast<ExprMul&>(e));
			case pa::expr_type_id::or_type:
				return this_().or_(expr_static_cast<ExprOr&>(e));
			case pa::expr_type_id::esf_type:
				return this_().esf(expr_static_cast<ExprESF&>(e));
			default:
				assert(false);
		}
		return e;
	}
	pa::Expr add(pa::ExprAdd const& e) { return e; }
	pa::Expr mul(pa::ExprMul const& e) { return e; }
	pa::Expr or_(pa::ExprOr const& e)  { return e; }
	pa::Expr esf(pa::ExprESF const& e) { return e; }
	pa::Expr sym(pa::ExprSym const& e) { return e; }
	pa::Expr imm(pa::ExprImm const& e) { return e; }

private:
	inline T& this_() { return static_cast<T&>(*this); }
};

}

#endif
