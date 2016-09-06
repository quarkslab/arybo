#include <pa/exprs.h>
#include <pa/prettyprinter.h>

#include "tests.h"

int main()
{
	int ret = 0;
	pa::pp_sym_display(true);

	pa::Expr s0 = pa::ExprSym(0);
	pa::Expr s1 = pa::ExprSym(1);
	{
		pa::Expr expr(pa::ExprImm(true) | pa::ExprImm(true));
		ret |= check_imm(expr, true);
	}

	{
		pa::Expr expr(pa::ExprImm(true) | pa::ExprImm(false));
		ret |= check_imm(expr, true);
	}

	{
		pa::Expr expr(pa::ExprImm(false) | pa::ExprImm(false));
		ret |= check_imm(expr, false);
	}

	{
		pa::Expr expr(pa::ExprImm(false) | pa::ExprImm(true));
		ret |= check_imm(expr, true);
	}


	{
		pa::Expr s2 = pa::ExprSym(2);
		pa::Expr e0 = s0|s1;
		pa::Expr e1 = e0|s2;
		ret |= check_expr("s0 | s1 | s2", e1, pa::ExprOr({s0, s1, s2}));
	}

	{
		pa::Expr s2 = pa::ExprSym(2);
		pa::Expr e = s0|s1|s2;
		ret |= check_expr("s0 | s1 | s2", e, pa::ExprOr({s0, s1, s2}));
	}

	{
		pa::Expr expr(s0 | s0);
		ret |= check_expr("s0 | s0", expr, s0);
	}

	{
		pa::Expr expr(s0 | pa::ExprImm(true));
		ret |= check_expr("s0 | 1", expr, pa::ExprImm(true));
	}

	{
		pa::Expr expr(s0 | pa::ExprImm(false));
		ret |= check_expr("s0 | 0", expr, s0);
	}

	{
		pa::Expr e0(s0 + pa::ExprImm(1));
		pa::Expr res0 = e0|e0;
		ret |= check_expr("e0 | e0", res0, e0);
	}

	return ret;
}
