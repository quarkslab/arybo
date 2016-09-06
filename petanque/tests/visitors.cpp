#include <pa/visitors.h>
#include <pa/prettyprinter.h>
#include <pa/symbols.h>

#include <iostream>

using namespace pa;

struct visit_replace_sym_inplace: public pa::visitor_inplace<visit_replace_sym_inplace>
{
	pa::ExprSym::idx_type org_;
	pa::ExprSym::idx_type by_;

	visit_replace_sym_inplace(ExprSym const& org, ExprSym const& by):
		org_(org.idx()),
		by_(by.idx())
	{ }

	bool with_args(pa::Expr& e)
	{
		// Depth first
		return this->visit_args(e);
	}

	bool sym(pa::ExprSym& e)
	{
		if (e.idx() == org_) {
			e = ExprSym{by_};
			return true;
		}
		return false;
	}
};

struct visit_replace_sym: public pa::visitor<visit_replace_sym>
{
	pa::ExprSym::idx_type org_;
	pa::ExprSym::idx_type by_;

	visit_replace_sym(ExprSym const& org, ExprSym const& by):
		org_(org.idx()),
		by_(by.idx())
	{ }

	pa::Expr with_args(pa::Expr const& e)
	{
		// Depth first
		return this->visit_args(e);
	}

	pa::Expr sym(pa::ExprSym const& e)
	{
		if (e.idx() == org_) {
			return ExprSym{by_};
		}
		return e;
	}
};

int main()
{
	ExprSym a = symbol("a");
	ExprSym b = symbol("b");
	ExprSym c = symbol("c");
	ExprSym d = symbol("d");

	Expr e = a+b;
	visit_replace_sym_inplace v1(a, c);
	v1.visit(e);
	visit_replace_sym_inplace v2(b, d);
	v2.visit(e);
	if (e != c+d) {
		std::cerr << "error while replacing symbols inplace: " << pa::pretty_print(e) << std::endl;
		return 1;
	}

	e = a+b;
	visit_replace_sym v1_(a, c);
	e = v1_.visit(e);
	visit_replace_sym v2_(b, d);
	e = v2_.visit(e);
	if (e != c+d) {
		std::cerr << "error while replacing symbols: " << pa::pretty_print(e) << std::endl;
		return 1;
	}

	return 0;
}
