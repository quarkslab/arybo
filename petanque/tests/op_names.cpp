#include <pa/exprs.h>
#include "tests.h"

int main()
{
	int ret = 0;

	pa::Expr m = pa::ExprMul();
	pa::ExprAdd a;
	pa::Expr i(pa::ExprImm(true));
	pa::Expr s(pa::ExprSym(0));

	ret |= check_name(i, "Imm");
	ret |= check_name(s, "Sym");
	ret |= check_name(m, "Mul");
	ret |= check_name(a, "Add");

	return ret;
}
