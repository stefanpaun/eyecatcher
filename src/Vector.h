// Vector.h

#ifndef _VECTOR_h
#define _VECTOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


struct Vector {
	float x;
	float y;

	void add(Vector a) {
		x += a.x;
		y += a.y;
	}

	void sub(Vector a) {
		x -= a.x;
		y -= a.y;
	}

	void normalize() {
		if (abs(x) > abs(y)) {
			y = y / abs(x);
			x = x > 0 ? 1 : -1;
		}
		else {
			x = x / abs(y);
			y = y > 0 ? 1 : -1;
		}
	}

	float mag() {
		return sqrt(sq(x) + sq(y));
	}

	Vector operator * (float f) {
		return Vector{ (x * f), (y * f) };
	};

};



#endif

