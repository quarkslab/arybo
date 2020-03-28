#ifndef PA_TESTS_H
#define PA_TESTS_H

#include <iostream>
#include <functional>

#include <pa/exprs.h>

int check_expr(pa::Expr const& e_, pa::Expr const& ref, std::function<void(pa::Expr&)> const& F);
int check_expr(const char* expr, pa::Expr const& v, pa::Expr const& ref);
int check_imm(pa::Expr const& e, bool v);
int check_name(pa::Expr const& e, const char* name);

template <class E>
static int check_type(pa::Expr const& e)
{
	if (e.type() != (E::type_id)) {
		std::cerr << "Bad type!" << std::endl;
		return 1;
	}
	return 0;
}


#endif
