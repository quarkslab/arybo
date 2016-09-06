#include <pa/exprs.h>
#include "tests.h"

int main()
{
	int ret = 0;
	pa::Expr s0 = pa::ExprSym(0);
	pa::Expr s1 = pa::ExprSym(1);

	{
		pa::Expr expr = (s1 * (s0 + s1) + pa::ExprImm(1)) * pa::ExprImm(0);
		ret |= check_expr("(s1 * (s0 + s1) + pa::ExprImm(1)) * pa::ExprImm(0)", expr, pa::ExprImm(0));
	}

	return ret;
}
