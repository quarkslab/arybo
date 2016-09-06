#include <pa/vector.h>
#include <pa/matrix.h>
#include <pa/prettyprinter.h>
#include <pa/simps.h>

using namespace pa;

int main()
{
	int ret = 0;
	Expr a = symbol("a");
	Expr b = symbol("b");

	Vector va({{a, b, ExprImm(1), ExprImm(0)}});

	auto mv = Matrix::identity(4) * va;
	simps::simplify(mv);
	if (va != mv) {
		std::cerr << "Error on identity(4) * va" << std::endl;
		ret = 1;
	}
	std::cout << "identity(4) * va = " << pretty_print(mv) << std::endl;

	Matrix m1(4, {ExprImm(1), ExprImm(0), ExprImm(1), ExprImm(0), 
	                  ExprImm(1), ExprImm(1), ExprImm(0), ExprImm(0),
					  ExprImm(0), ExprImm(0), ExprImm(0), ExprImm(1),
					  ExprImm(1), ExprImm(1), ExprImm(1), ExprImm(1)});

	Matrix minv(4, {ExprImm(1), ExprImm(1), ExprImm(1), ExprImm(1), 
	                    ExprImm(1), ExprImm(0), ExprImm(1), ExprImm(1),
					    ExprImm(0), ExprImm(1), ExprImm(1), ExprImm(1),
					    ExprImm(0), ExprImm(0), ExprImm(1), ExprImm(0)});

	Matrix m = m1*minv;
	simps::simplify(m);
	if (m != Matrix::identity(4)) {
		std::cerr << "Error on M*inv(M), expected identity, got " << pretty_print(m) << std::endl;
		ret = 1;
	}

	Matrix m1inv = m1.inverse();
	m = m1*m1inv;
	simps::simplify(m);
	if (m != Matrix::identity(4)) {
		std::cerr << "Error on M*inv(M), expected identity, got " << pretty_print(m) << std::endl;
		ret = 1;
	}

	return ret;
}
