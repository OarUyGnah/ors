#ifndef __ORS_UTILS_NUMERIC_H__
#define __ORS_UTILS_NUMERIC_H__
namespace ors {
  namespace utils {
    namespace numeric {
    // template <typename T,typename From> T down_cast(const From &from) {
    //   T t = static_cast<To>(from);
    //   // The following comparison (rather than "from==small") allows
    //   // this method to convert between signed and unsigned values.
    //   assert(from - static_cast<From>(t) == 0);
    //   return t;
    // }
    template <typename T,typename From> T down_cast(From &&from) {
      T t = static_cast<T>(from);
      // The following comparison (rather than "from==small") allows
      // this method to convert between signed and unsigned values.
      assert(from - static_cast<From>(t) == 0);
      return t;
    }
    } // namespace numeric
  }
}
#endif // !__ORS_UTILS_NUMERIC_H__
