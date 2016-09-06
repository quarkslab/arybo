#include <pa/app.h>
#include <pa/prettyprinter.h>
#include <pa/symbols.h>

using namespace pa;

int main()
{
	int ret = 0;

	Expr x = symbol("x");
	Expr y = symbol("y");
	Expr z = symbol("z");
	Expr t = symbol("t");

	Vector syms({x, y, z, t}); 
	Vector v({x + ExprImm(1), y + ExprImm(1), z + ExprImm(1), t + ExprImm(1)});

	VectorApp app(syms, v);
	{
		Vector ref({Symbols::arg_symbol(0) + ExprImm(1), Symbols::arg_symbol(1) + ExprImm(1), Symbols::arg_symbol(2) + ExprImm(1), Symbols::arg_symbol(3) + ExprImm(1)});
		if (app.vector() != ref) {
			std::cerr << "Error while creating VectorApp: bad vector computed!" << std::endl;
			ret = 1;
		}
	}

	{
		Vector test = app(syms);
		if (test != v) {
			std::cerr << "Error applying VectorApp: bad result" << std::endl;
			ret = 1;
		}
	}

	return ret;
}
