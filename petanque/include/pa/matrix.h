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

#ifndef PETANQUE_MATRIX_H
#define PETANQUE_MATRIX_H

#include <pa/exports.h>
#include <pa/vector.h>

#include <vector>

namespace pa {

class PA_API Matrix: private Vector
{
public:
	using Vector::iterator;
	using Vector::const_iterator;

public:
	Matrix():
		Vector(),
		_ncols(0)
	{ }

	Matrix(Matrix const& o):
		Vector(o),
		_ncols(o._ncols)
	{ }

	Matrix(Matrix&& o):
		Vector(std::move(o)),
		_ncols(o._ncols)
	{ }

	Matrix(size_t const nlines, size_t const ncols, pa::Expr const& e):
		Vector(nlines*ncols, e),
		_ncols(ncols)
	{ }

	Matrix(size_t const nlines, size_t const ncols):
		Vector(nlines*ncols),
		_ncols(ncols)
	{ }

	Matrix(size_t const ncols, std::initializer_list<Expr> const& args);

public:
	static Matrix identity(const size_t n);

	template <class F>
	static Matrix construct(size_t const nlines, size_t const ncols, F const& f)
	{
		pa::Matrix ret(nlines, ncols);
		for (size_t i = 0; i < nlines; i++) {
			for (size_t j = 0; j < ncols; j++) {
				ret.at(i, j) = f(i, j);
			}
		}
		return ret;
	}

public:
	Matrix& operator=(Matrix const& o)
	{
		if (&o != this) {
			_ncols = o._ncols;
			*static_cast<Vector*>(this) = o;
		}
		return *this;
	}

	Matrix& operator=(Matrix&& o)
	{
		if (&o != this) {
			_ncols = o._ncols;
			*static_cast<Vector*>(this) = std::move(o);
		}
		return *this;
	}

public:
	inline Expr& at(const size_t line, const size_t col)
	{
		return Vector::at(index(line, col));
	}

	inline Expr const& at(const size_t line, const size_t col) const
	{
		return Vector::at(index(line, col));
	}

	inline bool same_size(Matrix const& o) const
	{
		return (_ncols == o._ncols) && (size() == o.size());
	}

	inline size_t nelts() const { return size(); }
	inline Expr const& elt_at(size_t idx) const { return Vector::at(idx); }
	inline Expr& elt_at(size_t idx) { return Vector::at(idx); }

	inline size_t ncols()  const { return _ncols; }
	inline size_t nlines() const
	{
		if (ncols() == 0) {
			return 0;
		}
		return size() / ncols();
	}

	inline bool empty() const { return Vector::empty(); }

public:
	void add_lines(const size_t a, const size_t b);
	void swap_lines(const size_t a, const size_t b);
	void swap_cols(const size_t a, const size_t b);
	size_t T_fact(Matrix& T, Matrix& U, std::vector<size_t>& perm) const;
	void permute_rows(std::vector<size_t> const& perm);
	pa::Matrix inverse() const;

public:
	Matrix& operator+=(Matrix const& o);
	Matrix  operator+ (Matrix const& o) const;

	//Matrix& operator*=(Matrix const& o);
	Matrix  operator* (Matrix const& o) const;

	Vector  operator* (Vector const& o) const;

public:
	bool operator==(Matrix const& o) const;
	bool operator!=(Matrix const& o) const;

public:
	iterator begin() { return Vector::begin(); }
	const_iterator cbegin() const { return Vector::cbegin(); }
	const_iterator begin() const { return Vector::begin(); }

	iterator end() { return Vector::end(); }
	const_iterator cend() const { return Vector::cend(); }
	const_iterator end() const { return Vector::end(); }

private:
	inline size_t index(const size_t line, const size_t col) const { return line*_ncols + col; }

private:
	size_t _ncols;
};

}

#endif
