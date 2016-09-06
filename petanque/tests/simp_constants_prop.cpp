#include <pa/simps.h>
#include <pa/symbols.h>

#include "tests.h"

using namespace pa;

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
		Expr exp = ExprMul({a, ExprMul({b, c}), a, b, ExprImm(0)});
		simps::constants_prop(exp);
		ret |= check_expr("a * (b*c) * a * b * 0", exp, ExprImm(0));
	}

	return ret;
}
