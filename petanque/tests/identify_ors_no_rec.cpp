#include <pa/simps.h>
#include <pa/symbols.h>

#include "tests.h"

using namespace pa;

static int check_ors(pa::Expr const& ors)
{
	pa::Expr e = ors;
	simps::or_to_esf(e);
	simps::flatten(e);
	simps::sort(e);

	return check_expr(e, ors, simps::identify_ors_no_rec);
}

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
		Expr exp = ExprOr({a,b});
		ret |= check_ors(exp);
	}

	{
		Expr exp = ExprOr({a,b,c});
		ret |= check_ors(exp);
	}

	{
		Expr exp = ExprOr({a,b,c,d});
		ret |= check_ors(exp);
	}

	{
		Expr exp = ExprAdd({ExprOr({a,b,c,d}), e});
		ret |= check_ors(exp);
	}

	{
		Expr exp = ExprAdd({ExprOr({a,b,c,d}), e, ExprImm(true)});
		ret |= check_ors(exp);
	}

	{
		Expr exp = ExprAdd({e*f, ExprOr({a,b,c,d}), e, ExprImm(true)});
		pa::simps::sort(exp);
		ret |= check_ors(exp);
	}

	{
		Expr exp = ExprAdd({e*f, ExprESF(2, {a,b,e}), ExprOr({a,b,c,d}), e, ExprImm(true)});
		pa::simps::sort(exp);
		ret |= check_ors(exp);
	}

	{
		Expr exp = ExprAdd({ExprOr({a,b}), ExprOr({c,d})});
		ret |= check_ors(exp);
	}

	{
		Expr exp = ExprAdd({ExprOr({a,b}), ExprOr({c,d}), ExprOr({e, f})});
		ret |= check_ors(exp);
	}

	{
		Expr exp = ExprAdd({ExprOr({a,b,c}), ExprOr({d,e,f}), ExprImm(true)});
		ret |= check_ors(exp);
	}

	{
		Expr exp = ExprAdd{{ExprOr{{a,d}}, b, ExprImm{true}}};
		ret |= check_ors(exp);
	}

	{
		Expr exp = ExprAdd{{ExprOr{{a,b,d}}, c, ExprImm{true}}};
		ret |= check_ors(exp);
	}

	return 0;
}
