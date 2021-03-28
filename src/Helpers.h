#ifndef EET_HELPERS_H
#define EET_HELPERS_H

#include <cstdint>
#include <cassert>

namespace Eet {
  class Helpers {
    public:
      template<typename T>
      static T
      bits2type(uint32_t src, uint32_t byte, uint32_t bit, uint32_t n) {
        assert(byte < 4);
        bit += byte * 8;
        assert(bit + n - 1 < 32);
        uint32_t mask = 0U;
        for(uint32_t i = 0U; i < n; ++i) {
          mask |= (1U << i);
        }
        src = (src >> bit) & mask;
        return static_cast<T>(src);
      }


      template<typename T>
      static void setBits(uint32_t &target, T value, uint32_t byte,
                            uint32_t bit, uint32_t n) {
        assert(byte < 4);
        bit += byte * 8;
        assert(bit + n - 1 < 32);
        uint32_t mask = 0U;
        for(uint32_t i = 0U; i < n; ++i) {
          mask |= (1U << i);
        }
        // wipe in target
        target &= ~(mask << bit);
        // set in target
        target |= (static_cast<uint32_t>(value) & mask) << bit;
      }


      template<typename T>
      static T
      enum2type(uint32_t data, uint32_t byte, uint32_t bit, uint32_t n,
                T lowerBound, T upperBound) {
        auto val = bits2type<uint32_t>(data, byte, bit, n);
        T ret = T::INVALID;
        if((val >= static_cast<uint32_t>(lowerBound)) &&
           (val <= static_cast<uint32_t>(upperBound))) {
          ret = static_cast<T>(val);
        }
        return ret;
      }
  };
} // end namespace Eet

#endif //EET_HELPERS_H
