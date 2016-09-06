#include <pa/prettyprinter.h>
#include <pa/exprs.h>

#include <iostream>
#include <sstream>

int check_pp(pa::Expr const& e, const char* v, pa::Symbols const* syms = nullptr)
{
	std::stringstream ss;
	ss << pa::pretty_print(e, syms);
	std::string res = ss.str();
	if (res != v) {
		std::cerr << "Error: pretty printer result was '" << res.c_str() << "', expected '" << v << "'" << std::endl;
		return 1;
	}
	else {
		std::cerr << res.c_str() << std::endl;
	}
	return 0;
}
int main()
{
	int ret = 0;
	pa::pp_sym_display(true);

	{
		pa::Expr e = pa::ExprImm{true};
		ret |= check_pp(e, "1");
	}

	{
		pa::Expr e = pa::ExprAdd({pa::ExprImm(true), pa::ExprSym(0)});
		ret |= check_pp(e, "(Sym(0) + 1)");
	}

	{
		pa::Expr e = pa::ExprAdd({pa::ExprImm(true), pa::ExprSym(0), pa::ExprSym(1), pa::ExprImm(false)});
		ret |= check_pp(e, "(Sym(0) + Sym(1) + 0 + 1)");
	}

	{
		pa::Expr e = pa::ExprAdd({pa::ExprImm(true), pa::ExprImm(true)});
		ret |= check_pp(e, "(1 + 1)");
	}

	{
		pa::Expr e = pa::ExprAdd({pa::ExprSym(0), pa::ExprMul({pa::ExprSym(0), pa::ExprSym(1)})});
		ret |= check_pp(e, "((Sym(0) * Sym(1)) + Sym(0))");
	}

	{
		pa::Symbols syms;
		pa::Expr s0 = syms.symbol("S0");
		pa::Expr s1 = syms.symbol("S1");
		pa::ExprAdd e({s0, s1});
		ret |= check_pp(e, "(Sym(S0) + Sym(S1))", &syms);
	}

	return ret;
}
