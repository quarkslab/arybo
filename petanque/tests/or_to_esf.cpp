#include <pa/exprs.h>
#include <pa/prettyprinter.h>
#include <pa/simps.h>

#include <iostream>

static int check_expr(const char* expr, pa::Expr const& v, pa::Expr const& ref)
{
	if (v != ref) {
		std::cerr << expr << " is invalid, expected ";
		pa::pretty_print(std::cerr, ref);
		std::cerr << ", got ";
		pa::pretty_print(std::cerr, v);
		std::cerr << std::endl;
		return 1;
	}
	else {
		std::cerr << expr << " gave ";
		pa::pretty_print(std::cerr, v);
		std::cerr << " as expected";
		std::cerr << std::endl;
	}
	return 0;
}

int main()
{
	int ret = 0;
	pa::pp_sym_display(true);

	pa::Expr s0 = pa::ExprSym(0);
	pa::Expr s1 = pa::ExprSym(1);
	pa::Expr s2 = pa::ExprSym(2);
	pa::Expr s3 = pa::ExprSym(3);
	pa::Expr s4 = pa::ExprSym(4);

	{
		pa::Expr e = s0|s1;
		pa::simps::or_to_esf(e);
		std::cout << pa::pretty_print(e) << std::endl;
	}

	{
		pa::Expr e = s0|s1|s2;
		pa::simps::or_to_esf(e);
		std::cout << pa::pretty_print(e) << std::endl;
	}

	{
		pa::Expr e = s0|s1|s2|s3;
		pa::simps::or_to_esf(e);
		std::cout << pa::pretty_print(e) << std::endl;
	}

	{
		pa::Expr e = s0|s1|s2|s3|s4;
		pa::simps::or_to_esf(e);
		std::cout << pa::pretty_print(e) << std::endl;
	}

	return ret;
}
