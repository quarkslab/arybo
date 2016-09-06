#include <pa/exprs.h>
#include <pa/simps.h>
#include <pa/symbols.h>
#include "tests.h"

#include <iostream>

int main()
{
	int ret = 0;

	pa::Expr a = pa::symbol("a");
	pa::Expr b = pa::symbol("b");
	pa::Expr c = pa::symbol("c");
	pa::Expr d = pa::symbol("d");

#if 0
	{
		pa::ExprAdd sf_add{{pa::ExprESF{2, {a, b}}, pa::ExprESF{1, {b, a}}, pa::ExprESF{1, {a, b}}}};
		pa::simps::simplify(sf_add);
		ret |= check_expr("simplify(ESF(2, a, b) + ESF(1, b, a) + ESF(1, a, b))", sf_add, pa::ExprMul({a, b}));
	}
#endif

	{
		pa::ExprESF sf2{1, {a, b, a}};
		pa::simps::simplify(sf2);
		ret |= check_expr("ESF(1, a, b, a)", sf2, b);
	}

	{
		pa::ExprESF sf{1, {a, b, c, d}};
		//sf.expand();
		ret |= check_expr("expand(ESF(1, a, b, c, d))", sf, pa::ExprAdd({a, b, c, d}));
	}

	{
		pa::ExprESF sf{2, {a, b, c, d}};
		sf.expand();
		ret |= check_expr("expand(ESF(2, a, b, c, d))", sf, (a * b) + (a * c) + (a * d) + (b * c) + (b * d) + (c * d));
	}

	{
		pa::ExprESF sf{2, {a, b, c, d}};
		sf.expand();
		ret |= check_expr("expand(ESF(2, a, b, c, d))", sf, (a * b) + (a * c) + (a * d) + (b * c) + (b * d) + (c * d));
	}

	{
		pa::ExprESF sf{4, {a, b, c, d}};
		ret |= check_expr("ESF(4, a, b, c, d)", sf, a*b*c*d);
	}

	return ret;
}
