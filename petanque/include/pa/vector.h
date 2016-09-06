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

#ifndef PETANQUE_VECTOR_H
#define PETANQUE_VECTOR_H

#include <pa/exports.h>
#include <pa/exprs.h>

namespace pa {

class Matrix;

class PA_API Vector
{
	friend class Matrix;

public:
	typedef std::vector<Expr> storage_type;
	typedef storage_type::iterator iterator;
	typedef storage_type::const_iterator const_iterator;

public:
	Vector()
	{ }

	Vector(Vector&& o):
		_args(std::move(o._args))
	{ }

	Vector(Vector const& o):
		_args(o._args)
	{ }

	Vector(const size_t n):
		_args(n, pa::ExprImm(0))
	{ }

	Vector(const size_t n, Expr const& e):
		_args(n, e)
	{ }

	Vector(std::initializer_list<Expr>&& init):
		_args(std::move(init))
	{ }

public:
	Vector& operator=(Vector const& o)
	{
		if (&o != this) {
			_args = o._args;
		}
		return *this;
	}

	Vector& operator=(Vector&& o)
	{
		if (&o != this) {
			_args = std::move(o._args);
		}
		return *this;
	}

public:
	inline Expr& at(const size_t i)
	{
		assert(i < _args.size());
		return _args[i];
	}

	inline Expr const& at(const size_t i) const
	{
		assert(i < _args.size());
		return _args[i];
	}

	inline size_t size() const { return _args.size(); }

	void resize(const size_t n)
	{
		_args.resize(n);
	}

	void set_null();

	inline storage_type& args() { return _args; }
	inline storage_type const& args() const { return _args; }

	inline bool empty() const { return _args.size() == 0; }

public:
	size_t get_int_le(bool* res = nullptr) const;
	size_t get_int_be(bool* res = nullptr) const;

	void set_int_be(size_t v, const uint16_t nbits);
	void set_int_le(size_t v, const uint16_t nbits);

public:
	Vector& operator<<=(const size_t n);
	Vector  operator<<(const size_t n) const;

	Vector& operator>>=(const size_t n);
	Vector  operator>>(const size_t n) const;

public:
	inline Expr& operator[](const size_t i){ return at(i); }
	inline Expr const& operator[](const size_t i) const { return at(i); }

public:
	Vector& operator+=(Vector const& o);
	Vector  operator+(Vector const& o) const;

	Vector& operator*=(Vector const& o);
	Vector  operator*(Vector const& o) const;

	Vector& operator|=(Vector const& o);
	Vector  operator|(Vector const& o) const;

	Vector& operator*=(Expr const& e);
	Vector  operator*(Expr const& e) const;

public:
	bool operator==(Vector const& o) const;
	bool operator!=(Vector const& o) const;

public:
	iterator begin() { return _args.begin(); }
	const_iterator begin() const { return _args.begin(); }
	const_iterator cbegin() const { return _args.cbegin(); }

	iterator end() { return _args.end(); }
	const_iterator end() const { return _args.end(); }
	const_iterator cend() const { return _args.cend(); }

private:
	storage_type _args;
};

}

#endif
