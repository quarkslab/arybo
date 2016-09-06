#include <pa/simps.h>
#include <pa/prettyprinter.h>

using namespace pa;

int main()
{
	int ret = 0;
	//((Sym(X0) * Sym(X1) * Sym(X2) * Sym(Y0) * Sym(Y2)) +  // e0
	// (Sym(X0) * Sym(Y0) * Sym(Y1) * Sym(Y2) * Sym(Y3)) +  // e1 == e4
	// (Sym(X0) * Sym(X2) * Sym(Y0) * Sym(Y1) * Sym(Y2)) +  // e2
	// (Sym(X0) * Sym(X2) * Sym(Y0) * Sym(Y1) * Sym(Y3)) +  // e3
	// (Sym(X0) * Sym(Y0) * Sym(Y1) * Sym(Y2) * Sym(Y3)))   // e4 == e1

	Expr X0 = symbol("X0");
	Expr X1 = symbol("X1");
	Expr X2 = symbol("X2");
	Expr X3 = symbol("X3");
	Expr Y0 = symbol("Y0");
	Expr Y1 = symbol("Y1");
	Expr Y2 = symbol("Y2");
	Expr Y3 = symbol("Y3");

	Expr e0 = X0 * X1 * X2 * Y0 * Y2;
	Expr e1 = X0 * Y0 * Y1 * Y2 * Y3;
	Expr e2 = X0 * X2 * Y0 * Y1 * Y2;
	Expr e3 = X0 * X2 * Y0 * Y1 * Y3;
	Expr e4 = X0 * Y0 * Y1 * Y2 * Y3;

	Expr e = e0 + e1 + e2 + e3 + e4;

	std::cout << pretty_print(e0) << std::endl;
	std::cout << pretty_print(e1) << std::endl;
	std::cout << pretty_print(e2) << std::endl;
	std::cout << pretty_print(e3) << std::endl;
	std::cout << pretty_print(e4) << std::endl;

	std::cout << pretty_print(e) << std::endl;

	simps::flatten(e);
	std::cout << pretty_print(e) << std::endl;
	simps::sort(e);

	// Assert sort is stable!!
	pa::Expr eref = e;
	std::cout << pretty_print(e) << std::endl;
	simps::sort(e);

	if (e != eref) {
		std::cerr << "sort isn't stable! some bad things are going to happen... :/ !!" << std::endl;
		ret = 1;
	}

	return ret;
}
