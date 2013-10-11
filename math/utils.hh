#ifndef __SXGE_MATH_UTILS_HH__
#define __SXGE_MATH_UTILS_HH__

#include <math.h>

#ifndef M_PI
	#define M_PI (4.0 * std::atan(1.0))
#endif

namespace sxge {
	#define clamp(x, min, max) \
		do {\
			if (x < min) { \
				x = min; \
			} \
			else if (x > max) { \
				x = max; \
			} \
		} while (0)

	template <class T>
	static inline T degToRad(const T &x) {
		return x * M_PI / 180.0;
	}
	
	template <class T>
	static inline T radToDeg(const T &x) {
		return 180.0 * x / M_PI; 
	}

	template <class T>
	static inline T isPowerOfTwo(const T &x) {
		return x && !(x & (x - 1));
	}
	
} //namespace sxge

#endif //__SXGE_MATH_UTILS_HH__
