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
		Expr exp = ExprMul({ExprAdd({a, b}), ExprAdd({c, d})});
		simps::expand_no_rec(exp);
		ret |= check_expr("(a+b) * (c+d)", exp, ExprAdd({a*c, a*d, b*c, b*d}));
	}

	{
		Expr exp = ExprMul({ExprAdd({a, b}), ExprAdd({c, d}), ExprAdd({e, f})});
		simps::expand_no_rec(exp);
		ret |= check_expr("(a+b) * (c+d) * (e+f)", exp, ExprAdd({a*c*e, a*c*f, a*d*e, a*d*f, b*c*e, b*c*f, b*d*e, b*d*f}));
	}

	{
		Expr exp = ExprMul({ExprAdd({a, b}), ExprAdd({c, d}), e});
		simps::expand_no_rec(exp);
		ret |= check_expr("(a+b) * (c+d) * e", exp, ExprAdd({ExprMul({a,c,e}), ExprMul({a,d,e}), ExprMul({b,c,e}), ExprMul({b,d,e})}));
	}

	{
		Expr exp = ExprMul({ExprAdd({ExprImm(0), ExprImm(1)}), ExprAdd({ExprImm(0), ExprImm(1)})});
		simps::expand_no_rec(exp);
		ret |= check_expr("(0+1) * (0+1)", exp, ExprAdd({ExprImm(1)}));
	}

#if 0
	{
		Expr exp = ExprMul({ExprESF(1, {a, b}), ExprAdd({a, b}), ExprAdd({c, d}), e});
		simps::expand_no_rec(exp);
		ret |= check_expr("(a|b) * (a+b) * (c+d) * e", exp, ExprMul({ExprESF(1, {a, b}), ExprAdd({ExprMul({a,c,e}), ExprMul({a,d,e}), ExprMul({b,c,e}), ExprMul({b,d,e})})}));

		if (simps::expand_no_rec(exp)) {
			// Should not have been modified
			std::cerr << "previous expression was modified by expand_no_rec, expected not to." << std::endl;
			ret = 1;
		}
	}
#endif

	{
		Expr exp = ExprMul({ExprESF(1, {a, b}), e});
		simps::expand_no_rec(exp);
		ret |= check_expr("ESF(1, a, b) * e", exp, (a*e)+(b*e));
	}

	{
		Expr exp = ExprMul({ExprOr({a, b}), ExprAdd({a, b}), ExprAdd({c, d})});
		simps::expand_no_rec(exp);
		ret |= check_expr("(a|b) * (a+b) * (c+d)", exp, ExprMul({ExprOr({a, b}), ExprAdd({a*c, a*d, b*c, b*d})}));

		if (simps::expand_no_rec(exp)) {
			// Should not have been modified
			std::cerr << "previous expression was modified by expand_no_rec, expected not to." << std::endl;
			ret = 1;
		}
	}

	{
		Expr exp = ExprMul({ExprOr({a, b}), ExprAdd({a, b}), ExprAdd({c, d}), e});
		simps::expand_no_rec(exp);
		ret |= check_expr("(a|b) * (a+b) * (c+d) * e", exp, ExprMul({ExprOr({a, b}), ExprAdd({ExprMul({a,c,e}), ExprMul({a,d,e}), ExprMul({b,c,e}), ExprMul({b,d,e})})}));

		if (simps::expand_no_rec(exp)) {
			// Should not have been modified
			std::cerr << "previous expression was modified by expand_no_rec, expected not to, got: " << pa::pretty_print(exp) << std::endl;
			ret = 1;
		}
	}

	{
		Expr exp = ExprMul({ExprOr({a, b}), e});
		simps::expand_no_rec(exp);
		ret |= check_expr("(a|b) * e", exp, ExprMul({ExprOr({a, b}), e}));
	}

	return ret;
}
