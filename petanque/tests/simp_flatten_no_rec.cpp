#include <pa/simps.h>
#include <pa/symbols.h>

#include "tests.h"

using namespace pa;

int main()
{
	// flatten_no_rec will transform an expression like:
	//   ExprAdd(ExprAdd(a, b), ExprAdd(c, ExprAdd(d, e)))
	// into:
	//   ExprAdd(a, b, c, ExprAdd(d, e))
	
	int ret = 0;

	Expr a = symbol("a");
	Expr b = symbol("b");
	Expr c = symbol("c");
	Expr d = symbol("d");
	Expr e = symbol("e");

	{
		Expr exp = ExprAdd({ExprAdd({a, b}), ExprAdd({c, d})});
		simps::flatten_no_rec(exp);
		ret |= check_expr("(a+b) + (c+d)", exp, ExprAdd({a, b, c, d}));
	}

	{
		Expr exp = ExprAdd({ExprImm(true), ExprAdd({a, b}), ExprAdd({c, d})});
		simps::flatten_no_rec(exp);
		ret |= check_expr("1 + (a+b) + (c+d)", exp, ExprAdd({ExprImm(1), a, b, c, d}));
	}

	{
		Expr exp = ExprAdd({ExprAdd({a, b}), ExprAdd({c, ExprAdd({d, e})})});
		simps::flatten_no_rec(exp);
		ret |= check_expr("(a+b) + (c+d)", exp, ExprAdd({a, b, c, ExprAdd({d, e})}));
	}

	{
		Expr exp = ExprAdd({ExprAdd({a, b}), ExprMul({c, d}), e});
		simps::flatten_no_rec(exp);
		ret |= check_expr("a + b + (c*d) + e", exp, ExprAdd({a, b, ExprMul({c, d}), e}));
	}

	{
		Expr exp = ExprMul({ExprMul({a, b}), ExprMul({c, d}), e});
		simps::flatten_no_rec(exp);
		ret |= check_expr("(a*b) * (c*d) * e", exp, ExprMul({a, b, c, d, e}));
	}
}
