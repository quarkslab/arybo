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

#include <pa/errors.h>
#include <pa/vector.h>
#include <pa/simps.h>

void pa::Vector::set_null()
{
	for (Expr& a: args()) {
		a.set<pa::ExprImm>(0);
	}
}

pa::Vector& pa::Vector::operator+=(pa::Vector const& o)
{
	if (&o == this) {
		set_null();
		return *this;
	}

	if (size() != o.size()) {
		throw errors::SizeMismatch();
	}

	const size_t size_ = size();
	for (size_t i = 0; i < size_; i++) {
		at(i) += o.at(i);
	}

	return *this;
}

pa::Vector pa::Vector::operator+(pa::Vector const& o) const
{
	if (&o == this) {
		return pa::Vector(size(), ExprImm(0));
	}

	if (size() != o.size()) {
		throw errors::SizeMismatch();
	}

	pa::Vector ret;
	storage_type& ret_args = ret.args();
	ret_args.reserve(size());

	const size_t size_ = size();
	for (size_t i = 0; i < size_; i++) {
		ret_args.emplace_back(at(i) + o.at(i));
	}

	return ret;
}

pa::Vector& pa::Vector::operator*=(pa::Vector const& o)
{
	if (&o == this) {
		return *this;
	}

	if (size() != o.size()) {
		throw errors::SizeMismatch();
	}

	const size_t size_ = size();
	for (size_t i = 0; i < size_; i++) {
		at(i) *= o.at(i);
	}

	return *this;
}

pa::Vector pa::Vector::operator*(pa::Vector const& o) const
{
	if (&o == this) {
		return *this;
	}

	if (size() != o.size()) {
		throw errors::SizeMismatch();
	}

	pa::Vector ret;
	storage_type& ret_args = ret.args();
	ret_args.reserve(size());

	const size_t size_ = size();
	for (size_t i = 0; i < size_; i++) {
		ret_args.emplace_back(at(i) * o.at(i));
	}

	return ret;
}

pa::Vector& pa::Vector::operator*=(pa::Expr const& e)
{
	const size_t size_ = size();
	for (size_t i = 0; i < size_; i++) {
		at(i) *= e;
	}

	return *this;
}

pa::Vector pa::Vector::operator*(pa::Expr const& e) const
{
	pa::Vector ret;
	storage_type& ret_args = ret.args();
	ret_args.reserve(size());

	const size_t size_ = size();
	for (size_t i = 0; i < size_; i++) {
		ret_args.emplace_back(at(i) * e);
	}

	return ret;
}

pa::Vector& pa::Vector::operator|=(pa::Vector const& o)
{
	if (&o == this) {
		return *this;
	}

	if (size() != o.size()) {
		throw errors::SizeMismatch();
	}

	const size_t size_ = size();
	for (size_t i = 0; i < size_; i++) {
		at(i) |= o.at(i);
	}

	return *this;
}

pa::Vector pa::Vector::operator|(pa::Vector const& o) const
{
	if (&o == this) {
		return *this;
	}

	if (size() != o.size()) {
		throw errors::SizeMismatch();
	}

	pa::Vector ret;
	storage_type& ret_args = ret.args();
	ret_args.reserve(size());

	const size_t size_ = size();
	for (size_t i = 0; i < size_; i++) {
		ret_args.emplace_back(at(i) | o.at(i));
	}

	return ret;
}

pa::Vector& pa::Vector::operator>>=(const size_t n)
{
	if (n >= size()) {
		set_null();
		return *this;
	}

	std::move(_args.begin(), _args.end()-n, _args.begin()+n);
	for (size_t i = 0; i < n; i++) {
		_args[i] = pa::ExprImm(0);
	}
	return *this;
}

pa::Vector pa::Vector::operator>>(const size_t n) const
{
	pa::Vector ret;
	storage_type& new_args = ret.args();
	new_args.resize(size(), pa::ExprImm(0));
	if (n < size()) {
		std::copy(_args.begin(), _args.end()-n, new_args.begin()+n);
	}
	return ret;
}

pa::Vector& pa::Vector::operator<<=(const size_t n)
{
	if (n >= size()) {
		set_null();
		return *this;
	}

	std::move(_args.begin()+n, _args.end(), _args.begin());
	const size_t size_ = size();
	for (size_t i = size_-n; i < size_; i++) {
		_args[i] = pa::ExprImm(0);
	}
	return *this;
}

pa::Vector pa::Vector::operator<<(const size_t n) const
{
	pa::Vector ret = *this;
	ret <<= n;
	return ret;
}

size_t pa::Vector::get_int_be(bool* res) const
{
	const size_t n = std::min(sizeof(size_t)*8, size());
	// Fast path
	if (n == 0) {
		if (res) {
			*res = true;
		}
		return 0;
	}

	size_t ret = 0;
	bool res_ = true;
	for (size_t i = 0; i < n; i++) {
		Expr const& e = at(i);
		if (e.type() != expr_type_id::imm_type) {
			res_ = false;
			break;
		}
		const bool bit = e.as<pa::ExprImm>().value();
		if (bit) {
			ret |= 1ULL<<i;
		}
	}
	if (res) {
		*res = res_;
	}
	return ret;
}

size_t pa::Vector::get_int_le(bool* res) const
{
	const size_t n = std::min(sizeof(size_t)*8, size());
	// Fast path
	if (n == 0) {
		if (res) {
			*res = true;
		}
		return 0;
	}
	size_t ret = 0;
	bool res_ = true;
	for (size_t i = 0; i < n; i++) {
		Expr const& e = at(i);
		if (e.type() != expr_type_id::imm_type) {
			res_ = false;
			break;
		}
		const bool bit = e.as<pa::ExprImm>().value();
		if (bit) {
			ret |= 1ULL<<(n-i-1);
		}
	}
	if (res) {
		*res = res_;
	}
	return ret;
}

void pa::Vector::set_int_be(size_t v, const uint16_t nbits)
{
	// Fast path
	if (v == 0 || nbits == 0) {
		_args.resize(nbits, pa::ExprImm(0));
		return;
	}
		
	_args.clear();
	_args.reserve(nbits);
	uint16_t i;
	for (i = 0; (v > 0) && (i < nbits); i++) {
		_args.emplace_back(pa::ExprImm(v & 1));
		v >>= 1;
	}
	for (; i < nbits; i++) {
		_args.emplace_back(pa::ExprImm(0));
	}
}

void pa::Vector::set_int_le(size_t v, const uint16_t nbits)
{
	if (v == 0 || nbits == 0) {
		_args.resize(nbits, pa::ExprImm(0));
		return;
	}

	_args.resize(nbits);

	for (int32_t i = nbits-1; i >= 0; i--) {
		_args[i] = pa::ExprImm(v & 1);
		v >>= 1;
	}
}

bool pa::Vector::operator==(Vector const& o) const
{
	if (this == &o) {
		return true;
	}

	if (size() != o.size()) {
		return false;
	}

	const_iterator it   = begin();
	const_iterator it_o = o.begin();
	const_iterator it_end = end();
	for (; it != it_end; ++it, ++it_o) {
		if (*it != *it_o) {
			return false;
		}
	}
	return true;
}

bool pa::Vector::operator!=(Vector const& o) const
{
	return !(*this == o);
}
