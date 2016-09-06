#include <cstring>
#include <iostream>

#include <pa/exprs.h>

const char* test()
{
	pa::ExprImm eimm(true);
	return eimm.call([] (auto const& e) { return e.name(); });
}

int main()
{
	int ret = 0;
	std::cout << sizeof(pa::Expr) << std::endl;

	pa::Expr eimm = pa::ExprImm(true);
	pa::Expr esym = pa::ExprSym(0);
	pa::Expr eadd = pa::ExprAdd({eimm, esym});
	pa::Expr emul = pa::ExprMul({eimm, esym});

	if (strcmp(eimm.name(), "Imm") != 0) {
		std::cerr << "invalid name for ExprImm!" << std::endl;
		ret = 1;
	}
	if (strcmp(emul.name(), "Mul") != 0) {
		std::cerr << "invalid name for ExprMul!" << std::endl;
		ret = 1;
	}

	pa::Expr e2 = emul;
	eimm = emul;
	pa::Expr e3 = std::move(eimm);

	if (strcmp(e3.name(), "Mul") != 0) {
		std::cerr << "invalid name after move!" << std::endl;
		ret = 1;
	}

	if (strcmp(e2.name(), "Mul") != 0) {
		std::cerr << "invalid name after copy!" << std::endl;
		ret = 1;
	}


	const char* e2_call = e2.call([] (auto const& e) { return e.name(); });
	if (strcmp(e2_call, "Mul") != 0) {
		std::cerr << "invalid name after call!" << std::endl;
		ret = 1;
	}
	const char* e2_call_assert = e2.call_assert([](pa::ExprMul const& e) { return e.name(); });
	if (strcmp(e2_call_assert, "Mul") != 0) {
		std::cerr << "invalid name after call_assert!" << std::endl;
		ret = 1;
	}

	const char* e2_call_if;
	e2.call_if([&e2_call_if](pa::ExprMul const& e) { e2_call_if = e.name(); });
	if (strcmp(e2_call_if, "Mul") != 0) {
		std::cerr << "invalid name after call_if!" << std::endl;
		ret = 1;
	}

	return ret;
}
