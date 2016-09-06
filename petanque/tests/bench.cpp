#include <pa/simps.h>
#include <pa/prettyprinter.h>

#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv)
{
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " n" << std::endl;
		return 1;
	}

	srand(time(NULL));

	const size_t n = atoll(argv[1]);
	pa::Expr a = pa::symbol("a");
	pa::Expr b = pa::symbol("b");
	pa::Expr e = pa::ExprAdd(a, b);
	for (size_t i = 0; i < n; i++) {
		switch (rand()%7) {
			case 0:
				e = pa::ExprAdd(e, a);
				break;
			case 1:
				e = pa::ExprAdd(e, b);
				break;
			case 2:
				e = pa::ExprMul(e, a);
				break;
			case 3:
				e = pa::ExprMul(e, b);
				break;
			case 4:
				e = pa::ExprMul(e, pa::ExprAdd(b, a));
				break;
			case 5:
				e = pa::ExprAdd(e, pa::ExprImm(1));
				break;
			case 6:
				e = pa::ExprAdd(e, pa::ExprMul(a, b));
				break;
		}
	}

	//std::cout << pa::pretty_print(e) << std::endl;
	pa::simps::expand(e);
	std::cout << pa::pretty_print(e) << std::endl;

	return 0;
}
