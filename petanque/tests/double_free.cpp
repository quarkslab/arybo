#include <pa/simps.h>
#include <pa/prettyprinter.h>

using namespace pa;

int main()
{
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

	Expr a = ((C1 * C2) + (C0 * D0) + (C1 * D0) + (C2 * D0) + (D0 * D1) + (D0 * D2) + (C1 * D3) + (C2 * D3) + (C3 * D3) + (D0 * D3) + (D2 * D3) + (C0 * C2 * D1) + (C0 * D1 * D3) + (C1 * C2 * D1) + (C1 * D0 * D1) + (C2 * D0 * D1) + (C1 * C2 * D2) + (C1 * D0 * D2) + (C1 * D1 * D3) + (C1 * D2 * D3) + (C2 * D0 * D2) + (C2 * D2 * D3) + (D0 * D1 * D2) + (D0 * D1 * D3) + (C0 * C2 * D0 * D1) + (C0 * C2 * D1 * D2) + (C1 * C2 * D1 * D2) + (C1 * D0 * D1 * D2) + (C2 * D0 * D1 * D2) + (C0 * D0 * D1 * D3) + (C0 * D1 * D2 * D3) + (C1 * D1 * D2 * D3) + (D0 * D1 * D2 * D3) + (C0 * C2 * D0 * D1 * D2) + (C0 * D0 * D1 * D2 * D3) + C0 + C3 + D0 + D1 + D4);

	Expr b = ((C0 * C3) + (C0 * C1 * C2) + (C0 * C1 * C2 * C3) + C0 + C1 + C4);

	simps::simplify(a);
	simps::simplify(b);


	Expr e = a + b;
	simps::simplify(e);

	{
		Expr a = C0*C1 + ExprImm(1) + D2 + D3 + D4;
		Expr b = (C1*D1 + ExprImm(0)) + D4 + C2 + C0;
		simps::simplify(b);
		Expr c = a * b + b + ExprImm(0);
		simps::simplify(c);
		Expr g = (a + b) * (b + c);
		simps::simplify(g);
		Expr h = (c + a) * (a + b);
		Expr i = (c + b) * (b + c);
		simps::simplify(i);
		Expr j = h+(i+ExprImm(1))+(h+i*ExprImm(1));
		simps::simplify(h);
		std::cout << pretty_print(h) << std::endl;
		Expr z = ExprMul();
		z.args().resize(100, ExprImm(1));
		for (size_t i = 1; i < 90; i++) {
			int i_ = rand() % 4;
			Expr& arg = z.args()[i];
			switch (i_) {
			case 0:
				arg = C0+C1*z.args()[i-1];
				break;
			case 1:
				arg = D4*D2*z.args()[i-1];
				break;
			case 2:
				arg = C4+C2+D0*z.args()[i-1];
				break;
			case 3:
				arg = D1*z.args()[i-1];
				break;
			default:
				break;
			};
		}
		simps::simplify(z);
		std::cout << pretty_print(z) << std::endl;
	}

	return 0;
}
