#include <pa/bitfield.h>
#include <pa/simps.h>
#include <pa/subs.h>
#include <pa/prettyprinter.h>

#include <array>
#include <map>

using namespace pa;

int check_imm(pa::Expr const& e, bool v)
{
	int ret = 0;
	bool e_v;
	e.call_assert([&ret, &e_v, v](pa::ExprImm const& e) { e_v = e.value(); ret = !(e_v == v); });
	if (ret == 1) {
		std::cerr << "immediate value is " << e_v << ", expected " << v << std::endl;
	}
	return ret;
}

int main()
{
	int ret = 0;

	Expr a = symbol("a");
	Expr b = symbol("b");
	Expr c = symbol("c");
	Expr d = symbol("d");

	std::array<Expr, 4> X = {{a, b, c, d}};

	{
		Expr exp = ExprMul({ExprAdd({a, b}), ExprAdd({c, d})});
		subs(exp, X, std::array<bool, 4>{{0, 1, 0, 1}});
		simps::simplify(exp);
		ret |= check_imm(exp, 1);
	}

	{
		Expr exp = ExprMul({ExprAdd({a, b}), ExprAdd({c, d})});
		std::array<decltype(X), 1> xs{{X}};
		std::array<size_t, 1> values{{14}};
		subs_vectors(exp, xs, values);
		simps::simplify(exp);
		ret |= check_imm(exp, 0);
	}

	{
		Vector V({a, b, c, d});
		subs_vectors(V, std::array<decltype(X), 1>{{X}}, std::array<size_t, 1>{{14}});
		if (V != Vector({ExprImm(0), ExprImm(1), ExprImm(1), ExprImm(1)})) {
			std::cerr << "invalid res for subs_vectors!" << std::endl;
			ret = 1;
		}
	}

	{
		Matrix M(4, 4, a);
		subs_vectors(M, std::array<decltype(X), 1>{{X}}, std::array<size_t, 1>{{14}});
		if (M != Matrix(4, 4, ExprImm(0))) {
			std::cerr << "invalid res for subs_vectors with a matrix!" << std::endl;
			ret = 1;
		}
	}

	{
		Expr exp = ExprAdd({ExprMul({a, b}), ExprImm(1)});
		std::map<Expr, Expr> map;
		map[a] = b;
		subs_exprs(exp, map);
		Expr ref = ExprAdd({ExprMul({b, b}), ExprImm(1)});
		if (exp != ref) {
			std::cerr << "invalid res for subs_exprs" << std::endl;
			ret = 1;
		}
	}

	{
		Expr exp = ExprAdd({ExprMul({a, b}), ExprImm(1)});
		std::map<Expr, Expr> map;
		map[ExprMul({a, b})] = ExprImm(0);
		subs_exprs(exp, map);
		Expr ref = ExprAdd({ExprImm(0), ExprImm(1)});
		if (exp != ref) {
			std::cerr << "invalid res for subs_sym" << std::endl;
			ret = 1;
		}
	}

	return ret;
}
