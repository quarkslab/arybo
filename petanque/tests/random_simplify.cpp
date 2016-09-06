#include <pa/simps.h>
#include <pa/subs.h>
#include <pa/prettyprinter.h>

#include <stdlib.h>
#include <time.h>

#include <array>

int main(int argc, char** argv)
{
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " n" << std::endl;
		return 1;
	}

	int ret = 0;

	srand(time(NULL));

	const size_t n = atoll(argv[1]);
	pa::Expr a = pa::symbol("a");
	pa::Expr b = pa::symbol("b");
	pa::Expr c = pa::symbol("c");
	pa::Expr d = pa::symbol("d");
	pa::Expr e = pa::ExprAdd({a, b});
	for (size_t i = 0; i < n; i++) {
		switch (rand()%10) {
			case 0:
				e = pa::ExprAdd({e, a});
				break;
			case 1:
				e = pa::ExprAdd({e, b});
				break;
			case 2:
				e = pa::ExprMul({e, a});
				break;
			case 3:
				e = pa::ExprMul({e, b});
				break;
			case 4:
				e = pa::ExprMul({e, pa::ExprAdd({b, a})});
				break;
			case 5:
				e = pa::ExprAdd({e, pa::ExprImm(1)});
				break;
			case 6:
				e = pa::ExprAdd({e, pa::ExprMul({a, b})});
				break;
			case 7:
				e = pa::ExprAdd({e, c, d});
				break;
			case 8:
				e = pa::ExprMul({e, a, d, pa::ExprImm(1)});
				break;
			case 9:
				e = pa::ExprMul({e, a, c});
				break;
		}
	}

	pa::Expr eref = e;
	pa::simps::simplify(e);

	std::array<pa::Expr, 4> X = {{a, b, c, d}};

	for (size_t i = 0; i < 16; i++) {
		pa::Expr eref_ = eref;
		pa::subs_vectors(eref_, std::array<decltype(X), 1>{{X}}, std::array<size_t, 1>{{i}});
		pa::Expr e_ = e;
		pa::subs_vectors(e_, std::array<decltype(X), 1>{{X}}, std::array<size_t, 1>{{i}});

		pa::simps::simplify(eref_);
		pa::simps::simplify(e_);

		if (eref_ != e_) {
			std::cerr << "Invalid simplification for: " << std::endl;
			std::cerr << pa::pretty_print(eref) << std::endl;
			ret = 1;
		}
	}

	return ret;
}
