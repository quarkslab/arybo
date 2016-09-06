#include <pa/simps.h>
#include <pa/prettyprinter.h>

using namespace pa;

int main()
{
//	Expr X0 = symbol("X0");
//	Expr X1 = symbol("X1");
//	Expr X2 = symbol("X2");
//	Expr X3 = symbol("X3");
//	Expr X4 = symbol("X4");
//	Expr Y0 = symbol("Y0");
//	Expr Y1 = symbol("Y1");
//	Expr Y2 = symbol("Y2");
//	Expr Y3 = symbol("Y3");
//	Expr Y4 = symbol("Y4");
//
//	Expr e = 
//	((X0 * X1 * Y0 * Y1) +
//	 (X0 * X3 * Y0 * Y2) + 
//	 (X0 * X1 * Y0 * Y2) + 
//	 (X0 * Y0 * Y2 * Y3) + 
//	 (X1 * X2 * Y1 * Y2) + 
//	 (X0 * X1 * Y0 * Y2) + 
//	 (X0 * X2 * X3 * Y0) + 
//	 (X0 * Y0 * Y1 * Y3) + 
//	 (X1 * X2 * X3 * Y1) + 
//	 (X0 * X1 * X2 * Y0) + 
//	 (X1 * X2 * Y1 * Y2) + 
//	 (X0 * X1 * X2 * Y0) + 
//	 (X0 * X3 * Y0 * Y2) + 
//	 (X1 * X2 * Y1 * Y2) + 
//	 (X0 * X2 * Y0 * Y2) + 
//	 (X1 * X2 * Y1 * Y3) + 
//	 (X0 * X2 * Y0 * Y1) + 
//	 (X0 * Y0 * Y1 * Y2) + 
//	 (X1 * X2 * X3 * Y1) + 
//	 (X0 * Y0 * Y1 * Y2) + 
//	 (X0 * X1 * X3 * Y0) + 
//	 (X0 * X3 * Y0 * Y1) + 
//	 (X0 * X2 * Y0 * Y3) + 
//	 (X0 * X2 * Y0 * Y3) + 
//	 (X0 * X1 * X3 * Y0) + 
//	 (X0 * Y0 * Y1 * Y3) + 
//	 (X0 * X1 * Y0 * Y1) + 
//	 (X0 * X1 * Y0 * Y3) + 
//	 (X0 * Y0 * Y1 * Y2) + 
//	 (X0 * X1 * Y0 * Y2) + 
//	 (X0 * X3 * Y0 * Y1) + 
//	 (X0 * X2 * X3 * Y0) + 
//	 (X0 * X2 * Y0 * Y1) + 
//	 (X0 * X2 * Y0 * Y2) + 
//	 (X0 * X1 * Y0 * Y2) + 
//	 (X0 * Y0 * Y2 * Y3) + 
//	 (X0 * Y0 * Y1 * Y2) + 
//	 (X1 * X2 * Y1 * Y2) + 
//	 (X0 * X1 * Y0 * Y3) + 
//	 (X1 * X2 * Y1 * Y3) + 
//	 (X0 * X1 * Y0 * Y1 * Y2) + 
//	 (X0 * X2 * Y0 * Y1 * Y3) + 
//	 (X0 * X1 * X2 * Y0 * Y1) + 
//	 (X0 * X1 * Y0 * Y1 * Y3) + 
//	 (X0 * X1 * X3 * Y0 * Y2) + 
//	 (X0 * X1 * X2 * Y0 * Y2) + 
//	 (X0 * X1 * X2 * Y0 * Y3) + 
//	 (X0 * X1 * X2 * Y0 * Y1) + 
//	 (X0 * X1 * Y0 * Y2 * Y3) + 
//	 (X0 * Y0 * Y1 * Y2 * Y3) + 
//	 (X0 * X3 * Y0 * Y1 * Y2) + 
//	 (X0 * X1 * Y0 * Y1 * Y2) + 
//	 (X0 * X1 * X3 * Y0 * Y1) + 
//	 (X0 * X1 * X2 * X3 * Y0) + 
//	 (X0 * X2 * X3 * Y0 * Y1) + 
//	 (X0 * X1 * Y0 * Y1 * Y2) + 
//	 (X0 * X1 * Y0 * Y1 * Y3) + 
//	 (X0 * X2 * Y0 * Y1 * Y2) + 
//	 (X0 * X1 * X2 * Y0 * Y1) + 
//	 (X0 * X2 * Y0 * Y1 * Y3) + 
//	 (X0 * X2 * Y0 * Y1 * Y2) + 
//	 (X0 * X1 * Y0 * Y1 * Y2) + 
//	 (X0 * Y0 * Y1 * Y2 * Y3) + 
//	 (X0 * X1 * Y0 * Y2 * Y3) + 
//	 (X0 * X2 * X3 * Y0 * Y1) + 
//	 (X0 * X1 * X2 * X3 * Y0) + 
//	 (X0 * X1 * X2 * Y0 * Y1) + 
//	 (X0 * X1 * X2 * Y0 * Y3) + 
//	 (X0 * X1 * X3 * Y0 * Y2) + 
//	 (X0 * X3 * Y0 * Y1 * Y2) + 
//	 (X0 * X1 * X2 * Y0 * Y2) + 
//	 (X0 * X1 * X3 * Y0 * Y1) + 
//	 X4 + 
//	 Y4);
//
//  #if 0
//	Expr e = 
//	((X0 * X2 * Y0 * Y1) +
//	 (X0 * X3 * Y0 * Y2) + 
//	 (X0 * X2 * Y0 * Y2) + 
//	 (X0 * X3 * Y0 * Y2) + 
//	 (X0 * X2 * Y0 * Y2) + 
//	 (X0 * X2 * Y0 * Y1) +
//	 X4 + 
//	 Y4);
//	 #endif
//
//	std::cout << pa::pretty_print(e) << std::endl;
//	pa::simps::flatten(e);
//	//std::cout << pa::pretty_print(e) << std::endl;
//	pa::simps::sort(e);
//	std::cout << pa::pretty_print(e) << std::endl;
//	pa::simps::remove_dead_ops(e);
//	std::cout << pa::pretty_print(e) << std::endl;
//	
//	Expr e1 = (X0 * X3 * Y0 * Y2);
//	Expr e2 = (X0 * X3 * Y0 * Y2);
//	std::cout << (e1 == e2) << std::endl;
//	std::cout << (e1 < e2) << std::endl;
//
//	{
//		std::cout << "--" << std::endl;
//		Expr ea = X0 + X1 + X2;
//		Expr eb = X0 + X1 + X2 + Y0;
//		Expr ee = eb * ea * (X4 + pa::ExprImm(1));
//		pa::simps::flatten(ee);
//		std::cout << pa::pretty_print(ee) << std::endl;
//		pa::simps::sort(ee);
//		std::cout << pa::pretty_print(ee) << std::endl;
//	}

	int ret = 0;

	Expr C0 = symbol("C0");
	Expr C1 = symbol("C1");
	Expr C2 = symbol("C2");
	Expr C3 = symbol("C3");
	Expr C4 = symbol("C4");
	Expr D0 = symbol("D0");
	Expr D1 = symbol("D1");
	Expr D2 = symbol("D2");
	Expr D3 = symbol("D3");
	Expr D4 = symbol("D4");

	std::cout << " ---------- " << std::endl << std::endl;
	{
		//Expr e = ExprAdd(C1 * C2, C0 * D0, C1 * D0);
		Expr e = ExprAdd({(C1 * C2),
				(C0 * D0),
				(C1 * D0),
				(C2 * D0),
				(D0 * D1),
				(D0 * D2),
				(C1 * D3),
				(C2 * D3),
				(C3 * D3),
				(D0 * D3),
				(D2 * D3),
				(C0 * C2 * D1),
				(C0 * D1 * D3),
				(C1 * C2 * D1),
				(C1 * D0 * D1),
				(C2 * D0 * D1),
				(C1 * C2 * D2),
				(C1 * D0 * D2),
				(C1 * D1 * D3),
				(C1 * D2 * D3),
				(C2 * D0 * D2),
				(C2 * D2 * D3),
				(D0 * D1 * D2),
				(D0 * D1 * D3),
				(C0 * C2 * D0 * D1),
				(C0 * C2 * D1 * D2),
				(C1 * C2 * D1 * D2),
				(C1 * D0 * D1 * D2), 
				(C2 * D0 * D1 * D2),
				(C0 * D0 * D1 * D3),
				(C0 * D1 * D2 * D3),
				(C1 * D1 * D2 * D3),
				(D0 * D1 * D2 * D3),
				(C0 * C2 * D0 * D1 * D2),
				(C0 * D0 * D1 * D2 * D3),
				C0, C3, D0, D1, D4,
				(C0 * C3),
				(C0 * C1 * C2),
				(C0 * C1 * C2 * C3),
				C0, C1, C4});

		std::cout << &e.args()[0] << std::endl;
		std::cout << &e.args()[e.nargs()-1] << std::endl;
		pa::simps::sort_no_rec(e);
		std::cout << pa::pretty_print(e) << std::endl;

		// Check sort stability
		pa::Expr eref = e;
		pa::simps::sort_no_rec(e);
		if (e != eref) {
			std::cerr << "sort isn't stable! some bad things are going to happen... :/ !!" << std::endl;
			ret = 1;
		}
	}

	return ret;
}
