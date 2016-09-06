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


#ifndef PECTOR_ENHANCED_ALLOCATORS_H
#define PECTOR_ENHANCED_ALLOCATORS_H

namespace pt {

struct size_aware_allocator { };
struct reallocable_allocator { };

template <class Alloc>
struct is_size_aware_allocator: public std::is_base_of<size_aware_allocator, Alloc>
{ };

template <class Alloc>
struct is_reallocable_allocator: public std::is_base_of<reallocable_allocator, Alloc>
{ };

}

#endif
