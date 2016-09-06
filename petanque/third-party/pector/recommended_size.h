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


#ifndef PECTOR_RECOMMENDED_SIZE_H
#define PECTOR_RECOMMENDED_SIZE_H

namespace pt {

struct recommended_size_dummy
{
	template <class SizeType>
	static inline SizeType recommended(SizeType const /*ms*/, SizeType const /*old_cap*/, SizeType const new_cap)
	{
		return new_cap;
	}
};

template <size_t Num = 3, size_t Dem = 2>
struct recommended_size_multiply_by
{
	static_assert(Num > Dem, "Num <= Dem !");

	template <class SizeType>
	static inline SizeType recommended(SizeType const ms, SizeType const old_cap, SizeType const new_cap)
	{
#ifndef  _MSC_VER
		static_assert(Num < std::numeric_limits<SizeType>::max(), "Num is too big for current size_type");
		static_assert(Dem < std::numeric_limits<SizeType>::max(), "Dem is too big for current size_type");
#endif

		if (old_cap >= ((ms/Num)*Dem)) {
			return ms;
		}
		return std::max((SizeType)((Num*old_cap+(Dem-1))/Dem), new_cap);
	}
};

template <size_t N = 15>
struct recommended_size_add_by
{
	template <class SizeType>
	static inline SizeType recommended(SizeType const& ms, SizeType const old_cap, SizeType const new_cap)
	{
#ifndef _MSC_VER
		static_assert(N < std::numeric_limits<SizeType>::max(), "N is too big for current size_type");
#endif
		static constexpr SizeType N_ = N;

		if (old_cap >= (ms-N_)) {
			return ms;
		}
		return std::max((SizeType)(old_cap+N_), new_cap);
	}
};

typedef recommended_size_multiply_by<3, 2> default_recommended_size;

} // pector

#endif
