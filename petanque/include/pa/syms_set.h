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

#ifndef PA_SYM_SET
#define PA_SYM_SET

#include <set>

#include <pa/exports.h>
#include <pa/cast.h>
#include <pa/exprs.h>

namespace pa {

class PA_API SymbolsSet
{
	typedef std::set<pa::ExprSym::idx_type> storage_type;

public:
	class PA_LOCAL const_iterator: public storage_type::const_iterator
	{
		friend class SymbolsSet;
	public:
		typedef pa::ExprSym value_type;
		typedef pa::ExprSym reference;


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
		inline ExprSym operator*() const { return ExprSym{*base()}; }

	private:
		inline storage_type::const_iterator& base() { return *static_cast<storage_type::const_iterator*>(this); }
		inline storage_type::const_iterator const& base() const { return *static_cast<storage_type::const_iterator const*>(this); }
	};

public:
	bool insert(ExprSym const& e);
	bool has(ExprSym const& e) const;
	bool insert(Expr const& e);
	bool has(Expr const& e) const;

	inline const_iterator begin() const { return const_iterator{set().begin()}; }
	inline const_iterator end() const { return const_iterator{set().end()}; }

	inline size_t size() const { return _set.size(); }
	inline bool empty() const { return _set.empty(); }

private:
	inline storage_type& set() { return _set; }
	inline storage_type const& set() const { return _set; }
private:
	storage_type _set;
};

} // pa

#endif
