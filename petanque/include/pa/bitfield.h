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


#ifndef PA_BITFIELD_H
#define PA_BITFIELD_H

#include <boost/integer/static_log2.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include <memory>
#include <cstring>
#include <utility>
#include <algorithm>

#include <pa/compat.h>
#include <pa/exports.h>

// AG: copy-paste from leeloo's code.
// TODO: copy unittest code for this class too!

namespace pa {

class PA_API bitfield
{
public:
	typedef size_t integer_type;
	typedef std::allocator<integer_type> allocator_type;
	typedef size_t size_type;

private:
	static constexpr size_type bits_per_chunk = sizeof(integer_type)*8;
	static constexpr integer_type mask_chunk_bit = bits_per_chunk-1;
	static constexpr integer_type one = 1;
	static constexpr int ln2_bits_per_chunk = boost::static_log2<bits_per_chunk>::value;

public:
	class bit_reference
	{
	public:
		bit_reference():
			_chunk(nullptr),
			_bit(0)
		{ }

		explicit bit_reference(integer_type* chunk, uint8_t bit):
			_chunk(chunk),
			_bit(bit)
		{ }

		bit_reference(bit_reference const& o):
			_chunk(o._chunk),
			_bit(o._bit)
		{ }

	public:
		inline int bit() const { return _bit; }

		inline operator bool() const
		{
			return (*_chunk) & (one << _bit);
		}

		inline void set_value(bool const v)
		{
			if (v) {
				*_chunk |= (one << _bit);
			}
			else {
				*_chunk &= ~(one << _bit);
			}
		}

		inline bool operator==(bit_reference const& o) const
		{
			return (_chunk == o._chunk) && (_bit == o._bit);
		}

		inline void increment()
		{
			if (_bit == (bits_per_chunk-1)) {
				_chunk++;
				_bit = 0;
			}
			else {
				_bit++;
			}
		}

		inline void decrement()
		{
			if (_bit == 0) {
				_chunk--;
				_bit = bits_per_chunk-1;
			}
			else {
				_bit--;
			}
		}

		void advance(ssize_t n)
		{
			const ssize_t nchunks = n/(ssize_t)bits_per_chunk;
			_chunk += nchunks;
			int new_bit;
			if (n < 0) {
				new_bit = _bit - ((-n)&mask_chunk_bit);
			}
			else {
				new_bit = _bit + (n&mask_chunk_bit);
			}
			if (new_bit >= (ssize_t)bits_per_chunk) {
				_chunk++;
				_bit = new_bit-bits_per_chunk;
			}
			else
			if (new_bit < 0) {
				_chunk--;
				_bit = bits_per_chunk+new_bit;
			}
			else {
				_bit = new_bit;
			}
		}

		inline ssize_t distance_to(bit_reference const& o) const
		{
			return (std::distance(_chunk, o._chunk)<<ln2_bits_per_chunk) + ((int)(o._bit) - (int)_bit);
		}

	public:
		integer_type* _chunk;
		uint8_t _bit;
	};

	class bit_value
	{
	public:
		bit_value()
		{ }

		bit_value(bit_reference const& r):
			_r(r)
		{ }

	public:
		inline operator bool() const { return (bool) _r; }
		inline void operator=(bit_value const& o)
		{
			const bool v = (bool) o;
			_r.set_value(v);
		}
		inline void operator=(bool o)
		{
			_r.set_value(o);
		}
		inline bit_reference& ref() { return _r; }
		inline bit_reference const& ref() const { return _r; }

		inline int bit() const { return _r.bit(); }
	
	private:
		bit_reference _r;
	};

public:
	class bitfield_iterator: public boost::iterator_facade<bitfield_iterator, bool, boost::random_access_traversal_tag, bit_value>
	{
	public:
		bitfield_iterator()
		{ }

		bitfield_iterator(bit_reference const& ref):
			_ref(ref)
		{ }

	public:
		friend class boost::iterator_core_access;

		inline void increment() { ref().increment(); }
		inline void decrement() { ref().decrement(); }
		inline void advance(ssize_t n) { ref().advance(n); }
		inline bool equal(bitfield_iterator const& o) const
		{
			return ref() == o.ref();
		}
		inline ssize_t distance_to(bitfield_iterator const& o) const
		{
			return ref().distance_to(o.ref());
		}

		bit_value dereference() const { return bit_value(const_cast<bitfield_iterator*>(this)->_ref); }

	private:
		inline bit_reference& ref() { return _ref; }
		inline bit_reference const& ref() const { return _ref; }

	private:
		bit_reference _ref;
	};

	typedef bitfield_iterator iterator;

public:
	bitfield():
		_buf(nullptr),
		_size(0)
	{ }

	bitfield(size_t nbits):
		_buf(nullptr),
		_size(0)
	{
		reserve(nbits);
	}

	~bitfield()
	{
		if (_buf) {
			allocator_type().deallocate(_buf, _size);
		}
	}

	bitfield(bitfield const& o):
		_buf(nullptr)
	{
		copy(o);
	}

	bitfield(bitfield&& o):
		_buf(nullptr)
	{
		move(std::move(o));
	}

public:
	void clear_all()
	{
		if (size_chunks() == 0 || buffer() == nullptr) {
			return;
		}
		memset(buffer(), 0, sizeof(integer_type)*size_chunks());
	}

	void set_all()
	{
		if (_size == 0 || _buf == nullptr) {
			return;
		}
		memset(buffer(), 0xFF, sizeof(integer_type)*size_chunks());
	}

	void set_all_odd()
	{
		if (_size == 0 || _buf == nullptr) {
			return;
		}
		memset(buffer(), 0xAA, sizeof(integer_type)*size_chunks());
	}

	void set_all_even()
	{
		if (_size == 0 || _buf == nullptr) {
			return;
		}
		memset(buffer(), 0x55, sizeof(integer_type)*size_chunks());
	}

	void set_random();

	void clear_storage()
	{
		if (_buf) {
			allocator_type().deallocate(_buf, _size);
			_buf = nullptr;
			_size = 0;
		}
	}

	inline void reserve(size_type const nbits)
	{
		const size_type req_size = bit_count_to_chunk_count(nbits);
		if (req_size <= size_chunks()) {
			return;
		}
		if (buffer() == nullptr) {
			_buf = allocator_type().allocate(req_size);
			if (_buf == nullptr) {
				throw std::bad_alloc();
			}
			_size = req_size;
			return;
		}

		const size_type new_size = std::max(size_chunks() + 64, req_size);
		integer_type* new_buf = allocator_type().allocate(new_size);
		if (new_buf == nullptr) {
			throw std::bad_alloc();
		}
		memcpy(new_buf, buffer(), size_chunks()*sizeof(integer_type));
		allocator_type().deallocate(buffer(), size_chunks());
		_size = new_size;
		_buf = new_buf;
	}

	inline void set_bit(size_type const idx)
	{
		reserve(idx+1);
		set_bit_fast(idx);
	}

	inline void clear_bit(size_type const idx)
	{
		reserve(idx+1);
		clear_bit_fast(idx);
	}

	inline bool get_bit(size_type const idx) const
	{
		if (idx >= size_bits()) {
			return false;
		}
		return get_bit_fast(idx);
	}

	inline void set_bit_fast(size_type const idx)
	{
		chunk_at(bit_index_to_chunk(idx)) |= one<<(bit_index_to_chunk_bit(idx));
	}

	inline void clear_bit_fast(size_type const idx)
	{
		chunk_at(bit_index_to_chunk(idx)) &= ~(one<<(bit_index_to_chunk_bit(idx)));
	}

	inline bool get_bit_fast(size_type const idx) const
	{
		return chunk_at(bit_index_to_chunk(idx)) & (one<<(bit_index_to_chunk_bit(idx)));
	}

	bool compare(bitfield const& o) const
	{
		if (size_chunks() != o.size_chunks()) {
			return false;
		}
		for (size_type i = 0; i < size_chunks(); i++) {
			if (chunk_at(i) != o.chunk_at(i)) {
				return false;
			}
		}
		return true;
	}

	inline bool operator!=(bitfield const& o) const { return !compare(o); }
	inline bool operator==(bitfield const& o) const { return compare(o); }

public:
	iterator begin() { return bitfield_iterator(bit_reference(_buf, 0)); }
	iterator end()   { return bitfield_iterator(bit_reference(_buf+_size, 0)); }

public:
	bitfield& operator=(bitfield const& o)
	{
		if (&o != this) {
			copy(o);
		}
		return *this;
	}

	bitfield& operator=(bitfield&& o)
	{
		if (&o != this) {
			move(std::move(o));
		}
		return *this;
	}

private:
	inline integer_type* buffer() { return _buf; }
	inline integer_type const* buffer() const { return _buf; }

	inline integer_type& chunk_at(size_type const n) { return buffer()[n]; }
	inline integer_type const& chunk_at(size_type const n) const { return buffer()[n]; }

	inline size_type size_chunks() const { return _size; }
	inline size_type size_bits() const { return size_chunks()*bits_per_chunk; }

private:
	void copy(bitfield const& o)
	{
		if (_buf) {
			allocator_type().deallocate(_buf, _size);
		}
		_buf = allocator_type().allocate(o.size_chunks());
		if (_buf == nullptr) {
			throw std::bad_alloc();
		}
		_size = o.size_chunks();
		memcpy(buffer(), o.buffer(), size_chunks()*sizeof(integer_type));
	}

	void move(bitfield&& o)
	{
		if (_buf) {
			allocator_type().deallocate(_buf, _size);
		}
		_buf = o._buf;
		_size = o._size;
		o._buf = nullptr;
		o._size = 0;
	}

private:
	static inline size_type bit_index_to_chunk(size_type const n) { return n >> ln2_bits_per_chunk; }
	static inline size_type bit_index_to_chunk_bit(size_type const n) { return n & mask_chunk_bit; }
	static inline size_type chunk_to_bit_index(size_type const n) { return n << ln2_bits_per_chunk; }
	static inline size_type bit_count_to_chunk_count(size_type const nbits) { return (nbits+bits_per_chunk-1)/bits_per_chunk; }

private:
	integer_type* _buf;
	size_t _size;
};

}

/*namespace std {

template <>
void swap(pa::bitfield::bit_value& a, pa::bitfield::bit_value& b)
{
	const bool tmp = b;
	b = a;
	a = tmp;
}

}*/

#endif
