#include <pa/exprs.h>
#include "tests.h"

#include <iostream>

int main()
{
	int ret = 0;
	pa::Expr s0 = pa::ExprSym(0);
	pa::Expr s1 = pa::ExprSym(1);
	pa::Expr s2 = pa::ExprSym(2);
	{
		pa::Expr expr(pa::ExprImm(true) + pa::ExprImm(true));
		ret |= check_imm(expr, false);
	}

	{
		pa::Expr a0(pa::ExprImm(true));
		pa::Expr a1(pa::ExprImm(false));

		pa::Expr expr = a0+a1;
		ret |= check_imm(expr, true);
	}

	{
		pa::Expr a0(pa::ExprImm(true));
		pa::Expr a1(pa::ExprImm(false));

		a0 += a1;
		ret |= check_imm(a0, true);
	}

	{
		pa::Expr expr = pa::ExprImm(true) + pa::ExprImm(true);
		ret |= check_imm(expr, false);
	}

	{
		pa::Expr expr = pa::ExprSym(0) + pa::ExprImm(true);
		printf("%s\n", expr.name());
	}

	{
		pa::Expr expr = pa::ExprSym(0) + pa::ExprSym(0);
		ret |= check_imm(expr, false);
	}

	{
		pa::Expr e0 = pa::ExprAdd({pa::ExprSym(0), pa::ExprSym(1)});
		pa::Expr e1 = pa::ExprAdd({pa::ExprSym(2), pa::ExprSym(3)});
		pa::Expr expr = e0 + e1;
		ret |= expr.call_assert([](pa::ExprAdd const& e)
				{
					const size_t nargs = e.args().size();
					std::cout << "nargs: " << nargs << std::endl;
					if (nargs != 4) {
						std::cerr << "error: got " << nargs << ", expected 4!" << std::endl;
						return 1;
					}
					return 0;
				});
	}

	{
		pa::Expr e0 = pa::ExprAdd({pa::ExprSym(0), pa::ExprSym(1)}) + pa::ExprImm(1);
		ret |= check_type<pa::ExprAdd>(e0);
		if (e0.nargs() != 3) {
			std::cerr << "error: expected 3 args, got " << e0.nargs() << std::endl;
			ret = 1;
		}
	}

	{
		pa::Expr e0 = pa::ExprAdd({pa::ExprSym(0), pa::ExprSym(1)}) + pa::ExprMul({pa::ExprSym(0), pa::ExprSym(1)});
		ret |= check_type<pa::ExprAdd>(e0);
		if (e0.nargs() != 3) {
			std::cerr << "error: expected 3 args, got " << e0.nargs() << std::endl;
			ret = 1;
		}
	}

	{
		pa::Expr e0 = s0;
		pa::Expr e1 = s0;
		e0 += e1;
		ret |= check_imm(e0, false);
	}

	{
		pa::Expr e0 = s0;
		pa::Expr e1 = s1;
		e0 += e1;
		ret |= check_type<pa::ExprAdd>(e0);
	}

	{
		pa::Expr e0 = s0;
		pa::Expr e1 = s1;
		e0 += e1;
		ret |= check_type<pa::ExprAdd>(e0);
	}

	{
		pa::Expr e0 = s0 + s1;
		e0 += e0;
		ret |= check_imm(e0, 0);
	}

	pa::Expr esf0 = pa::ExprESF{2, {s0, s1, s2}};
	{
		pa::Expr e0 = s0+s1;
		e0 += esf0;
	}
	{
		pa::Expr e0 = s0|s1;
		e0 += esf0;
	}
	{
		pa::Expr e0 = s0*s1;
		e0 += esf0;
	}

	return ret;
}
