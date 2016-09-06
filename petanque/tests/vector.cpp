#include <pa/vector.h>
#include <pa/prettyprinter.h>

using namespace pa;

int check_vector(Vector const& v, Vector const& ref)
{
	if (v != ref) {
		std::cerr << "Invalid vector, got " << pretty_print(v) << ", got " << pretty_print(ref) << std::endl;
		return 1;
	}
	return 0;
}

int main()
{
	int ret = 0;

	Expr a = symbol("a");
	Expr b = symbol("b");

	Vector v1(4, ExprImm(1));
	Vector v0(4, ExprImm(0));

	Vector va(4, a);
	Vector vb(4, b);

	ret |= check_vector(va + vb, Vector(4, a + b));
	ret |= check_vector(va + v1, Vector(4, a + ExprImm(1)));
	ret |= check_vector(va + v0, Vector(4, a));
	ret |= check_vector(va + va, Vector(4, ExprImm(0)));
	ret |= check_vector(va*va, va);

	va *= va;
	ret |= check_vector(va, va);

	va += va;
	ret |= check_vector(va, Vector(4, ExprImm(0)));

	Expr c = symbol("c");
	Expr d = symbol("d");

	Vector vs({{a, b, c, d}});

	ret |= check_vector(vs << 2, Vector({{c, d, ExprImm(0), ExprImm(0)}}));
	ret |= check_vector(vs << 4, Vector(4, ExprImm(0)));
	ret |= check_vector(vs >> 1, Vector({{ExprImm(0), a, b, c}}));
	ret |= check_vector(vs >> 4, Vector(4, ExprImm(0)));
	ret |= check_vector(vs >> 5, Vector(4, ExprImm(0)));
	
	return ret;
}
