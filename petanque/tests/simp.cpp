#include <pa/exprs.h>
#include <pa/simps.h>
#include <pa/symbols.h>

#include "tests.h"

using namespace pa;

static int check(Expr const& e, Expr const& ref)
{
	return check_expr(e, ref, [](pa::Expr& e) { simps::simplify(e); });
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
		// ((Sym(X0) * Sym(Y0)) + (Sym(X0) * Sym(Y0) * Sym(X0) * Sym(Y0)) + Sym(X2) + Sym(Y2))
		Expr exp = ExprAdd({ExprMul({a, b}), ExprMul({a, b, a, b}), d, e});
		ret |= check(exp, ExprAdd({d, e}));
	}

	{
		// x[0]*y[0]*(x[0]+y[0])
		Expr exp = ExprMul({a, b, ExprAdd({a, b})});
		ret |= check(exp, ExprImm(false));
	}

	{
		Expr exp = ExprMul({ExprOr({a,b}), c});
		ret |= check(exp, ExprMul({ExprOr({a,b}), c}));
	}

	return ret;
}
