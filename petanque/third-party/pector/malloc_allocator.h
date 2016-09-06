// This file is part of pector (https://github.com/aguinet/pector).
// Copyright (C) 2014-2015 Adrien Guinet <adrien@guinet.me> 
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA


#ifndef PECTOR_MALLOC_ALLOCATOR_H
#define PECTOR_MALLOC_ALLOCATOR_H

#include <cstdlib>
#include <cstddef>

#include <pector/enhanced_allocators.h>
#include <malloc.h>

namespace pt {

namespace internals {
struct dummy1 { };
struct dummy2 { };
} // internals

template <class T, bool make_reallocable = true, bool make_size_aware = false>
struct malloc_allocator: public std::conditional<make_reallocable, reallocable_allocator, internals::dummy1>::type
#ifdef __GNUC__
						 , public std::conditional<make_size_aware, size_aware_allocator, internals::dummy2>::type
#endif
{
public:
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef ptrdiff_t difference_type;
	typedef size_t size_type;
    template<class U> struct rebind {
        typedef malloc_allocator<U, make_reallocable, make_size_aware> other;
    };

public:
    malloc_allocator() throw() { }
    malloc_allocator(malloc_allocator const&) throw() { }
    template<class U>
	malloc_allocator(malloc_allocator<U> const&) throw() { }

    pointer address(reference x) const { return &x; }
    const_pointer address(const_reference x) const { return &x; }
    
    pointer allocate(size_type n, const void* /*hint*/ = 0)
	{
		pointer const ret = reinterpret_cast<pointer>(malloc(n*sizeof(value_type)));
		if (ret == nullptr) {
			throw std::bad_alloc();
		}
		return ret;
    }

    void deallocate(pointer p, size_type)
	{
		free(p);
    }

    size_type max_size() const throw()
	{
        size_type max = static_cast<size_type>(-1) / sizeof (value_type);
        return (max > 0 ? max : 1);
    }
    
    template <typename U, typename... Args>
    void construct(U *p, Args&& ... args)
	{
		::new(p) U(std::forward<Args>(args)...);
	}

    void destroy(pointer p) { p->~value_type(); }

#ifdef __GNUC__
	size_type usable_size(const_pointer p) const
	{
		return malloc_usable_size(const_cast<pointer>(p))/sizeof(value_type);
	}
#endif

	pointer realloc(pointer p, size_type const n)
	{
		pointer const ret = reinterpret_cast<pointer>(::realloc(p, n*sizeof(value_type)));
		if (ret == nullptr) {
			throw std::bad_alloc();
		}
		return ret;
	}
};

template<> 
class malloc_allocator<void>
{
public:
    typedef void* pointer;
    typedef const void* const_pointer;
    typedef void value_type;
    template<typename U> struct rebind {
        typedef malloc_allocator<U> other;
    };
};

} // pector

#endif
