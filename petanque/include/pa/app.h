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

#ifndef PETANQUE_APP_H
#define PETANQUE_APP_H

#include <pa/exports.h>
#include <pa/matrix.h>
#include <pa/vector.h>

#include <utility>

namespace pa {

// Represents an affine application, as X:->M*X+V
class PA_API AffApp
{
public:
	struct InvalidSizes: public std::exception
	{
		InvalidSizes(Matrix const&, Vector const&)
		{ }

	public:
		const char* what() const noexcept override
		{
			return "incompatible sizes of M and V: expected M a square matrix of the size of V.";
		}
	};

public:
	AffApp(Matrix const& M, Vector const& V):
		_M(M),
		_V(V)
	{
		check_sizes(M, V);
	}

	AffApp(Matrix&& M, Vector&& V):
		_M(M),
		_V(V)
	{
		check_sizes(_M, _V);
	}

	AffApp(Matrix const& M, Vector&& V):
		_M(M),
		_V(std::move(V))
	{
		check_sizes(M, V);
	}

	AffApp(Matrix&& M, Vector const& V):
		_M(std::move(M)),
		_V(V)
	{
		check_sizes(M, V);
	}

	AffApp(AffApp const& app):
		AffApp(app.matrix(), app.cst())
	{ }

	AffApp(AffApp&& app):
		AffApp(std::move(app._M), std::move(app._V))
	{ }

public:
	void set_Matrix(Matrix const& M)
	{
		check_sizes(M, _V);
		_M = M;
	}

	void set_Vector(Vector const& V)
	{
		check_sizes(_M, V);
		_V = V;
	}

	void set_Matrix(Matrix&& M)
	{
		check_sizes(M, _V);
		_M = std::move(M);
	}

	void set_Vector(Vector& V)
	{
		check_sizes(_M, V);
		_V = std::move(V);
	}

	Matrix const& matrix() const { return _M; }
	Vector const& cst() const { return _V; }

public:
	Vector operator()(Vector const& X) const
	{
		return matrix()*X+cst();
	}

public:
	inline AffApp& operator=(AffApp const& o)
	{ 
		if (&o != this) {
			_M = o.matrix(); _V = o.cst();
		}
		return *this;
	}

	inline AffApp& operator=(AffApp&& o)
	{
		if (&o != this) {
			_M = std::move(o.matrix()); _V = std::move(o.cst());
		}
		return *this;
	}

private:
	static void check_sizes(Matrix const& M, Vector const& V)
	{
		if (M.nlines() != V.size()) {
			throw InvalidSizes(M, V);
		}
	}

private:
	Matrix _M;
	Vector _V;
};

// Represents an application defined through a vector
class PA_API VectorApp
{
public:
    VectorApp(Vector const& symbols, Vector const& V)
    {
        set(symbols, V);
    }

    VectorApp(VectorApp const& o): 
		_V(o._V)
    { }

    VectorApp(VectorApp&& o): 
		_V(std::move(o._V))
    { }

public:
	void set(Vector const& symbols, Vector const& V);
	Vector const& vector() const { return _V; }

public:
    Vector operator()(Vector const& x) const;

private:
    Vector _V;
};

// Represents a generic application, as X:->NL(X) + M*X + V
class PA_API App
{
public:
    App(VectorApp const& NL, AffApp const& aff):
        _NL(NL),
        _aff(aff)
    {
        check_sizes(NL, aff);
    }

    App(VectorApp&& NL, AffApp&& aff):
        _NL(std::move(NL)),
        _aff(std::move(aff))
    {
        check_sizes(_NL, _aff);
    }
    
    App(VectorApp const& NL, Matrix const& M, Vector const& V):
        _NL(NL),
        _aff(M, V)
    {
        check_sizes(NL, _aff);
    }

    App(VectorApp&& NL, Matrix&& M, Vector&& V):
        _NL(std::move(NL)),
        _aff(std::forward<Matrix>(M), std::forward<Vector>(V))
    {
        check_sizes(_NL, _aff);
    }

    App(App const &o):
        _NL(o._NL),
        _aff(o._aff)
    { }

    App(App&& o):
        _NL(std::move(o._NL)),
        _aff(std::move(o._aff))
    { }

public:
    VectorApp const& nl() const { return _NL; }

    AffApp const& affine() const { return _aff; }
    Vector const& cst() const { return _aff.cst(); }
    Matrix const& matrix() const { return _aff.matrix(); }

public:
	static void check_sizes(VectorApp const&, AffApp const&) { }

public:
	Vector operator()(Vector const& X) const
	{
		return nl()(X) + affine()(X);
	}

private:
    VectorApp _NL;
    AffApp _aff;
};

}

#endif
