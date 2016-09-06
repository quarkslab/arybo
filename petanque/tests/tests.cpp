#include <iostream>
#include <string.h>

#include <pa/exprs.h>
#include <pa/prettyprinter.h>
#include <pa/simps.h>

#include "tests.h"

int check_expr(pa::Expr const& e_, pa::Expr const& ref, std::function<void(pa::Expr&)> const& F)
{
	pa::Expr e = e_;
	F(e);
	if (e != ref) {
		std::cerr << pa::pretty_print(e_) << " is invalid, expected " << pa::pretty_print(ref) << ", got " << pa::pretty_print(e) << std::endl;
		return 1;
	}

	std::cerr << pa::pretty_print(e_) << ", gave " << pa::pretty_print(e) << ", as expected." << std::endl;
	return 0;
}

int check_expr(const char* expr, pa::Expr const& v, pa::Expr const& ref)
{
	if (v != ref) {
		std::cerr << expr << " is invalid, expected ";
		pa::pretty_print(std::cerr, ref);
		std::cerr << ", got ";
		pa::pretty_print(std::cerr, v);
		std::cerr << std::endl;
		return 1;
	}
	else {
		std::cerr << expr << " gave ";
		pa::pretty_print(std::cerr, v);
		std::cerr << " as expected";
		std::cerr << std::endl;
	}
	return 0;
}


int check_imm(pa::Expr const& e, bool v)
{
	int ret = 0;
	bool e_v;
	e.call_assert([&ret, &e_v, v](pa::ExprImm const& e) { e_v = e.value(); ret = !(e_v == v); });
	if (ret == 1) {
		std::cerr << "immediate value is " << e_v << ", expected " << v << std::endl;
	}
	return ret;
}

int check_name(pa::Expr const& e, const char* name)
{
	if (strcmp(e.name(), name) != 0) {
		std::cerr << "name of expression is " << e.name() << ", expected " << name << std::endl;
		return 1;
	}
	else {
		std::cerr << "name of expression " << name << " is good." << std::endl;
	}
	return 0;
}
