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

#ifndef PETANQUE_EXPRS_H
#define PETANQUE_EXPRS_H

#include <vector>
#include <cstdint>
#include <cassert>

#include <pa/exports.h>
#include <pa/traits.h>
//#include <pector/pector.h>
//#include <pector/malloc_allocator.h>
#include <pa/sorted_vector.h>

namespace pa {
class Expr;
}

namespace std {

template <>
struct is_pod<pa::Expr>: public std::false_type
{ };

}

namespace pa {

enum class expr_type_id: unsigned char {
	or_type,
	esf_type, // elementary symmetric function
	mul_type,
	add_type,
	symbol_type,
	imm_type
};

#pragma pack(push)
#pragma pack(1)
class PA_API Expr
{
public:
	//typedef pt::pector<Expr, std::allocator<Expr>, uint32_t, pt::default_recommended_size, false> list;
	//typedef pt::pector<Expr, pt::malloc_allocator<Expr, true, true>, uint32_t, pt::default_recommended_size, false> list;
	//typedef std::vector<Expr> list;
	typedef pa::SortedVector<std::vector<Expr>, 3> list;

	// TODO: make this protected with a proxy for Expr* functions below
public:
	class PA_API ExprImmStorage
	{
	public:
		ExprImmStorage() = default;
		ExprImmStorage(const bool v):
			_v(v)
		{ }

	public:
		inline bool value() const { return _v; }
		inline bool& value() { return _v; }

	private:
		bool _v;
	};

	class ExprSymStorage
	{
	public:
		typedef uint32_t idx_type;

	public:
		ExprSymStorage() = default;
		ExprSymStorage(idx_type const idx):
			_symb_idx(idx)
		{ }

	public:
		inline idx_type idx() const { return _symb_idx; }
		inline idx_type& idx() { return _symb_idx; }

	private:
		idx_type _symb_idx;
	};

	struct PA_API ExprArgsStorage
	{
		ExprArgsStorage() = default;

		ExprArgsStorage(list const& l):
			_args(l)
		{ }

		ExprArgsStorage(list&& l):
			_args(std::move(l))
		{ }

		ExprArgsStorage(std::initializer_list<Expr> const& l):
			_args(false, l)
		{ }

		template <class Iterator>
		explicit ExprArgsStorage(Iterator begin, Iterator end):
			_args(false, begin, end)
		{ }

	public:
		inline list const& args() const { return _args; }
		inline list& args() { return _args; }

	public:
		inline void reserve(const size_t n) { args().reserve(n); }

		void extend(list const& c)
		{
			extend(std::begin(c), std::end(c));
		}

		template <class Iterator>
		void extend(Iterator begin, Iterator end)
		{
			const size_t n = std::distance(begin, end);
			args().reserve(args().size() + n);
			args().insert_dup(begin, end);
		}

		void extend_no_dup(list const& c)
		{
			extend_no_dup(std::begin(c), std::end(c));
		}

		template <class Iterator>
		void extend_no_dup(Iterator begin, Iterator end)
		{
			const size_t n = std::distance(begin, end);
			args().reserve(args().size() + n);
			args().insert(begin, end);
		}

		void extend_move(list&& c)
		{
			extend_move(std::begin(c), std::end(c));
		}

		template <class Iterator>
		void extend_move(Iterator begin, Iterator end)
		{
			const size_t n = std::distance(begin, end);
			args().reserve(args().size() + n);
			args().insert_dup_move(begin, end);
		}

		void extend_move_no_dup(list&& c)
		{
			extend_move_no_dup(std::begin(c), std::end(c));
		}

		template <class Iterator>
		void extend_move_no_dup(Iterator begin, Iterator end)
		{
			const size_t n = std::distance(begin, end);
			args().reserve(args().size() + n);
			args().insert_move(begin, end);
		}

		template <class E>
		void insert(E&& e)
		{
			args().insert_dup(std::forward<E>(e));
		}

	public:
		static bool args_equal_with(list const& a, list const& b);
		static bool args_less_than(list const& a, list const& b);

		inline bool operator==(ExprArgsStorage const& o) const
		{
			return args_equal_with(args(), o.args());
		}

		inline bool operator<(ExprArgsStorage const& o) const
		{
			return args_less_than(args(), o.args());
		}

	private:
		list _args;
	};

	class PA_API ExprESFStorage: public ExprArgsStorage
	{
	public:
		typedef uint8_t degree_type;

	public:
		ExprESFStorage():
			ExprArgsStorage(),
			_degree(0)
		{ }

		ExprESFStorage(std::initializer_list<Expr> const& l):
			ExprArgsStorage(l),
			_degree(0)
		{ }

		ExprESFStorage(list&& l):
			ExprArgsStorage(std::move(l)),
			_degree(0)
		{ }

		template <class Iterator>
		explicit ExprESFStorage(Iterator begin, Iterator end):
			ExprArgsStorage(begin, end),
			_degree(0)
		{ }

		ExprESFStorage(ExprESFStorage const& o):
			ExprArgsStorage(o),
			_degree(o._degree)
		{ }

		ExprESFStorage(ExprESFStorage&& o):
			ExprArgsStorage(std::move(o)),
			_degree(o._degree)
		{ }

	public:
		degree_type  degree() const { return _degree; }
		degree_type& degree()       { return _degree; }

	public:
		inline bool operator==(ExprESFStorage const& o) const
		{
			if (_degree != o._degree) {
				return false;
			}
			return static_cast<ExprArgsStorage const&>(*this) == static_cast<ExprArgsStorage const&>(o);
		}

		inline bool operator<(ExprESFStorage const& o) const
		{
			if (_degree == o._degree) {
				return static_cast<ExprArgsStorage const&>(*this) < static_cast<ExprArgsStorage const&>(o);
			}
			return _degree < o._degree;
		}

	private:
		degree_type _degree;
	};

	union PA_API ExprStorage
	{
		void destruct_args()
		{
			args.~ExprArgsStorage();
		}

		void destruct_args_sf()
		{
			sf.~ExprESFStorage();
		}

		ExprStorage() { }

		explicit ExprStorage(ExprImmStorage const& imm_):
			imm(imm_)
		{ }

		explicit ExprStorage(ExprSymStorage const& sym_):
			sym(sym_)
		{ }

		explicit ExprStorage(ExprArgsStorage const& args_):
			args(args_)
		{ }

		explicit ExprStorage(ExprArgsStorage&& args_):
			args(std::move(args_))
		{ }

		explicit ExprStorage(ExprESFStorage const& sf_):
			sf(sf_)
		{ }

		explicit ExprStorage(ExprESFStorage&& sf_):
			sf(std::move(sf_))
		{ }

		~ExprStorage() { }
		
		inline void copy_args(ExprStorage const& o)
		{
			args = o.args;
		}

		inline void extend_move(ExprStorage&& o)
		{
			args = std::move(o.args);
		}

		inline void copy_immediates(ExprStorage const& o)
		{
			// "imm" is included in "sym"
			sym = o.sym;
		}

		inline void move_immediates(ExprStorage&& o)
		{
			copy_immediates(o);
		}

		ExprImmStorage imm;
		ExprSymStorage sym;
		ExprArgsStorage args;
		ExprESFStorage sf;
	};

public:
	Expr():
		_type(expr_type_id::imm_type)
	{ }

	Expr(Expr const& o):
		_type(o.type())
	{
		if (is_esf()) {
			new (&storage()) ExprStorage(o.storage().sf);
		}
		else
		if (has_args()) {
			new (&storage()) ExprStorage(o.storage().args);
		}
		else {
			storage().copy_immediates(o.storage());
		}
	}

	Expr(Expr&& o):
		_type(o.type())
	{
		if (is_esf()) {
			new (&storage()) ExprStorage(std::move(o.storage().sf));
		}
		else
		if (has_args()) {
			new (&storage()) ExprStorage(std::move(o.storage().args));
		}
		else {
			storage().copy_immediates(o.storage());
		}
	}

	template <class Storage>
	Expr(expr_type_id type, Storage&& storage):
		_type(type),
		_storage(std::forward<Storage>(storage))
	{ }

	~Expr()
	{
		destruct_args();
	}

public:
	Expr& operator=(Expr const& o)
	{
		if (&o != this) {
			// TODO: optimize this to prevent destruction/construction of the vector class!
			Expr o_ = o;
			destruct_args();
			if (o_.is_esf()) {
				new (&storage()) ExprStorage(o_.storage().sf);
			}
			else
			if (o_.has_args()) {
				new (&storage()) ExprStorage(o_.storage().args);
			}
			else {
				storage().copy_immediates(o_.storage());
			}
			_type = o_.type();
		}
		return *this;
	}

	Expr& operator=(Expr&& o)
	{
		if (&o != this) {
			// TODO: optimize this to prevent destruction/construction of the vector class!
			Expr o_cp(std::move(o));
			destruct_args();
			if (o_cp.is_esf()) {
				new (&storage()) ExprStorage(std::move(o_cp.storage().sf));
			}
			else
			if (o_cp.has_args()) {
				new (&storage()) ExprStorage(std::move(o_cp.storage().args));
			}
			else {
				storage().move_immediates(std::move(o_cp.storage()));
			}
			_type = o_cp.type();
		}
		return *this;
	}

public:
	bool operator==(Expr const& o) const;
	bool operator!=(Expr const& o) const
	{
		return !(*this == o);
	}

public:
	template <class F>
	auto call(F const& f);

	template <class F>
	auto call(F const& f) const;

	template <class F>
	void call_no_return(F const& f);

	template <class F>
	void call_no_return(F const& f) const;

	template <class F>
	void call_if(F const& f)
	{
		typedef typename function_arg_expr<F>::type E;
		if (type() == E::type_id) { 
			f(static_cast<E const&>(*this));
		}
	}

	template <class F>
	void call_if(F const& f) const
	{
		typedef typename function_arg_expr<F>::type E;
		if (type() == E::type_id) { 
			f(static_cast<E const&>(*this));
		}
	}

	template <class F>
	auto call_assert(F const& f)
	{
		typedef typename function_arg_expr<F>::type E;
		assert(E::type_id == type());
		return f(static_cast<E&>(*this));
	}

	template <class F>
	auto call_assert(F const& f) const
	{
		typedef typename function_arg_expr<F>::type E;
		assert(E::type_id == type());
		return f(static_cast<E const&>(*this));
	}

	// Helpers
public:
	inline bool is_sym() const
	{
		return type() == expr_type_id::symbol_type;
	}

	inline bool is_imm() const
	{
		return type() == expr_type_id::imm_type;
	}

	inline bool is_add() const
	{
		return type() == expr_type_id::add_type;
	}

	inline bool is_mul() const
	{
		return type() == expr_type_id::mul_type;
	}

	inline bool is_esf() const
	{
		return type() == expr_type_id::esf_type;
	}

	inline bool is_or() const
	{
		return type() == expr_type_id::or_type;
	}

	inline bool is_or_esf() const
	{
		return type() <= expr_type_id::esf_type;
	}

	inline bool is_zero() const
	{
		return is_imm() && (storage().imm.value() == 0);
	}

public:
	inline expr_type_id type() const { return _type; }

	inline bool has_args() const
	{
		return type() < expr_type_id::symbol_type;
	}

	inline list& args()
	{
		assert(has_args());
		return storage().args.args();
	}

	inline list const& args() const
	{
		assert(has_args());
		return storage().args.args();
	}

	inline size_t nargs() const
	{
		return args().size();
	}

	inline const char* name() const
	{
		switch (type()) {
			case expr_type_id::or_type:
				return "Or";
			case expr_type_id::esf_type:
				return "SEF";
			case expr_type_id::mul_type:
				return "Mul";
			case expr_type_id::add_type:
				return "Add";
			case expr_type_id::symbol_type:
				return "Sym";
			case expr_type_id::imm_type:
				return "Imm";
		};
		assert(false);
		return "";
	}

	template <class E, class... Args>
	void set(Args&& ... args)
	{
		*this = E(std::forward<Args>(args)...);
	}

	template <class E>
	E& as() { return static_cast<E&>(*this); }

	template <class E>
	E const& as() const { return static_cast<E const&>(*this); }

	bool is_anf() const;
	unsigned anf_esf_max_degree() const;

	bool contains(Expr const&) const;

public:
	Expr operator|(Expr const& o) const;
	Expr operator+(Expr const& o) const;
	Expr operator*(Expr const& o) const;

	Expr& operator|=(Expr const& o);
	Expr& operator|=(Expr&& o);

	Expr& operator+=(Expr const& o);
	Expr& operator+=(Expr&& o);

	Expr& operator*=(Expr const& o);
	Expr& operator*=(Expr&& o);

public:
	bool operator<(Expr const& e) const;

public:
	inline void set_type(expr_type_id const type) { _type = type; }

	void fix_unary()
	{
		if (has_args() && (nargs() == 1)) {
			static_cast<Expr&>(*this) = std::move(args()[0]);
		}
	}

	uint64_t hash() const;

private:
	inline void destruct_args()
	{
		if (has_args()) {
			storage().destruct_args();
		}
	}

protected:
	inline ExprStorage& storage() { return _storage; }
	inline ExprStorage const& storage() const { return _storage; }


private:
	expr_type_id _type;
	ExprStorage _storage;
};
#pragma pack(pop)

typedef Expr::list ExprArgs;
typedef std::initializer_list<Expr> ExprIl;

class ExprOr;
class ExprESF;
class ExprMul;
class ExprAdd;
class ExprSym;
class ExprImm;

template <class E>
class ExprWithArgs: public Expr
{
	using storage_type = typename std::conditional<std::is_same<E, ExprESF>::value,
		  ExprESFStorage, ExprArgsStorage>::type;
public:
	ExprWithArgs(ExprIl const& il):
		Expr(E::type_id, storage_type(il))
	{ }

	ExprWithArgs(ExprArgs&& args):
		Expr(E::type_id, storage_type(std::move(args)))
	{ }

	template <class Iterator>
	explicit ExprWithArgs(Iterator begin, Iterator end):
		Expr(E::type_id, storage_type(begin, end))
	{ }

	ExprWithArgs():
		ExprWithArgs({})
	{ }


public:
	inline list& args()
	{
		return storage().args.args();
	}

	inline list const& args() const
	{
		return storage().args.args();
	}

	inline size_t nargs() const
	{
		return args().size();
	}

	inline void reserve_args(size_t const n) { args().reserve(n); }
	inline void resize_args(size_t const n) { args().resize(n); }

	bool has_args() const { return true; }

	void extend_args(ExprArgs const& args)
	{
		storage().args.extend(args);
	}

	template <class Iterator>
	void extend_args(Iterator start, Iterator end)
	{
		storage().args.extend(start, end);
	}

	void extend_move(ExprArgs&& args)
	{
		storage().args.extend_move(std::move(args));
	}

	template <class Iterator>
	void extend_move(Iterator start, Iterator end)
	{
		storage().args.extend_move(start, end);
	}

	void extend_args_no_dup(ExprArgs const& args)
	{
		storage().args.extend_no_dup(args);
	}

	template <class Iterator>
	void extend_args_no_dup(Iterator start, Iterator end)
	{
		storage().args.extend_no_dup(start, end);
	}

	void extend_move_no_dup(ExprArgs&& args)
	{
		storage().args.extend_move_no_dup(std::move(args));
	}

	template <class Iterator>
	void extend_move_no_dup(Iterator start, Iterator end)
	{
		storage().args.extend_move_no_dup(start, end);
	}


	template <class E_>
	inline void emplace_arg(E_&& e)
	{
		args().insert_dup(std::forward<E_>(e));
	}

	template <class E_>
	inline void emplace_arg_no_dup(E_&& e)
	{
		args().insert(std::forward<E_>(e));
	}

public:
	bool operator==(Expr const&) const
	{
		return false;
	}

	bool operator==(E const& o) const
	{
		return storage().args == o.storage().args;
	}

	void fix_unary()
	{
		if (nargs() == 1) {
			static_cast<Expr&>(*this) = std::move(args()[0]);
		}
	}

	uint64_t hash() const
	{
		uint64_t ret = 0;
		for (Expr const& a: args()) {
			ret = (ret << 4) | (uint64_t)a.type();
			ret = ret*0x5555555555555555ULL + a.hash();
		}
		return ret;
	}

public:
	bool operator<(E const& o) const
	{
		return storage().args < o.storage().args;
	}
};

class PA_API ExprESF: public ExprWithArgs<ExprESF>
{
	typedef ExprWithArgs<ExprESF> base_type;

public:
	static constexpr expr_type_id type_id = expr_type_id::esf_type;

public:
	using base_type::base_type;
	using degree_type = ExprESFStorage::degree_type;

public:
	ExprESF(degree_type const degree, ExprIl const& il):
		base_type(il)
	{
		init_degree(degree);
	}

	ExprESF(degree_type const degree, ExprArgs&& args):
		base_type(std::move(args))
	{
		init_degree(degree);
	}

	template <class Iterator>
	explicit ExprESF(degree_type const degree, Iterator begin, Iterator end):
		base_type(begin, end)
	{
		init_degree(degree);
	}

	ExprESF(degree_type const degree, ExprArgs const& args):
		base_type(args.begin(), args.end())
	{
		init_degree(degree);
	}


public:
	static const char* name()
	{
		return "SF";
	}

public:
	bool operator==(Expr const& o) const
	{
		return static_cast<base_type const&>(*this) == o;
	}

	bool operator==(ExprESF const& o) const
	{
		if (degree() != o.degree()) {
			return false;
		}
		return static_cast<base_type const&>(*this) == o;
	}

	bool operator<(ExprESF const& o) const
	{
		if (degree() < o.degree()) {
			return true;
		}
		if (degree() > o.degree()) {
			return false;
		}
		return static_cast<base_type const&>(*this) < o;
	}

public:
	degree_type degree() const { return storage().sf.degree(); }

	void expand();

private:
	void init_degree(degree_type const degree)
	{
		storage().sf.degree() = degree;
		if (degree == 1) {
			// That's just an addition if degree == 1. 
			// Arguments are created so let's just change the type!  Let the
			// degree at 1 so that this is also still a valid ExprESF object
			// (until the layout changes...)
			set_type(expr_type_id::add_type);
		}
		else
		if (degree == nargs()) {
			set_type(expr_type_id::mul_type);
		}
	}
};

class ExprOr: public ExprWithArgs<ExprOr>
{
	typedef ExprWithArgs<ExprOr> base_type;

public:
	static constexpr expr_type_id type_id = expr_type_id::or_type;

public:
	using base_type::base_type;

public:
	static const char* name()
	{
		return "Or";
	}

public:
	Expr& operator|=(Expr const& e);
	Expr& operator|=(Expr&& e);

	ExprOr& operator|=(ExprOr const& e)
	{
		if (&e != this) {
			extend_args_no_dup(e.args());
		}
		return *this;
	}

	ExprOr& operator|=(ExprOr&& e)
	{
		if (&e != this) {
			extend_move_no_dup(std::move(e.args()));
		}
		return *this;
	}

	Expr& operator|=(ExprMul&& e);
	Expr& operator|=(ExprMul const& e);

	Expr& operator|=(ExprESF&& e);
	Expr& operator|=(ExprESF const& e);

	Expr& operator|=(ExprAdd&& e);
	Expr& operator|=(ExprAdd const& e);

	Expr& operator|=(ExprImm const& e);

	Expr& operator|=(ExprSym const& e);
};

class PA_API ExprMul: public ExprWithArgs<ExprMul>
{
	typedef ExprWithArgs<ExprMul> base_type;

public:
	static constexpr expr_type_id type_id = expr_type_id::mul_type;

public:
	using base_type::base_type;

public:
	static const char* name()
	{
		return "Mul";
	}

public:
	Expr& operator*=(Expr const& e);
	Expr& operator*=(Expr&& e);

	ExprMul& operator*=(ExprMul const& e)
	{
		if (&e != this) {
			extend_args_no_dup(e.args());
		}
		return *this;
	}

	ExprMul& operator*=(ExprMul&& e)
	{
		if (&e != this) {
			extend_move_no_dup(std::move(e.args()));
		}
		return *this;
	}

	Expr& operator*=(ExprOr&& e);
	Expr& operator*=(ExprOr const& e);

	Expr& operator*=(ExprESF&& e);
	Expr& operator*=(ExprESF const& e);

	Expr& operator*=(ExprAdd&& e);
	Expr& operator*=(ExprAdd const& e);

	Expr& operator*=(ExprImm const& e);

	Expr& operator*=(ExprSym const& e);
};

class ExprAdd: public ExprWithArgs<ExprAdd>
{
	typedef ExprWithArgs<ExprAdd> base_type;

public:
	static constexpr expr_type_id type_id = expr_type_id::add_type;

public:
	using base_type::base_type;

public:
	static const char* name()
	{
		return "Add";
	}

public:
	template <class E_>
	inline void emplace_arg(E_&& e)
	{
		// If the expression already exists, remove it!
		auto lb = args().insert(std::forward<E_>(e));
		if (!lb.second) {
			args().erase(lb.first);
		}
	}

	template <class... Values>
	void extend_args(Values&& ... values)
	{
		args().insert(std::forward<Values>(values)..., 
			[](list& v, list::const_iterator it) { return v.erase(it); });
	}

public:
	Expr& operator+=(Expr const& e);
	Expr& operator+=(Expr&& e);

	ExprAdd& operator+=(ExprAdd const& e);
	ExprAdd& operator+=(ExprAdd&& e);

	Expr& operator+=(ExprOr&& e);
	Expr& operator+=(ExprOr const& e);

	Expr& operator+=(ExprESF&& e);
	Expr& operator+=(ExprESF const& e);

	Expr& operator+=(ExprMul&& e);
	Expr& operator+=(ExprMul const& e);

	Expr& operator+=(ExprImm const& e);

	Expr& operator+=(ExprSym const& e);
};

class PA_API ExprSym: public Expr
{
public:
	static constexpr expr_type_id type_id = expr_type_id::symbol_type;
	typedef ExprSymStorage::idx_type idx_type;

public:
	ExprSym() = default;

	ExprSym(idx_type const v):
		Expr(type_id, Expr::ExprSymStorage(v))
	{ }

public:
	inline idx_type idx() const
	{
		return storage().sym.idx();
	}

	static const char* name()
	{
		return "Sym";
	}

public:
	bool operator==(Expr const&) const
	{
		return false;
	}

	bool operator==(ExprSym const& o) const
	{
		return idx() == o.idx();
	}

public:
	bool operator<(Expr const&) const
	{
		return false;
	}

	bool operator<(ExprSym const& o) const
	{
		return idx() < o.idx();
	}

public:
	uint64_t hash() const { return (((uint64_t)type()) << sizeof(idx_type)*8) | idx(); }
};

class PA_API ExprImm: public Expr
{
public:
	static constexpr expr_type_id type_id = expr_type_id::imm_type;

public:
	ExprImm() = default;

	ExprImm(bool v):
		Expr(type_id, Expr::ExprImmStorage(v))
	{ }

public:
	static const char* name()
	{
		return "Imm";
	}

	inline bool value() const
	{
		return storage().imm.value();
	}

	inline bool& value()
	{
		return storage().imm.value();
	}

public:
	bool operator==(Expr const&) const
	{
		return false;
	}

	bool operator==(ExprImm const& o) const
	{
		return value() == o.value();
	}

public:
	bool operator<(Expr const&) const
	{
		return false;
	}

	bool operator<(ExprImm const& o) const
	{
		return value() < o.value();
	}

public:
	uint64_t hash() const { return ((uint64_t)type() << 1) | ((uint64_t)value()); }
};

template <class F>
auto Expr::call(F const& f)
{
	switch (type()) {
		case expr_type_id::or_type:
			return f(static_cast<ExprOr&>(*this));
		case expr_type_id::esf_type:
			return f(static_cast<ExprESF&>(*this));
		case expr_type_id::mul_type:
			return f(static_cast<ExprMul&>(*this));
		case expr_type_id::add_type:
			return f(static_cast<ExprAdd&>(*this));
		case expr_type_id::symbol_type:
			return f(static_cast<ExprSym&>(*this));
		case expr_type_id::imm_type:
			// Avoid "control reaches end of non-void function" warning!
			break;
	};
	return f(static_cast<ExprImm&>(*this));
}

template <class F>
auto Expr::call(F const& f) const
{
	switch (type()) {
		case expr_type_id::or_type:
			return f(static_cast<ExprOr const&>(*this));
		case expr_type_id::esf_type:
			return f(static_cast<ExprESF const&>(*this));
		case expr_type_id::mul_type:
			return f(static_cast<ExprMul const&>(*this));
		case expr_type_id::add_type:
			return f(static_cast<ExprAdd const&>(*this));
		case expr_type_id::symbol_type:
			return f(static_cast<ExprSym const&>(*this));
		case expr_type_id::imm_type:
			// Avoid "control reaches end of non-void function" warning!
			break;
	};
	return f(static_cast<ExprImm const&>(*this));
}

template <class F>
void Expr::call_no_return(F const& f)
{
	switch (type()) {
		case expr_type_id::or_type:
			f(static_cast<ExprOr&>(*this));
			break;
		case expr_type_id::esf_type:
			f(static_cast<ExprESF&>(*this));
			break;
		case expr_type_id::mul_type:
			f(static_cast<ExprMul&>(*this));
			break;
		case expr_type_id::add_type:
			f(static_cast<ExprAdd&>(*this));
			break;
		case expr_type_id::symbol_type:
			f(static_cast<ExprSym&>(*this));
			break;
		case expr_type_id::imm_type:
			f(static_cast<ExprImm&>(*this));
			break;
	};
}

template <class F>
void Expr::call_no_return(F const& f) const
{
	switch (type()) {
		case expr_type_id::or_type:
			f(static_cast<ExprOr const&>(*this));
			break;
		case expr_type_id::esf_type:
			f(static_cast<ExprESF const&>(*this));
			break;
		case expr_type_id::mul_type:
			f(static_cast<ExprMul const&>(*this));
			break;
		case expr_type_id::add_type:
			f(static_cast<ExprAdd const&>(*this));
			break;
		case expr_type_id::symbol_type:
			f(static_cast<ExprSym const&>(*this));
			break;
		case expr_type_id::imm_type:
			f(static_cast<ExprImm const&>(*this));
			break;
	};
}

namespace expr_traits
{

template <class E>
struct has_args: public std::conditional<E::type_id < expr_type_id::symbol_type, std::true_type, std::false_type>::type
{ };

}

} // pa

#endif
