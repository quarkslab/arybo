// Copyright (c) 2016 Adrien Guinet <adrien@guinet.me>
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the <organization> nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PETANQUE_PRETTYPRINTER_H
#define PETANQUE_PRETTYPRINTER_H

#include <pa/exports.h>
#include <pa/exprs.h>
#include <pa/matrix.h>
#include <pa/vector.h>
#include <pa/app.h>
#include <pa/traits.h>
#include <pa/symbols.h>

#include <iostream>

namespace pa {

PA_API extern bool pp_print_sym;

template <class O>
void pretty_print(std::ostream& os, O const& o);

PA_API void pretty_print(std::ostream& os, Expr const& e, Symbols const* symbols);

namespace __impl {

template <class E, bool has_args>
struct pretty_print_args_;

template <class E>
struct expr_math_name
{ };

template <>
struct expr_math_name<ExprAdd>
{
	static constexpr const char* name = " + ";
};

template <>
struct expr_math_name<ExprMul>
{
	static constexpr const char* name = " * ";
};

template <>
struct expr_math_name<ExprOr>
{
	static constexpr const char* name = " | ";
};

template <class E>
struct pretty_print_args_<E, true>
{
	static inline void print(std::ostream& os, E const& e, Symbols const* symbols)
	{
		auto it = e.args().cbegin();
		auto it_end = e.args().cend();
		if (it == it_end) {
			os << "(empty)";
			return;
		}
		it_end--;
		os << '(';
		for (; it != it_end; ++it) {
			pretty_print(os, *it, symbols);
			os << expr_math_name<E>::name;
		}
		pretty_print(os, *it, symbols);
		os << ')';
	}
};

template <>
struct pretty_print_args_<ExprImm, false>
{
	static inline void print(std::ostream& os, ExprImm const& e, Symbols const*)
	{
		os << e.value(); 
	}
};

template <>
struct pretty_print_args_<ExprSym, false>
{
	static inline void print(std::ostream& os, ExprSym const& e, Symbols const* symbols)
	{
		const char* name = nullptr;
		if (pp_print_sym) {
			os << "Sym(";
		}
		if (symbols != nullptr) {
			name = symbols->name(e);
		}

		if (name == nullptr) {
			os << e.idx();
		}
		else {
			os << symbols->name(e);
		}
		if (pp_print_sym) {
			os << ")";
		}
	}
};

template <>
struct pretty_print_args_<ExprESF, true>
{
	static inline void print(std::ostream& os, ExprESF const& e, Symbols const* symbols)
	{
		os << "ESF(" << (size_t) e.degree() << ", ";

		auto it = e.args().cbegin();
		auto it_end = e.args().cend();
		if (it == it_end) {
			os << "(empty))";
			return;
		}
		it_end--;
		for (; it != it_end; ++it) {
			pretty_print(os, *it, symbols);
			os << ", ";
		}
		pretty_print(os, *it, symbols);
		os << ')';
	}
};

template <class E>
struct pretty_print_args: public pretty_print_args_<E, expr_traits::has_args<E>::value>
{ };

template <class E>
inline void pretty_print(std::ostream& os, E const& e, Symbols const* symbols) 
{
   	__impl::pretty_print_args<E>::print(os, e, symbols);
}

template <class O>
struct stream_wrapper
{
	stream_wrapper(O const& o, Symbols const* symbols):
		_o(o),
		_symbols(symbols)
	{ }

	O const& obj() const { return _o; }
	Symbols const* symbols() const { return _symbols; }

private:
	O const& _o;
	Symbols const* _symbols;
};

} // __impl

PA_API inline void pretty_print(std::ostream& os, Expr const& e, Symbols const* symbols)
{
	e.call([&os, symbols] (auto const& e) { __impl::pretty_print(os, e, symbols); });
}

PA_API inline void pretty_print(std::ostream& os, Vector const& v, Symbols const* symbols)
{
	os << "Vec([" << std::endl;;
	Vector::const_iterator it = v.begin();
	Vector::const_iterator it_end = v.end();
	it_end--;
	for (; it != it_end; it++) {
		pa::pretty_print(os, *it, symbols);
		os << "," << std::endl;
	}
	pa::pretty_print(os, *it, symbols);
	os << std::endl << "])";
}

PA_API inline void pretty_print(std::ostream& os, Matrix const& m, Symbols const* symbols)
{
	if (m.empty()) {
		os << "Mat(empty)";
		return;
	}

	os << "Mat([" << std::endl;;
	size_t i,j;
	for (i = 0; i < m.nlines()-1; i++) {
		os << "[";
		for (j = 0; j < m.ncols()-1; j++) {
			pa::pretty_print(os, m.at(i,j), symbols);
			os << ", ";
		}
		pa::pretty_print(os, m.at(i, j), symbols);
		os << "]" << std::endl;
	}

	os << "[";
	for (j = 0; j < m.ncols()-1; j++) {
		pa::pretty_print(os, m.at(i,j), symbols);
		os << ", ";
	}
	pa::pretty_print(os, m.at(i, j), symbols);
	os << "]" << std::endl << "])";
}

PA_API inline void pretty_print(std::ostream& os, VectorApp const& a, Symbols const* symbols)
{
	pretty_print(os, a.vector(), symbols);
}

PA_API inline void pretty_print(std::ostream& os, AffApp const& a, Symbols const* symbols)
{
	os << "AffApp matrix = ";
	pretty_print(os, a.matrix(), symbols);
	os << "\n\nAffApp cst = ";
	pretty_print(os, a.cst(), symbols);
}

PA_API inline void pretty_print(std::ostream& os, App const& a, Symbols const* symbols)
{
	os << "App NL = ";
	pretty_print(os, a.nl(), symbols);
	os << "\n\n";
	pretty_print(os, a.affine(), symbols);
}

template <class O>
inline __impl::stream_wrapper<O> pretty_print(O const& o, Symbols const* symbols)
{
	return __impl::stream_wrapper<O>(o, symbols);
}

template <class O>
inline void pretty_print(std::ostream& os, O const& o)
{
	pretty_print(os, o, symbols());
}


template <class O>
inline __impl::stream_wrapper<O> pretty_print(O const& o)
{
	return pretty_print(o, symbols());
}

PA_API void pp_sym_display(bool show = true);


} // pa

template <class O>
std::ostream& operator<<(std::ostream& os, pa::__impl::stream_wrapper<O> const& sw)
{
	pretty_print(os, sw.obj(), sw.symbols());
	return os;
}

#ifndef NDEBUG
PA_API void print_expr(pa::Expr const& e);
#endif

#endif
