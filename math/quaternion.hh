#ifndef __SXGE_MATH_QUATERNION_HH__
#define __SXGE_MATH_QUATERNION_HH__

#include "vmath.hh"

namespace vmath {

template <typename T>
class Quaternion {
public:
	Quaternion<T>() : x(0), y(0), z(0), w(1) {}
	Quaternion<T>(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

	Quaternion<T>(const vmath::vec3<T> &vec, const T angle) {
		T nangle = angle / 2.0;
		T s = std::sin(nangle);
		T vmag = vec.mag();

		x = vec.X() * s / vmag;
		y = vec.Y() * s / vmag;
		z = vec.Z() * s / vmag;
		w = std::cos(nangle);
	}

	Quaternion<T> (const Quaternion<T> &q) : x(q.x), y(q.y), z(q.x), w(q.w) {}

	Quaternion<T> normalized(void) {
		Quaternion<T> ret;
		return ret;
	}

	Quaternion<T> conjugated(void) {
		return Quaternion<T>(-x, -y, -z, w);
	}

	T mag(void) {
		return sqrt(x * x + y * y + z * z + w * w);
	}

	Quaternion<T> operator+(const Quaternion<T>& q) {
		return Quaternion<T>(
			x + q.x,
			y + q.y,
			z + q.z,
			w + w.w);
	}

	Quaternion<T> operator-(const Quaternion<T>& q) {
		return Quaternion<T>(
			x - q.x,
			y - q.y,
			z - q.z,
			w - w.w);
	}
protected:
	T x;
	T y;
	T z;
	T w;
};

} //namespace vmath

#endif //__SXGE_MATH_QUATERNION_HH__
