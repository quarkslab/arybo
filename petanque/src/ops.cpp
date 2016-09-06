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

#include <pa/exprs.h>
#include <pa/cast.h>

namespace pa {

namespace ops {

// Or

static Expr or_(Expr const& a, Expr const& b)
{
	expr_type_id ta = a.type();
	expr_type_id tb = b.type();
	if ((&a == &b) || (a == b)) {
		return a;
	}

	Expr const* ea = &a;
	Expr const* eb = &b;
	if (ta > tb) {
		std::swap(ta, tb);
		std::swap(ea, eb);
	}

	if (tb == expr_type_id::imm_type) {
		ExprImm const& b_ = expr_static_cast<ExprImm const&>(*eb);
		if (b_.value() == 1) {
			return ExprImm(1);
		}
		else {
			return *ea;
		}
	}

	if (ta == expr_type_id::or_type) {
		ExprOr const& a_ = expr_static_cast<ExprOr const&>(*ea);
		ExprOr ret;
		if (tb == expr_type_id::or_type) {
			ExprOr const& b_ = expr_static_cast<ExprOr const&>(*eb);
			ret.reserve_args(a_.nargs() + b_.nargs());
			ret.extend_args(a_.args());
			ret.extend_args_no_dup(b_.args());
		}
		else {
			ret.reserve_args(a_.nargs() + 1);
			ret.extend_args_no_dup(a_.args());
			ret.emplace_arg_no_dup(*eb);
		}
		if (ret.nargs() == 1) {
			return std::move(ret.args()[0]);
		}
		return std::move(ret);
	}

	return ExprOr({*ea, *eb});
}

// Additions

static Expr add(Expr const& a, Expr const& b)
{
	expr_type_id ta = a.type();
	expr_type_id tb = b.type();
	if (ta == tb) {
		if ((&a == &b) || (a == b)) {
			return ExprImm(0);
		}
		switch (ta) {
			case expr_type_id::add_type:
			{
				ExprAdd const& a_ = expr_static_cast<ExprAdd const&>(a);
				ExprAdd const& b_ = expr_static_cast<ExprAdd const&>(b);
				ExprAdd ret;
				// Just one allocation done here
				ret.reserve_args(a.nargs() + b.nargs());
				ret.extend_args(a_.args());
				ret.extend_args(b_.args());
				assert(std::is_sorted(ret.args().begin(), ret.args().end()));
				if (ret.nargs() == 0) {
					return ExprImm{false};
				}
				if (ret.nargs() == 1) {
					return std::move(ret.args()[0]);
				}
				return std::move(ret);
			}
			case expr_type_id::imm_type:
			{
				ExprImm const& a_ = expr_static_cast<ExprImm const&>(a);
				ExprImm const& b_ = expr_static_cast<ExprImm const&>(b);
				return ExprImm(a_.value() ^ b_.value());
			}
			case expr_type_id::symbol_type:
			{
				ExprSym const& a_ = expr_static_cast<ExprSym const&>(a);
				ExprSym const& b_ = expr_static_cast<ExprSym const&>(b);
				if (a_.idx() == b_.idx()) {
					return ExprImm(0);
				}
				return ExprAdd({a_, b_});
			}
			default:
				break;
		};
		return ExprAdd({a, b});
	}

	Expr const* ea = &a;
	Expr const* eb = &b;
	if (ta > tb) {
		std::swap(ta, tb);
		std::swap(ea, eb);
	}
	if (tb == expr_type_id::imm_type) {
		ExprImm const& b_ = expr_static_cast<ExprImm const&>(*eb);
		if (b_.value() == 0) {
			return *ea;
		}
	}

	if (tb == expr_type_id::add_type) {
		std::swap(ta, tb);
		std::swap(ea, eb);
	}

	if (ta == expr_type_id::add_type) {
		ExprAdd const& a_ = expr_static_cast<ExprAdd const&>(*ea);
		ExprAdd ret;
		ret.reserve_args(a_.nargs() + 1);
		ret.extend_args(a_.args());
		ret.emplace_arg(*eb);
		assert(std::is_sorted(ret.args().begin(), ret.args().end()));
		if (ret.nargs() == 0) {
			return ExprImm{false};
		}
		if (ret.nargs() == 1) {
			return std::move(ret.args()[0]);
		}
		return std::move(ret);
	}

	return ExprAdd({a, b});
}

// Multiplications

static Expr mul(Expr const& a, Expr const& b)
{
	expr_type_id ta = a.type();
	expr_type_id tb = b.type();
	if ((&a == &b) || (a == b)) {
		return a;
	}

	Expr const* ea = &a;
	Expr const* eb = &b;
	if (ta > tb) {
		std::swap(ta, tb);
		std::swap(ea, eb);
	}
	if (tb == expr_type_id::imm_type) {
		ExprImm const& b_ = expr_static_cast<ExprImm const&>(*eb);
		if (b_.value() == 0) {
			return ExprImm(0);
		}
		else {
			return *ea;
		}
	}

	if (ta == expr_type_id::mul_type) {
		ExprMul const& a_ = expr_static_cast<ExprMul const&>(*ea);
		ExprMul ret;
		if (tb == expr_type_id::mul_type) {
			ExprMul const& b_ = expr_static_cast<ExprMul const&>(*eb);
			ret.reserve_args(a_.nargs() + b_.nargs());
			ret.extend_args(a_.args());
			ret.extend_args_no_dup(b_.args());
		}
		else {
			ret.reserve_args(a_.nargs() + 1);
			ret.extend_args(a_.args());
			ret.emplace_arg_no_dup(*eb);
		}
		assert(std::is_sorted(ret.args().begin(), ret.args().end()));
		if (ret.nargs() == 1) {
			return std::move(ret.args()[0]);
		}
		return std::move(ret);
	}

	return ExprMul({*ea, *eb});
}

} // ops

} // pa

pa::Expr pa::Expr::operator|(Expr const& o) const
{
	return ops::or_(*this, o);
}

pa::Expr pa::Expr::operator+(Expr const& o) const
{
	return ops::add(*this, o);
}

pa::Expr pa::Expr::operator*(Expr const& o) const
{
	return ops::mul(*this, o);
}

// Generic expr
//

pa::Expr& pa::Expr::operator*=(Expr const& e)
{
	if (&e == this) {
		return *this;
	}

	if (is_mul()) {
		expr_static_cast<ExprMul&>(*this) *= e;
		return *this;
	}

	*this = (*this) * e;

	return *this;
}

// Mul
//

// Generic expr
//

pa::Expr& pa::Expr::operator|=(Expr const& e)
{
	if (&e == this) {
		return *this;
	}

	if (is_or()) {
		expr_static_cast<ExprOr&>(*this) |= e;
		return *this;
	}

	*this = (*this) | e;

	return *this;
}

pa::Expr& pa::Expr::operator|=(Expr&& e)
{
	if (&e == this) {
		return *this;
	}

	if (is_or()) {
		expr_static_cast<ExprOr&>(*this) |= std::move(e);
		return *this;
	}

	if (e.is_or()) {
		expr_static_cast<ExprOr&>(e).emplace_arg_no_dup(*this);
		*this = std::move(e);
	}
	
	*this = (*this) | std::move(e);

	return *this;
}

pa::Expr& pa::Expr::operator*=(Expr&& e)
{
	if (&e == this) {
		return *this;
	}

	if (is_mul()) {
		expr_static_cast<ExprMul&>(*this) *= std::move(e);
		return *this;
	}

	if (e.is_mul()) {
		expr_static_cast<ExprMul&>(e).emplace_arg_no_dup(*this);
		*this = std::move(e);
	}
	
	*this = (*this) * std::move(e);

	return *this;
}

pa::Expr& pa::Expr::operator+=(Expr const& e)
{
	if (&e == this) {
		*this = ExprImm(0);
		return *this;
	}

	if (is_add()) {
		expr_static_cast<ExprAdd&>(*this) += e;
		return *this;
	}

	*this = (*this) + e;

	return *this;
}

pa::Expr& pa::Expr::operator+=(Expr&& e)
{
	if (&e == this) {
		return *this;
	}

	if (is_add()) {
		expr_static_cast<ExprAdd&>(*this) += std::move(e);
		return *this;
	}

	if (e.is_add()) {
		expr_static_cast<ExprAdd&>(e).emplace_arg_no_dup(*this);
		*this = std::move(e);
	}
	
	*this = (*this) + std::move(e);

	return *this;
}

// Or
//

pa::Expr& pa::ExprOr::operator|=(ExprImm const& e)
{
	if (e.value() == 1) {
		static_cast<Expr&>(*this) = ExprImm(1);
	}
	
	return *this;
}

pa::Expr& pa::ExprOr::operator|=(ExprMul const& e)
{
	emplace_arg_no_dup(e);
	return *this;
}

pa::Expr& pa::ExprOr::operator|=(ExprMul&& e)
{
	emplace_arg_no_dup(std::move(e));
	return *this;
}

pa::Expr& pa::ExprOr::operator|=(ExprESF const& e)
{
	emplace_arg_no_dup(e);
	return *this;
}

pa::Expr& pa::ExprOr::operator|=(ExprESF&& e)
{
	emplace_arg_no_dup(std::move(e));
	return *this;
}

pa::Expr& pa::ExprOr::operator|=(ExprAdd const& e)
{
	emplace_arg_no_dup(e);
	return *this;
}

pa::Expr& pa::ExprOr::operator|=(ExprAdd&& e)
{
	emplace_arg_no_dup(std::move(e));
	return *this;
}

pa::Expr& pa::ExprOr::operator|=(ExprSym const& e)
{
	emplace_arg_no_dup(e);
	return *this;
}

pa::Expr& pa::ExprOr::operator|=(Expr const& e)
{
	e.call_no_return([this] (auto const& e_) { return *this |= e_; });
	return *this;
}

pa::Expr& pa::ExprOr::operator|=(Expr&& e)
{
	e.call_no_return([this] (auto&& e_) { return *this |= std::move(e_); });
	return *this;
}

// Mul
//

pa::Expr& pa::ExprMul::operator*=(ExprImm const& e)
{
	if (e.value() == 0) {
		static_cast<Expr&>(*this) = ExprImm(0);
	}
	
	return *this;
}

pa::Expr& pa::ExprMul::operator*=(ExprOr const& e)
{
	emplace_arg_no_dup(e);
	assert(std::is_sorted(args().begin(), args().end()));
	return *this;
}

pa::Expr& pa::ExprMul::operator*=(ExprOr&& e)
{
	emplace_arg_no_dup(std::move(e));
	assert(std::is_sorted(args().begin(), args().end()));
	return *this;
}

pa::Expr& pa::ExprMul::operator*=(ExprESF const& e)
{
	emplace_arg_no_dup(e);
	assert(std::is_sorted(args().begin(), args().end()));
	return *this;
}

pa::Expr& pa::ExprMul::operator*=(ExprESF&& e)
{
	emplace_arg_no_dup(std::move(e));
	assert(std::is_sorted(args().begin(), args().end()));
	return *this;
}

pa::Expr& pa::ExprMul::operator*=(ExprAdd const& e)
{
	emplace_arg_no_dup(e);
	assert(std::is_sorted(args().begin(), args().end()));
	return *this;
}

pa::Expr& pa::ExprMul::operator*=(ExprAdd&& e)
{
	emplace_arg_no_dup(std::move(e));
	assert(std::is_sorted(args().begin(), args().end()));
	return *this;
}

pa::Expr& pa::ExprMul::operator*=(ExprSym const& e)
{
	emplace_arg_no_dup(e);
	assert(std::is_sorted(args().begin(), args().end()));
	return *this;
}

pa::Expr& pa::ExprMul::operator*=(Expr const& e)
{
	e.call_no_return([this] (auto const& e_) { return *this *= e_; });
	return *this;
}

pa::Expr& pa::ExprMul::operator*=(Expr&& e)
{
	e.call_no_return([this] (auto&& e_) { return *this *= std::move(e_); });
	return *this;
}

// Add
//

pa::ExprAdd& pa::ExprAdd::operator+=(ExprAdd const& e)
{
	if (&e == this) {
		static_cast<Expr&>(*this) = ExprImm(0);
	}
	else {
		extend_args(e.args());
		fix_unary();
	}
	return *this;
}

pa::ExprAdd& pa::ExprAdd::operator+=(ExprAdd&& e)
{
	if (&e == this) {
		static_cast<Expr&>(*this) = ExprImm(0);
	}
	else {
		extend_args(std::move(e.args()));
		fix_unary();
	}
	return *this;
}

pa::Expr& pa::ExprAdd::operator+=(ExprOr&& e)
{
	emplace_arg(std::move(e));
	assert(std::is_sorted(args().begin(), args().end()));
	fix_unary();
	return *this;
}

pa::Expr& pa::ExprAdd::operator+=(ExprOr const& e)
{
	emplace_arg(e);
	assert(std::is_sorted(args().begin(), args().end()));
	fix_unary();
	return *this;
}

pa::Expr& pa::ExprAdd::operator+=(ExprESF&& e)
{
	emplace_arg(std::move(e));
	assert(std::is_sorted(args().begin(), args().end()));
	fix_unary();
	return *this;
}

pa::Expr& pa::ExprAdd::operator+=(ExprESF const& e)
{
	emplace_arg(e);
	assert(std::is_sorted(args().begin(), args().end()));
	fix_unary();
	return *this;
}

pa::Expr& pa::ExprAdd::operator+=(ExprMul&& e)
{
	emplace_arg(std::move(e));
	assert(std::is_sorted(args().begin(), args().end()));
	fix_unary();
	return *this;
}

pa::Expr& pa::ExprAdd::operator+=(ExprMul const& e)
{
	emplace_arg(e);
	assert(std::is_sorted(args().begin(), args().end()));
	fix_unary();
	return *this;
}

pa::Expr& pa::ExprAdd::operator+=(ExprImm const& e)
{
	if (e.value() == 1) {
		emplace_arg(e);
	}
	assert(std::is_sorted(args().begin(), args().end()));
	fix_unary();
	return *this;
}

pa::Expr& pa::ExprAdd::operator+=(ExprSym const& e)
{
	emplace_arg(e);
	assert(std::is_sorted(args().begin(), args().end()));
	fix_unary();
	return *this;
}

pa::Expr& pa::ExprAdd::operator+=(Expr const& e)
{
	e.call_no_return([this] (auto const& e_) { return *this += e_; });
	return *this;
}

pa::Expr& pa::ExprAdd::operator+=(Expr&& e)
{
	e.call_no_return([this] (auto&& e_) { return *this += std::move(e_); });
	return *this;
}
