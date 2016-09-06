#ifndef PA_TESTS_H
#define PA_TESTS_H

#include <iostream>
#include <functional>

#include <pa/exprs.h>

#if defined _WIN32 || defined __CYGWIN__
  #define PATESTS_HELPER_IMPORT __declspec(dllimport)
  #define PATESTS_HELPER_EXPORT __declspec(dllexport)
  #define PATESTS_HELPER_LOCAL
#else
  #define PATESTS_HELPER_IMPORT __attribute__ ((visibility ("default")))
  #define PATESTS_HELPER_EXPORT __attribute__ ((visibility ("default")))
  #define PATESTS_HELPER_LOCAL  __attribute__ ((visibility ("hidden")))
#endif

#ifdef patests_EXPORTS
  #define PATESTS_API PATESTS_HELPER_EXPORT
  #define PATESTS_LOCAL PATESTS_HELPER_LOCAL
  #define PATESTS_TEMPLATE_EXPIMP
#else
  #define PATESTS_API PATESTS_HELPER_IMPORT
  #define PATESTS_LOCAL PATESTS_HELPER_LOCAL
  #define PATESTS_TEMPLATE_EXPIMP extern
#endif

PATESTS_API int check_expr(pa::Expr const& e_, pa::Expr const& ref, std::function<void(pa::Expr&)> const& F);
PATESTS_API int check_expr(const char* expr, pa::Expr const& v, pa::Expr const& ref);
PATESTS_API int check_imm(pa::Expr const& e, bool v);
PATESTS_API int check_name(pa::Expr const& e, const char* name);

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
