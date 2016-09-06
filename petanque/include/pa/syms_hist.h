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

#ifndef PA_SYMHIST_H
#define PA_SYMHIST_H

#include <map>

#include <pa/exports.h>
#include <pa/exprs.h> // for ExprSym::idx_type

namespace pa {

class PA_API SymbolsHist
{
public:
	typedef uint32_t count_type;
	
private:
	typedef std::map<ExprSym::idx_type, count_type> storage_type;

public:
	class const_iterator: public storage_type::const_iterator
	{
		friend class SymbolsHist;
	public:
		typedef std::pair<pa::ExprSym, count_type> value_type;
		typedef value_type reference;

	public:
		const_iterator() { }
		const_iterator(const_iterator const& o):
			storage_type::const_iterator(o)
		{ }

	protected:
		const_iterator(storage_type::const_iterator const& o):
			storage_type::const_iterator(o)
		{ }

	public:
		value_type operator*() const
		{
			auto& B = base();
			return value_type{ExprSym{B->first}, B->second};
		}

	private:
		storage_type::const_iterator const& base() const { return *static_cast<storage_type::const_iterator const*>(this); }
	};

public:
	SymbolsHist() { }
	SymbolsHist(Expr const& e)
	{
		compute(e);
	}
	SymbolsHist(SymbolsHist&& o):
		_hist(std::move(o._hist))
	{ }
	SymbolsHist(SymbolsHist const& o):
		_hist(o._hist)
	{ }

public:
	bool compute(Expr const&);
	bool compute(Expr const&, unsigned args_mul);

	count_type count(ExprSym const& sym) const;
	count_type count(Expr const& sym) const;

	const_iterator begin() const { return const_iterator{_hist.begin()}; }
	const_iterator end() const { return const_iterator{_hist.end()}; }

	size_t size() const { return _hist.size(); }
	bool empty() const { return _hist.empty(); }

private:
	storage_type _hist;
};

}

#endif
