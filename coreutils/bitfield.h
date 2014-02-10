#ifndef UTILS_BITFIELD_H
#define UTILS_BITFIELD_H

#include <vector>
#include <cstdint>
#include <cstring>
#include <coreutils/log.h>

class BitField {
	struct BitRef {
		BitRef(BitField &bf, int pos) : bf(bf), pos(pos) {}
		BitRef& operator=(bool b) { bf.set(pos, b); return *this; }
		operator int() { return bf.get(pos) ? 1 : 0; }
		operator bool() { return bf.get(pos); }
		BitField &bf;
		int pos;
	};
public:
		BitField(int size=0) : bits((size+63)/64) {
			if(size)
				memset(&bits[0], 0, bits.size()*8);
		};

		void grow(int pos) {
			int sz = (pos+63)/64;
			if(sz > bits.size())
				bits.resize(sz);
		}

		void set(uint8_t *ptr, int size) {
			bits.resize((size+63)/64);
			memcpy(&bits[0], ptr, size);
		};
		void *get() {
			return &bits[0];
		};

		int size() { return bits.size() * 64; }

		void set(int pos, bool value) {
			grow(pos);
			if(value)
				bits[pos>>6] |= (1<<(pos&0x3f));
			else
				bits[pos>>6] &= ~(1<<(pos&0x3f));
		}
		bool get(int pos) const {
			if(pos>>6 >= bits.size())
				return false;
			return (bits[pos>>6] & (1<<(pos&0x3f))) != 0;
		}

		int first_bit_clr(uint64_t x) {
			uint64_t m = 1;
			int i=0;
			while(i<64) {
				if((x & m) == 0)
					return i;
				i++;
				m <<= 1;
			}
			return -1;
		}

		int first_bit_set(uint64_t x) {
			uint64_t m = 1;
			int i=0;
			while(i<64) {
				if(x & m)
					return i;
				i++;
				m <<= 1;
			}
			return -1;
		}

		int lowest_set() {
			for(uint64_t i : bits) {
				LOGD("%x", i);
				if(i) {
					int o = first_bit_set(i);
					return i/64+o;
				}
			}
			return -1;
		}

		int lowest_unset() {
			for(uint64_t i : bits) {
				LOGD("U %x", i);
				if(i != 0xffffffffffffffffL) {
					int o = first_bit_clr(i);
					return i/64+o;
				}
			}
			return -1;
		}

		BitRef operator[](int pos) { return BitRef(*this, pos); }


private:
	std::vector<uint64_t> bits;
};



#endif // UTILS_BITFIELD_H