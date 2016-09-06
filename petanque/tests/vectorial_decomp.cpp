#include <pa/analyses.h>
#include <pa/prettyprinter.h>
#include <pa/simps.h>

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
	Expr f = symbol("f");
	Expr g = symbol("g");
	Expr h = symbol("h");

	Vector X{{a, b, c, d, e, f, g, h}};
	Vector F{{a + b + ExprImm(1),
			  b + c + d,
			  c,
			  ExprImm(1),
			  a*b + e + h,
			  a*b*c*d + a + ExprImm(1),
			  g,
			  h + ExprImm(1)}};

	App app = analyses::vectorial_decomp(X, F);

	Matrix mref(8, {
		ExprImm(1), ExprImm(1), ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(0),
		ExprImm(0), ExprImm(1), ExprImm(1), ExprImm(1), ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(0),
		ExprImm(0), ExprImm(0), ExprImm(1), ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(0),
		ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(0),
		ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(1), ExprImm(0), ExprImm(0), ExprImm(1),
		ExprImm(1), ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(0),
		ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(1), ExprImm(0),
		ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(1)
	});

	Vector vref({ExprImm(1), ExprImm(0), ExprImm(0), ExprImm(1), ExprImm(0), ExprImm(1), ExprImm(0), ExprImm(1)});

	if (app.matrix() != mref) {
		std::cerr << "invalid matrix!" << std::endl;
		ret = 1;
	}

	if (app.cst() != vref) {
		std::cerr << "invalid vector!" << std::endl;
		ret = 1;
	}

	Vector nlref({
			ExprImm(0),
			ExprImm(0),
			ExprImm(0),
			ExprImm(0),
			ExprMul({Symbols::arg_symbol(0), Symbols::arg_symbol(1)}),
			ExprMul({Symbols::arg_symbol(0), Symbols::arg_symbol(1), Symbols::arg_symbol(2), Symbols::arg_symbol(3)}),
			ExprImm(0),
			ExprImm(0)});
	if (app.nl().vector() != nlref) {
		std::cerr << "invalid non linear part!" << std::endl;
		ret = 1;
	}

	Vector test = app(X);
	simps::simplify(test);
	if (test != F) {
		std::cerr << "app(X) != original vector" << std::endl;
		ret = 1;
	}

	return ret;
}
