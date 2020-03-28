#include <pa/exprs.h>
#include <pa/matrix.h>
#include <pa/vector.h>
#include <pa/prettyprinter.h>
#include <pa/simps.h>
#include <pa/subs.h>
#include <pa/analyses.h>
#include <pa/syms_set.h>
#include <pa/syms_hist.h>
#include <pa/visitors.h>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>

#include <sstream>

namespace py = pybind11;

template <class T, class PyIt = pybind11::detail::list_iterator>
struct stl_input_iterator: public std::iterator<std::input_iterator_tag, T>
{
	typedef std::iterator<std::input_iterator_tag, T> iterator_base;
	using category = typename iterator_base::iterator_category;
	using value_type = typename iterator_base::value_type;
	static constexpr bool is_integer = std::is_integral<value_type>::value;
	using difference_type = typename iterator_base::difference_type;
	using pointer = typename iterator_base::pointer;
	using reference = typename iterator_base::reference;

	stl_input_iterator(PyIt&& it):
		_it(std::move(it))
	{ }

	bool operator!=(stl_input_iterator const& o) const { return _it != o._it; }
	bool operator==(stl_input_iterator const& o) const { return _it == o._it; }

	reference operator*() const
	{
		return *(*_it).template cast<typename std::add_pointer<typename std::remove_reference<reference>::type>::type>();
	}
	pointer operator->() const
	{ 
		return (*_it).template cast<pointer>();
	}

	stl_input_iterator& operator++() { ++_it; return *this; }
	stl_input_iterator& operator++(int) { return _it++; }

private:
	PyIt _it;
};

template <class T, class PyIt = pybind11::detail::list_iterator>
struct stl_value_input_iterator: public std::iterator<std::input_iterator_tag, T>
{
	typedef T value_type;

	stl_value_input_iterator() { }
	stl_value_input_iterator(PyIt&& it):
		_it(std::move(it))
	{ }

	bool operator!=(stl_value_input_iterator const& o) const { return _it != o._it; }
	bool operator==(stl_value_input_iterator const& o) const { return _it == o._it; }

	value_type operator*() const
	{
		return (*_it).template cast<value_type>();
	}

	stl_value_input_iterator& operator++() { ++_it; return *this; }
	stl_value_input_iterator& operator++(int) { return _it++; }

private:
	PyIt _it;
};

namespace pa {

template <>
auto array_size(py::list const& l)
{
	return py::len(l);
}

}

bool (pa::Expr::*expr_eq)(pa::Expr const&) const = &pa::Expr::operator==;
bool (pa::Expr::*expr_neq)(pa::Expr const&) const = &pa::Expr::operator!=;

pa::Expr (pa::Expr::*expr_add)(pa::Expr const&) const = &pa::Expr::operator+;
pa::Expr (pa::Expr::*expr_mul)(pa::Expr const&) const = &pa::Expr::operator*;
pa::Expr (pa::Expr::*expr_or)(pa::Expr const&)  const = &pa::Expr::operator|;
pa::Expr& (pa::Expr::*expr_self_add)(pa::Expr const&) = &pa::Expr::operator+=;
pa::Expr& (pa::Expr::*expr_self_mul)(pa::Expr const&) = &pa::Expr::operator*=;
pa::Expr& (pa::Expr::*expr_self_or)(pa::Expr const&)  = &pa::Expr::operator|=;

pa::Expr const& (pa::ExprArgs::*exprargs_at)(size_t) const = &pa::ExprArgs::operator[];

static std::string expr_str(pa::Expr const& e)
{
	std::stringstream ss;
	ss << pa::pretty_print(e);
	return ss.str();
}

static pa::Expr expr_imm(int const v)
{
	return pa::ExprImm((bool)v);
}


pa::Expr& (pa::Vector::*vector_at)(const size_t) = &pa::Vector::at;
pa::Vector (pa::Vector::*vector_lshift)(const size_t) const = &pa::Vector::operator<<;
pa::Vector (pa::Vector::*vector_rshift)(const size_t) const = &pa::Vector::operator>>;

pa::Vector (pa::Vector::*vector_mul_vector)(pa::Vector const&) const = &pa::Vector::operator*;
pa::Vector (pa::Vector::*vector_mul_expr)(pa::Expr const&) const = &pa::Vector::operator*;

pa::Vector& (pa::Vector::*vector_imul_vector)(pa::Vector const&) = &pa::Vector::operator*=;
pa::Vector& (pa::Vector::*vector_imul_expr)(pa::Expr const&) = &pa::Vector::operator*=;

static std::string vector_str(pa::Vector const& v)
{
	std::stringstream ss;
	ss << pa::pretty_print(v);
	return ss.str();
}

static void vector_set(pa::Vector& v, const size_t i, pa::Expr const& e)
{
	v[i] = e;
}

static pa::Vector vector_slice(pa::Vector const& v, py::slice const& s)
{
	size_t start, stop, step;
	size_t slicelength;
	if (!s.compute(v.size(), &start, &stop, &step, &slicelength))
		throw py::error_already_set();

	pa::Vector ret;
	auto& ret_storage = ret.args();
	ret_storage.reserve(slicelength);
	auto it = v.begin();
	auto end = v.begin();
	std::advance(it, start);
	std::advance(end, stop);
	while (it != end) {
		ret_storage.emplace_back(*it);
		std::advance(it, step);
	}
	return ret;
}

struct VectorNotImmediate: public std::exception
{
	const char* what() const noexcept override { return "Expected a vector with only immediates"; }
};

static size_t vector_get_int_le(pa::Vector const& v)
{
	bool ok = true;
	size_t ret = v.get_int_le(&ok);
	if (!ok) {
		throw VectorNotImmediate();
	}
	return ret;
}

static size_t vector_get_int_be(pa::Vector const& v)
{
	bool ok = true;
	size_t ret = v.get_int_be(&ok);
	if (!ok) {
		throw VectorNotImmediate();
	}
	return ret;
}

pa::Expr& (pa::Matrix::*matrix_at)(const size_t, const size_t) = &pa::Matrix::at;
pa::Matrix (pa::Matrix::*matrix_mul_matrix)(pa::Matrix const&) const = &pa::Matrix::operator*;
pa::Vector (pa::Matrix::*matrix_mul_vector)(pa::Vector const&) const = &pa::Matrix::operator*;

static std::shared_ptr<pa::Matrix> matrix_identity(const size_t n)
{
	return std::make_shared<pa::Matrix>(pa::Matrix::identity(n));
}

void matrix_construct(pa::Matrix* self, const size_t nlines, const size_t ncols, py::object& f)
{
	new (self) pa::Matrix{pa::Matrix::construct(nlines, ncols,
		[&f] (const size_t i, const size_t j) -> pa::Expr
		{
			return *f(i, j).cast<pa::Expr const*>();
		})};
}

static std::string matrix_str(pa::Matrix const& m)
{
	std::stringstream ss;
	ss << pa::pretty_print(m);
	return ss.str();
}

pa::Expr& (*simp_exp)(pa::Expr&) =   &pa::simps::simplify;
pa::Vector& (*simp_vec)(pa::Vector&) = &pa::simps::simplify;
pa::Matrix& (*simp_mat)(pa::Matrix&) = &pa::simps::simplify;

static pa::Expr simp_exp_copy(pa::Expr const& e)
{
	pa::Expr ret = e;
	pa::simps::simplify(ret);
	return ret;
}

static pa::Vector simp_vec_copy(pa::Vector const& v)
{
	pa::Vector ret = v;
	pa::simps::simplify(ret);
	return ret;
}

static pa::Matrix simp_mat_copy(pa::Matrix const& m)
{
	pa::Matrix ret = m;
	pa::simps::simplify(ret);
	return ret;
}

namespace __impl {
template <class Iterator>
class python_list_ro_wrapper
{
public:
	typedef Iterator iterator;

public:
	python_list_ro_wrapper(py::list const& l):
		_l(l)
	{ }

public:
	inline iterator begin() const { return iterator{_l.begin()}; }
	inline iterator end() const { return iterator{_l.end()}; }

private:
	py::list const& _l;
};
} // __impl

template <class T>
using python_list_ro_wrapper = __impl::python_list_ro_wrapper<stl_input_iterator<const T>>;
template <class T>
using python_list_value_wrapper = __impl::python_list_ro_wrapper<stl_value_input_iterator<T>>;

static pa::Expr subs_vectors_exp(pa::Expr const& e, py::list const& vecs, py::list const& values)
{
	pa::Expr ret = e;
	pa::subs_vectors(ret, python_list_ro_wrapper<pa::Vector>(vecs), python_list_value_wrapper<uint64_t>(values));
	return ret;
}

static pa::Vector subs_vectors_vec(pa::Vector const& v, py::list const& vecs, py::list const& values)
{
	pa::Vector ret = v;
	pa::subs_vectors(ret, python_list_ro_wrapper<pa::Vector>{vecs}, python_list_value_wrapper<uint64_t>{values});
	return ret;
}

static pa::Matrix subs_vectors_mat(pa::Matrix const& m, py::list const& vecs, py::list const& values)
{
	pa::Matrix ret = m;
	pa::subs_vectors(ret, python_list_ro_wrapper<pa::Vector>(vecs), python_list_value_wrapper<uint64_t>(values));
	return ret;
}

static std::map<pa::Expr, pa::Expr> lists_to_map(py::list const& keys, py::list const& values)
{
	using map_type = std::map<pa::Expr, pa::Expr>;
	const auto n = py::len(keys);
	if (n != py::len(values)) {
		return map_type{};
	}
	map_type ret;
	stl_input_iterator<pa::Expr> itk(keys.begin());
	stl_input_iterator<pa::Expr> end(keys.end());
	stl_input_iterator<pa::Expr> itv(values.begin());
	for (; itk != end; ++itk) {
		ret[*itk] = *itv;
		++itv;
	}
	return ret;
}

static void subs_exprs_exp_inplace(pa::Expr& e, py::list const& src, py::list const& to)
{
	pa::subs_exprs(e, lists_to_map(src, to));
}

static void subs_exprs_vec_inplace(pa::Vector& v, py::list const& src, py::list const& to)
{
	pa::subs_exprs(v, lists_to_map(src, to));
}

static void subs_exprs_mat_inplace(pa::Matrix& m, py::list const& src, py::list const& to)
{
	pa::subs_exprs(m, lists_to_map(src, to));
}

static pa::Expr subs_exprs_exp(pa::Expr& e, py::list const& src, py::list const& to)
{
	pa::Expr ret = e;
	subs_exprs_exp_inplace(ret, src, to);
	return ret;
}

static pa::Vector subs_exprs_vec(pa::Vector& v, py::list const& src, py::list const& to)
{
	pa::Vector ret = v;
	subs_exprs_vec_inplace(ret, src, to);
	return ret;
}

static pa::Matrix subs_exprs_mat(pa::Matrix& m, py::list const& src, py::list const& to)
{
	pa::Matrix ret = m;
	subs_exprs_mat_inplace(ret, src, to);
	return ret;
}

struct PythonAnalyses
{ };

static std::string affapp_str(pa::AffApp const& a)
{
	std::stringstream ss;
	ss << pa::pretty_print(a);
	return ss.str();
}

static std::string vectorapp_str(pa::VectorApp const& a)
{
	std::stringstream ss;
	ss << pa::pretty_print(a);
	return ss.str();
}

static std::string app_str(pa::App const& a)
{
	std::stringstream ss;
	ss << pa::pretty_print(a);
	return ss.str();
}

struct BadType: public std::exception
{
private:
    struct tag_no_args { };

private:
    BadType(const char* expected, const char* got)
    {
        std::stringstream ss;
        ss << "Bad expression type: expected '" << expected << ", got '" << got << "'";
        _err = ss.str();
    }

    BadType(tag_no_args, const char* got)
    {
        std::stringstream ss;
        ss << "Bad expression type: expected an argument (ExprAdd, ExprMul, ExprOr or ExprESF), got '" << got << "'";
        _err = ss.str();
    }

public:
    template <class E>
    static void throw_error(pa::Expr const& e)
    {
        throw BadType(E::name(), e.name());
    }

    static void throw_error_no_arg(pa::Expr const& e)
    {
        throw BadType(tag_no_args(), e.name());
    }

    static void throw_error_no_arg(const char* name)
    {
        throw BadType(tag_no_args(), name);
    }

public:
    const char* what() const throw() override { return _err.c_str(); }

private:
    std::string _err;
};

template <class E>
static void check_type_throw(pa::Expr const& e)
{
    if (e.type() != E::type_id) {
        BadType::throw_error<E>(e);
    }
}

static void check_has_args(pa::Expr const& e)
{
    if (!e.has_args()) {
        BadType::throw_error_no_arg(e);
    }
}

static pa::Symbols::idx_type expr_sym_idx(pa::Expr const& e)
{
    check_type_throw<pa::ExprSym>(e);
    return e.call_assert([] (pa::ExprSym const& e_) { return e_.idx(); });
}

static pa::ExprArgs const& expr_args(pa::Expr const& e)
{
    check_has_args(e);
    return e.args();
}

static bool expr_imm_value(pa::Expr const& e)
{
    check_type_throw<pa::ExprImm>(e);
    return e.call_assert([] (pa::ExprImm const& e_) { return e_.value(); });
}

static pa::Expr expr_with_args_list(pa::expr_type_id type, py::list const& l)
{
	if (type >= pa::expr_type_id::symbol_type || type == pa::expr_type_id::esf_type) {
		// TODO: put real name
		BadType::throw_error_no_arg("invalid");
		return pa::Expr{};
	}
	pa::Expr::ExprArgsStorage args(stl_input_iterator<pa::Expr>{l.begin()}, stl_input_iterator<pa::Expr>{l.end()});
	return pa::Expr{type, std::move(args)};
}

static pa::Expr expr_with_args(pa::expr_type_id type, pa::ExprArgs const& args)
{
	if (type >= pa::expr_type_id::symbol_type || type == pa::expr_type_id::esf_type) {
		// TODO: put real name
		BadType::throw_error_no_arg("invalid");
		return pa::Expr{};
	}
	return pa::Expr{type, args};
}

static pa::ExprESF::degree_type expr_esf_degree(pa::Expr const& e)
{
	check_type_throw<pa::ExprESF>(e);
    return e.call_assert([] (pa::ExprESF const& e_) { return e_.degree(); });
}

static pa::Expr expr_copy(pa::Expr const& e)
{
	return e;
}

static pa::Expr expr_eval(pa::Expr const& e, std::map<pa::Expr, pa::Expr> const& map)
{
	pa::Expr ret = e;
	subs_exprs(ret, map);
  pa::simps::simplify(ret);
	return ret;
}

static pa::Expr esf(pa::ExprESF::degree_type const degree, py::list const& l)
{
	return pa::ExprESF(degree, stl_input_iterator<pa::Expr>{l.begin()}, stl_input_iterator<pa::Expr>{l.end()});
}

static pa::Vector esf_vector(pa::ExprESF::degree_type const degree, py::list const& l)
{
	const size_t nargs = py::len(l);
	if (nargs == 0) {
		return pa::Vector{};
	}
	stl_input_iterator<pa::Vector> const begin(l.begin());
	stl_input_iterator<pa::Vector> end(l.end());
	stl_input_iterator<pa::Vector> it(begin); ++it;
	const size_t n = begin->size();
	for (; it != end; ++it) {
		if (n != it->size()) {
			return pa::Vector{};
		}
	}
	pa::Vector ret{n};
	for (size_t i = 0; i < n; i++) {
		pa::ExprArgs tmp;
		tmp.resize(nargs, pa::ExprImm(true));
		size_t j = 0;
		stl_input_iterator<pa::Vector> const begin(l.begin());
		for (it = begin; it != end; ++it) {
			tmp[j] = (*it)[i];
			j++;
		}
		ret[i] = pa::ExprESF(degree, std::move(tmp));
	}
	return ret;
}

static void expand_esf_inplace(pa::Expr& e)
{
	pa::simps::expand_esf(e);
}

static void expand_esf_inplace_vec(pa::Vector& v)
{
	pa::simps::expand_esf(v);
}

static pa::Expr expand_esf(pa::Expr& e)
{
	pa::Expr ret = e;
	pa::simps::expand_esf(ret);
	return ret;
}

static pa::Vector expand_esf_vec(pa::Vector& v)
{
	pa::Vector ret{v};
	pa::simps::expand_esf(ret);
	return ret;
}

static void or_to_esf_inplace(pa::Expr& e)
{
	pa::simps::or_to_esf(e);
}

static pa::Expr or_to_esf(pa::Expr& e)
{
	pa::Expr ret = e;
	pa::simps::or_to_esf(ret);
	return ret;
}

#if 0
struct ExprSymToExpr
{
	static PyObject* convert(pa::ExprSym const& s)
	{
		return boost::python::incref(boost::python::object(static_cast<pa::Expr const&>(s)).ptr());
	}
};
#endif

bool (pa::SymbolsSet::*syms_set_insert)(pa::Expr const&) = &pa::SymbolsSet::insert;
bool (pa::SymbolsSet::*syms_set_has)(pa::Expr const&) const = &pa::SymbolsSet::has;

struct PythonAlgos
{ };

template <class Integer>
class SetReadOnly 
{
public:
	typedef Integer const* iterator;

public:
	SetReadOnly():
		_buf(nullptr),
		_size(0)
	{ }

	SetReadOnly(Integer const* buf, size_t const size):
		_buf(buf),
		_size(size)
	{ }

public:
	inline Integer at(size_t idx) const
	{
		assert(idx < _size);
		return _buf[idx];
	}

	inline size_t size() const { return _size; }

	Integer const* begin() const { return _buf; }
	Integer const* end() const   { return _buf+_size; }

private:
	Integer const* _buf;
	size_t _size;
};
typedef SetReadOnly<size_t> SetReadOnlySizeT;

static void draw_without_replacement_python(const size_t n, const size_t end, py::object& f)
{
	pa::draw_without_replacement(n, end, 
		[&f](size_t const* idxes, const size_t n) -> bool
		{
			return f(SetReadOnlySizeT{idxes,n}).cast<bool>();
		});
}

static std::string syms_hist_value_repr(pa::SymbolsHist::const_iterator::value_type const& V)
{
	std::stringstream ss;
	ss << "<" << pa::pretty_print(V.first) << ": " << V.second << ">";
	return ss.str();
}

static pa::ExprSym syms_hist_value_sym(pa::SymbolsHist::const_iterator::value_type const& V)
{
	return V.first;
}

static pa::SymbolsHist::count_type syms_hist_value_count(pa::SymbolsHist::const_iterator::value_type const& V)
{
	return V.second;
}


bool (pa::SymbolsHist::*syms_hist_compute)(pa::Expr const&) = &pa::SymbolsHist::compute;
bool (pa::SymbolsHist::*syms_hist_compute_args_mul)(pa::Expr const&, unsigned) = &pa::SymbolsHist::compute;

template <class T>
auto py_iterator()
{
	return [](T const& o) { return py::make_iterator(o); };
}

PYBIND11_MAKE_OPAQUE(pa::SymbolsHist::const_iterator::value_type);

PYBIND11_MODULE(pytanque, m)
{
  m.doc() = "petanque python bindings";

	py::enum_<pa::expr_type_id>(m, "ExprType", "Enum of the various expression types")
		.value("esf", pa::expr_type_id::esf_type)
		.value("mul", pa::expr_type_id::mul_type)
		.value("add", pa::expr_type_id::add_type)
		.value("or_", pa::expr_type_id::or_type)
		.value("sym", pa::expr_type_id::symbol_type)
		.value("imm", pa::expr_type_id::imm_type)
		;

	py::class_<pa::ExprArgs>(m, "ExprArgs", "Represents the argument of an expression")
		.def("__iter__", py_iterator<pa::ExprArgs>(), py::keep_alive<0,1>())
		.def("size", &pa::ExprArgs::size)
		.def("len", &pa::ExprArgs::size)
		.def("__len__", &pa::ExprArgs::size)
		.def("__getitem__", exprargs_at, py::return_value_policy::reference_internal)
        ;

	py::class_<pa::Expr>(m, "Expr", "Represents a symbolic boolean expression")
		.def("has_args", &pa::Expr::has_args)
		.def(py::self + py::self)
		.def(py::self += py::self)
		.def(py::self * py::self)
		.def(py::self *= py::self)
		.def(py::self | py::self)
		.def(py::self |= py::self)
		.def("__repr__", expr_str)
		.def("__eq__", expr_eq)
		.def("__ne__", expr_neq)
		.def("__lt__", &pa::Expr::operator<)
		.def("args", expr_args, py::return_value_policy::reference_internal,
				"Returns a reference to an ExprArgs object if this expression\
				contains arguments. Throws a BadType exception otherwise")
		.def("type", &pa::Expr::type)
		.def("sym_idx", expr_sym_idx,
				"Returns the index associated to a symbol is this expression is\
				a symbol. Throws a BadType exception otherwise")
		.def("imm_value", expr_imm_value)
		.def("is_imm", &pa::Expr::is_imm)
		.def("is_sym", &pa::Expr::is_sym)
		.def("is_add", &pa::Expr::is_add)
		.def("is_mul", &pa::Expr::is_mul)
		.def("is_esf", &pa::Expr::is_esf)
		.def("name", &pa::Expr::name)
		.def("contains", &pa::Expr::contains,
				"Returns true iif arg0 is in self. This does not try to search for\
				it recursively")
		.def("anf_esf_max_degree", &pa::Expr::anf_esf_max_degree,
				"Returns the maximum possible degree that a potential ESF could\
				have in the expression. This is used by find_esfs")
		.def("esf_degree", expr_esf_degree)
		.def("copy", expr_copy, "Create a deep copy of the expression")
		.def("__hash__", &pa::Expr::hash)
		.def("eval", &expr_eval)
		;

	py::class_<pa::ExprSym, pa::Expr>(m, "ExprSym")
		;

	m.def("ExprWithArgs", expr_with_args_list);
	m.def("ExprWithArgs", expr_with_args);

	//boost::python::to_python_converter<pa::ExprSym, ExprSymToExpr>();

	py::class_<pa::Vector>(m, "Vector", "Represents a vector of Expr objects")
		.def(py::init<const size_t>())
		.def(py::init<const size_t, pa::Expr const&>())
		.def(py::init<pa::Vector const&>())
		.def("at", vector_at, py::return_value_policy::reference_internal)
		.def("size", &pa::Vector::size)
		.def("set_null", &pa::Vector::set_null)
		.def("set_int_be", &pa::Vector::set_int_be)
		.def("set_int_le", &pa::Vector::set_int_le)
		.def("get_int_be", vector_get_int_be)
		.def("get_int_le", vector_get_int_le)
		.def("__iter__", py_iterator<pa::Vector>(), py::keep_alive<0,1>())
		.def(py::self + py::self)
		.def(py::self += py::self)
		.def(py::self * py::self)
		.def(py::self *= py::self)
		.def(py::self * pa::Expr{})
		.def(py::self *= pa::Expr{})
		.def(py::self | py::self)
		.def(py::self |= py::self)
		.def("__getitem__", vector_at, py::return_value_policy::reference_internal)
		.def("__getitem__", vector_slice)
		.def("__setitem__", vector_set)
		.def("__len__", &pa::Vector::size)
		.def("__repr__", vector_str)
		.def("__lshift__", vector_lshift)
		.def("__rshift__", vector_rshift)
		.def("__eq__", &pa::Vector::operator==)
		.def("__ne__", &pa::Vector::operator!=)
		;

	py::class_<pa::Matrix>(m, "Matrix", "Represents a matrix of Expr objects")
		.def(py::init<const size_t, const size_t>())
		.def(py::init<const size_t, const size_t, pa::Expr const&>())
		.def("__init__", matrix_construct)
		.def("nlines", &pa::Matrix::nlines)
		.def("ncols", &pa::Matrix::ncols)
		.def("same_size", &pa::Matrix::same_size)
		.def("inverse", &pa::Matrix::inverse)
		.def("at", matrix_at, py::return_value_policy::reference_internal)
		.def(py::self + py::self)
		.def(py::self += py::self)
		.def(py::self * py::self)
		.def(py::self * pa::Vector{})
		.def("identity", matrix_identity)
		.def("__iter__", py_iterator<pa::Matrix>(), py::keep_alive<0,1>())
		.def("__repr__", matrix_str)
		.def("__eq__", &pa::Matrix::operator==)
		.def("__ne__", &pa::Matrix::operator!=)
		;

	py::class_<pa::AffApp>(m, "AffApp", "Represents an affine application")
		.def(py::init<pa::Matrix const&, pa::Vector const&>())
		.def("__call__", &pa::AffApp::operator())
		.def("matrix", &pa::AffApp::matrix, py::return_value_policy::reference_internal)
		.def("cst", &pa::AffApp::cst, py::return_value_policy::reference_internal)
		.def("__repr__", affapp_str)
		;

	py::class_<pa::VectorApp>(m, "VectorApp", "Represents a generic application by a Vector object")
		.def(py::init<pa::Vector const&, pa::Vector const&>())
		.def("__call__", &pa::VectorApp::operator())
		.def("vector", &pa::VectorApp::vector, py::return_value_policy::reference_internal)
		.def("__repr__", vectorapp_str)
		;

	py::class_<pa::App>(m, "App",
		"Represents an application by separating the\
		non-linear and affine parts")
		.def(py::init<pa::VectorApp const&, pa::AffApp const&>())
		.def("__call__", &pa::App::operator())
		.def("matrix", &pa::App::matrix, py::return_value_policy::reference_internal)
		.def("cst", &pa::App::cst, py::return_value_policy::reference_internal)
		.def("nl", &pa::App::nl, py::return_value_policy::reference_internal)
		.def("affine", &pa::App::affine, py::return_value_policy::reference_internal)
		.def("__repr__", app_str)
		;

	//py::class_<std::map<pa::Expr, pa::Expr>>(m, "map_exprs")
	//	.def(py::map_indexing_suite<std::map<pa::Expr, pa::Expr>>())
	//	;

	py::class_<pa::SymbolsSet>(m, "SymbolsSet")
		.def(py::init<>())
		.def("insert", syms_set_insert)
		.def("has", syms_set_has)
		.def("size", &pa::SymbolsSet::size)
		.def("empty", &pa::SymbolsSet::empty)
		.def("__contains__", syms_set_has)
		.def("__iter__", py_iterator<const pa::SymbolsSet>(), py::keep_alive<0,1>())
		.def("__len__", &pa::SymbolsSet::size)
		;

	py::class_<pa::SymbolsHist>(m, "SymbolsHist")
		.def(py::init<>())
		.def("compute", syms_hist_compute)
		.def("compute", syms_hist_compute_args_mul)
		.def("size", &pa::SymbolsHist::size)
		.def("empty", &pa::SymbolsHist::empty)
		.def("__iter__", py_iterator<const pa::SymbolsHist>(), py::keep_alive<0,1>())
		.def("__len__", &pa::SymbolsHist::size)
		;

	py::class_<pa::SymbolsHist::const_iterator::value_type>(m, "SymbolsHistValue")
		.def("sym", syms_hist_value_sym)
		.def("count", syms_hist_value_count)
		.def("__repr__", syms_hist_value_repr)
		;

	m.def("imm", expr_imm);
	m.def("symbol", pa::symbol);
	m.def("arg_symbol", pa::arg_symbol);
	m.def("esf", esf);
	m.def("esf_vector", esf_vector);

	m.def("simplify_inplace", simp_exp, py::return_value_policy::reference_internal);
	m.def("simplify_inplace", simp_vec, py::return_value_policy::reference_internal);
	m.def("simplify_inplace", simp_mat, py::return_value_policy::reference_internal);

	m.def("simplify", simp_exp_copy);
	m.def("simplify", simp_vec_copy);
	m.def("simplify", simp_mat_copy);

	m.def("subs_vectors", subs_vectors_exp);
	m.def("subs_vectors", subs_vectors_vec);
	m.def("subs_vectors", subs_vectors_mat);

	m.def("subs_exprs_inplace", subs_exprs_exp_inplace);
	m.def("subs_exprs_inplace", subs_exprs_vec_inplace);
	m.def("subs_exprs_inplace", subs_exprs_mat_inplace);

	m.def("subs_exprs", subs_exprs_exp);
	m.def("subs_exprs", subs_exprs_vec);
	m.def("subs_exprs", subs_exprs_mat);

	m.def("expand_esf_inplace", expand_esf_inplace, py::return_value_policy::reference_internal);
	m.def("expand_esf_inplace", expand_esf_inplace_vec, py::return_value_policy::reference_internal);

	m.def("expand_esf", expand_esf);
	m.def("expand_esf", expand_esf_vec);

	m.def("or_to_esf_inplace", or_to_esf_inplace, py::return_value_policy::reference_internal);
	m.def("or_to_esf", or_to_esf);

	m.def("identify_ors_inplace", pa::simps::identify_ors, py::return_value_policy::reference_internal);

	py::class_<VectorNotImmediate>(m, "VectorNotImmediate");

	py::class_<SetReadOnlySizeT>(m, "SetReadOnly")
		.def("at", &SetReadOnlySizeT::at)
		.def("size", &SetReadOnlySizeT::size)
		.def("__iter__", py_iterator<SetReadOnlySizeT>(), py::keep_alive<0,1>())
		;

	{
		py::module analyses = m.def_submodule("analyses");
		analyses.def("vectorial_decomp", pa::analyses::vectorial_decomp);
	}

	{
		py::module algos = m.def_submodule("algos");
		algos.def("draw_without_replacement", draw_without_replacement_python);
	}
}
