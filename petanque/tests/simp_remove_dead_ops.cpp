#include <pa/simps.h>
#include <pa/symbols.h>

#include "tests.h"

using namespace pa;

static int check(Expr const& e, Expr const& ref)
{
	return check_expr(e, ref, &pa::simps::remove_dead_ops);
}

int main()
{
	// simplify will sort the arguments of add and mul operations and remove
	// duplicates.
	
	int ret = 0;

	Expr a = symbol("a");
	Expr b = symbol("b");
	Expr c = symbol("c");
	Expr d = symbol("d");
	Expr e = symbol("e");

	{
		Expr exp = ExprAdd({a, ExprMul({b, c}), a, b});
		ret |= check(exp, ExprAdd({ExprMul({b, c}), b}));
	}

	{
		Expr exp = ExprAdd({a, ExprMul({b, c}), a, b, ExprMul({b, c}), b, a});
		ret |= check(exp, a);
	}

	{
		Expr exp = ExprMul({a, ExprAdd({b, c}), a, b, ExprAdd({b, c}), b, a});
		ret |= check(exp, ExprMul({ExprAdd({b, c}), a, b})); 
	}

	{
		Expr exp = ExprAdd({ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(1)});
		ret |= check(exp, ExprImm(1));
	}

	{
		Expr exp = ExprAdd({a});
		ret |= check(exp, a);
	}

	{
		Expr exp = ExprAdd({a, a});
		ret |= check(exp, ExprImm(0));
	}

	{
		Expr exp = ExprAdd({a, a, a});
		ret |= check(exp, a);
	}

	{
		Expr exp = ExprAdd({a, a, ExprImm(0), a});
		ret |= check(exp, a);
	}

	{
		Expr exp = ExprMul({a});
		ret |= check(exp, a);
	}

	{
		Expr exp = ExprMul({a, a});
		ret |= check(exp, a);
	}

	{
		Expr exp = ExprMul({a, a, a});
		ret |= check(exp, a);
	}

	{
		Expr exp = ExprMul({a, a, a, ExprImm(1)});
		ret |= check(exp, a);
	}

	{
		Expr exp = ExprMul({ExprImm(1), ExprImm(1)});
		ret |= check(exp, ExprImm(1));
	}

	{
		Expr exp = ExprMul({a, ExprImm(1), ExprImm(1), ExprImm(1)});
		ret |= check(exp, a);
	}

	{
		Expr exp = ExprMul({a, b, a, b});
		ret |= check(exp, ExprMul({a, b}));
	}

	return ret;
}
