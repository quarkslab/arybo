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

#include <pa/compat.h>
#include <pa/errors.h>
#include <pa/matrix.h>

pa::Matrix::Matrix(size_t const ncols, std::initializer_list<Expr> const& args):
	Vector(),
	_ncols(ncols)
{
	if ((args.size() == 0) || (args.size() % ncols != 0)) {
		_ncols = 0;
		return;
	}

	_args = args;
}

pa::Matrix& pa::Matrix::operator+=(pa::Matrix const& o)
{
	if (!same_size(o)) {
		throw errors::SizeMismatch();
	}

	return static_cast<Matrix&>(static_cast<Vector&>(*this) += static_cast<Vector const&>(o));
}

pa::Matrix pa::Matrix::operator+(pa::Matrix const& o) const
{
	if (!same_size(o)) {
		throw errors::SizeMismatch();
	}

	if (&o == this) {
		return Matrix(nlines(), ncols(), pa::ExprImm(0));
	}

	Matrix ret;
	ret._ncols = ncols();
	storage_type& ret_args = ret.args();

	const size_t size_ = size();
	ret_args.reserve(size_);

	storage_type const& this_args = args();
	storage_type const& o_args = o.args();
	for (size_t i = 0; i < size_; i++) {
		ret_args.emplace_back(this_args[i] + o_args[i]);
	}
	
	return ret;
}

pa::Vector pa::Matrix::operator*(Vector const& o) const
{
	const size_t ncols_ = ncols();
	if (o.size() != ncols_) {
		throw errors::SizeMismatch();
	}

	pa::Vector ret;
	pa::Vector::storage_type& ret_args = ret.args();
	ret_args.reserve(o.size());

	const size_t nlines_ = nlines();
	for (size_t i = 0; i < nlines_; i++) {
		pa::Expr add = pa::ExprAdd();
		pa::ExprArgs& add_args = add.args();
		for (size_t j = 0; j < ncols_; j++) {
			add_args.insert_dup(at(i, j) * o.at(j));
		}
		ret_args.emplace_back(std::move(add));
	}

	return ret;
}

pa::Matrix pa::Matrix::identity(const size_t n)
{
	pa::Matrix ret(n, n, pa::ExprImm(0));
	for (size_t i = 0; i < n; i++) {
		ret.at(i, i) = pa::ExprImm(1);
	}
	return ret;
}

pa::Matrix pa::Matrix::operator*(Matrix const& o) const
{
	if (size() != o.size() || o.ncols() != nlines()) {
		throw errors::SizeMismatch();
	}
	pa::Matrix ret(nlines(), o.ncols());
	for (size_t i = 0; i < nlines(); i++) {
		for (size_t j = 0; j < o.ncols(); j++) {
			pa::Expr add = pa::ExprAdd();
			pa::ExprArgs& add_args = add.args();
			add_args.reserve(ncols());
			for (size_t k = 0; k < ncols(); k++) {
				add_args.insert_dup(at(i,k) * o.at(k, j));
			}
			ret.at(i, j) = std::move(add);
		}	
	}

	return ret;
}

bool pa::Matrix::operator==(Matrix const& o) const
{
	if (this == &o) {
		return true;
	}

	if (_ncols != o._ncols) {
		return false;
	}

	return static_cast<Vector const&>(*this) == static_cast<Vector const&>(o);
}

bool pa::Matrix::operator!=(Matrix const& o) const
{
	return !(*this == o);
}

// Mainly inspired by http://itpp.sourceforge.net/4.3.1/gf2mat_8cpp_source.html#l00561
size_t pa::Matrix::T_fact(Matrix& T, Matrix& U, std::vector<size_t>& perm) const
{
	T = identity(nlines());
	U = *this;

	const size_t ncols_ = ncols();
	const size_t nlines_ = nlines();
	perm.resize(ncols_);
	for (size_t i = 0; i < ncols_; i++) {
		perm[i] = i;
	}

	for (size_t j = 0; j < nlines_; j++) {
		size_t i1, j1;
		for (i1 = j; i1 < nlines_; i1++) {
			for (j1 = j; j1 < ncols_; j1++) {
				if (U.at(i1, j1) == ExprImm(1)) {
					goto found;
				}
			}
		}

		return j;

found:
		U.swap_lines(i1, j);
		T.swap_lines(i1, j);
		U.swap_cols(j1, j);

		std::swap(perm[j], perm[j1]);

		for (size_t i1 = j + 1; i1 < nlines_; i1++) {
			if (U.at(i1, j) == ExprImm(1)) {
				U.add_lines(i1, j);
				T.add_lines(i1, j);
			}
		}
	}
	return nlines_;
}

void pa::Matrix::add_lines(const size_t a, const size_t b)
{
	const size_t ncols_ = ncols();
	for (size_t j = 0; j < ncols_; j++) {
		at(a, j) += at(b, j);
	}
}

void pa::Matrix::swap_lines(const size_t a, const size_t b)
{
	const size_t ncols_ = ncols();
	// TODO: optimize
	for (size_t j = 0; j < ncols_; j++) {
		std::swap(at(a, j), at(b, j));
	}
}

void pa::Matrix::swap_cols(const size_t a, const size_t b)
{
	const size_t nlines_ = nlines();
	for (size_t i = 0; i < nlines_; i++) {
		std::swap(at(i, a), at(i, b));
	}
}

// Mainly inspired by http://itpp.sourceforge.net/4.3.1/gf2mat_8cpp_source.html#l00561
pa::Matrix pa::Matrix::inverse() const
{
	if (nlines() != ncols()) {
		return pa::Matrix();
	}

	// first compute the T-factorization
	pa::Matrix T, U;
	std::vector<size_t> perm;
	size_t rank = T_fact(T, U, perm);
	if (rank != ncols()) {
		return pa::Matrix();
	}

	// backward substitution
	for (ssize_t i = ncols() - 2; i >= 0; i--) {
		for (ssize_t j = ncols() - 1; j > i; j--) {
			if (U.at(i, j) == ExprImm(1)) {
				U.add_lines(i, j);
				T.add_lines(i, j);
			}
		}
	}
	T.permute_rows(perm);
	return T;
}

void pa::Matrix::permute_rows(std::vector<size_t> const& perm)
{
	const size_t nlines_ = nlines();
	const size_t ncols_ = ncols();

	Matrix tmp = *this;
	for (size_t i = 0; i < nlines_; i++) {
		for (size_t j = 0; j < ncols_; j++) {
			at(perm[i], j) = tmp.at(i, j);
		}
	}
}
