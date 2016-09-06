#include <pa/exprs.h>
#include "tests.h"

int main()
{
	int ret = 0;
	pa::Expr s0 = pa::ExprSym(0);
	pa::Expr s1 = pa::ExprSym(1);
	{
		pa::Expr expr(pa::ExprImm(true) * pa::ExprImm(true));
		ret |= check_imm(expr, true);
	}

	{
		pa::Expr expr(pa::ExprImm(true) * pa::ExprImm(false));
		ret |= check_imm(expr, false);
	}

	{
		pa::Expr expr(pa::ExprImm(false) * pa::ExprImm(true));
		ret |= check_imm(expr, false);
	}

	{
		pa::Expr expr(pa::ExprImm(true) * pa::ExprImm(false));
		ret |= check_imm(expr, false);
	}

	{
		pa::Expr expr(pa::ExprImm(false) * pa::ExprImm(true));
		ret |= check_imm(expr, false);
	}

	{
		pa::Expr s2 = pa::ExprSym(2);
		pa::Expr e0 = s0*s1;
		pa::Expr e1 = e0*s2;
		ret |= check_expr("s0 * s1 * s2", e1, pa::ExprMul({s0, s1, s2}));
	}

	{
		pa::Expr s2 = pa::ExprSym(2);
		pa::Expr e = s0*s1*s2;
		ret |= check_expr("s0 * s1 * s2", e, pa::ExprMul({s0, s1, s2}));
	}

	{
		pa::Expr expr(s0 * s0);
		ret |= check_expr("s0 * s0", expr, s0);
	}

	{
		pa::Expr expr(s0 * pa::ExprImm(true));
		ret |= check_expr("s0 * 1", expr, s0);
	}

	{
		pa::Expr expr(s0 * pa::ExprImm(false));
		ret |= check_expr("s0 * 0", expr, pa::ExprImm(false));
	}

	{
		pa::Expr expr(pa::ExprImm(true) * pa::ExprImm(true));
		ret |= check_expr("1 * 1", expr, pa::ExprImm(true));
	}

	{
		pa::Expr e0(s0 + pa::ExprImm(1));
		pa::Expr e1(s1 + pa::ExprImm(1));

		pa::Expr res0 = e0*e0;
		ret |= check_expr("e0 * e0", res0, e0);

		pa::Expr res1 = e0+e0;
		ret |= check_expr("e0 + e0", res1, pa::ExprImm(0));

		pa::Expr res2 = e0*(s0+pa::ExprImm(1));
		ret |= check_expr("e0*(s0+1)", res2, e0);

		pa::Expr res3 = e0+(s0+pa::ExprImm(1));
		ret |= check_expr("e0+(s0+1)", res0, e0);
	}

	{
		pa::Expr e0 = s0 * s1;
		pa::Expr s2 = pa::ExprSym(2);
		pa::Expr e1 = e0 * s2;
		ret |= check_expr("(s0 * s1) * s2", e1, pa::ExprMul({s0, s1, s2}));
	}

	{
		pa::Expr s2 = pa::ExprSym(2);
		pa::Expr e0 = s0 * (s1 + s2);
		ret |= check_expr("s0 * (s1 + s2)", e0, pa::ExprMul({pa::ExprAdd({s1, s2}), s0}));
	}

	{
		pa::ExprMul e0 = pa::ExprMul({pa::ExprSym(1), pa::ExprSym(2)});
		e0 *= pa::ExprImm(1);
		ret |= check_expr("e0 *= 1", e0, e0);
		e0 *= pa::ExprImm(0);
		ret |= check_expr("e0 *= 0", e0, pa::ExprImm(0));
	}

	{
		pa::ExprMul e0 = pa::ExprMul({pa::ExprSym(1), pa::ExprSym(2)});
		e0 *= pa::ExprSym(3);
		ret |= check_expr("e0 *= ExprSym(3)", e0, pa::ExprMul({pa::ExprSym(1), pa::ExprSym(2), pa::ExprSym(3)}));
	}

	{
		pa::ExprSym e0(3);
		pa::ExprMul e1 = pa::ExprMul({pa::ExprSym(1), pa::ExprSym(2)});
		e0 *= e1;
		ret |= check_expr("ExprSym(..) *= ExprMul(...)", e0, pa::ExprMul({pa::ExprSym(1), pa::ExprSym(2), pa::ExprSym(3)}));
	}

	{
		pa::ExprImm e0(false);
		pa::ExprMul e1 = pa::ExprMul({pa::ExprSym(1), pa::ExprSym(2)});
		e0 *= e1;
		ret |= check_expr("ExprImm(false) *= ExprMul(...)", e0, pa::ExprImm(0));
	}

	{
		pa::ExprSym e0(0);
		pa::ExprSym e1(1);
		e0 *= e1;
		ret |= check_expr("ExprSym(0) *= ExprSym(1)", e0, pa::ExprMul({pa::ExprSym(0), pa::ExprSym(1)}));
	}

	return ret;
}
