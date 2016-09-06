#include <pa/simps.h>
#include <pa/symbols.h>
#include <pa/prettyprinter.h>

#include "tests.h"

using namespace pa;

int main()
{
	int ret = 0;

	Expr a = symbol("a");
	Expr b = symbol("b");
	Expr c = symbol("c");
	Expr d = symbol("d");
	Expr e = symbol("e");
	Expr f = symbol("f");

	{
		Expr exp = ExprMul({ExprAdd({a, b}), ExprAdd({c, d}), e});
		simps::expand(exp);
		ret |= check_expr("(a+b) * (c+d) * e", exp, ExprAdd({ExprMul({a,c,e}), ExprMul({a,d,e}), ExprMul({b,c,e}), ExprMul({b,d,e})}));
	}

	{
		Expr exp = ExprAdd({ExprMul({ExprAdd({a, b}), ExprAdd({c, d}), e}), ExprImm(1)});
		simps::expand(exp);
		ret |= check_expr("(a+b) * (c+d) * e + 1", exp, ExprAdd({ExprAdd({ExprMul({a, c, e}), ExprMul({a, d, e}), ExprMul({b, c, e}), ExprMul({b, d, e})}), ExprImm(1)}));
	}

#if 0
	{
		Expr exp = ExprMul({ExprAdd({ExprImm(0), ExprMul({ExprAdd({a, b}), ExprAdd({c, d}), e}), ExprImm(1)}), ExprImm(1), a, b});
		simps::expand(exp);
		ret |= check_expr("(0 + ((a+b)*(c+d)*e)+1)*1*a*b", exp, ExprMul({a,b}));
	}
#endif

	{
		Expr exp = ExprAdd({ExprMul({a,b,c}), ExprImm(1), ExprMul({ExprAdd({ExprImm(0), ExprMul({ExprAdd({a, b}), ExprAdd({c, d}), e}), ExprImm(1)}), a, b})});
		simps::expand(exp);
		//ret |= check_expr("(0 + ((a+b)*(c+d)*e)+1)*1*a*b", exp, ExprMul({a,b}));
		std::cout << pa::pretty_print(exp) << std::endl;
	}

	{
		Expr exp = ExprMul({a,b,c});
		simps::expand(exp);
		std::cout << pa::pretty_print(exp) << std::endl;
	}

	{
		Expr exp = ExprMul({a, ExprAdd({ExprMul({a,b,c}), ExprImm(1), ExprMul({ExprAdd({ExprImm(0), ExprMul({ExprAdd({a, b}), ExprAdd({c, d}), e}), ExprImm(1)}), a, b})})});
		std::cout << pa::pretty_print(exp) << std::endl;
		simps::expand(exp);
		std::cout << pa::pretty_print(exp) << std::endl;
	}

	{
		Expr exp = ExprMul({ExprAdd({ExprImm(0), ExprImm(1)}), ExprAdd({ExprImm(0), ExprImm(1)})});
		simps::expand(exp);
		std::cout << pa::pretty_print(exp) << std::endl;
		ret |= check_expr("(0+1) * (0+1)", exp, ExprAdd({ExprImm(1)}));
	}

	return ret;
}
