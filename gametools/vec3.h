#ifndef _VEC3_H
#define _VEC3_H

/**
 * copyright 2004 Jean-Christophe Hoelt
 *
 * This program is distributed under the terms of the
 * GNU General Public Licence
 */

#include <math.h>
#include <iostream>
using namespace std;

// namespace pineapple3 {

class Vec3 {
public:
	float x, y, z;
	Vec3(float x=0, float y=0, float z=0) : x(x),y(y),z(z) { }
    Vec3(const Vec3&v) : x(v.x),y(v.y),z(v.z) { }

    const Vec3& operator=(const Vec3&v) {
        x=v.x;
        y=v.y;
        z=v.z;
        return *this;
    }

    bool operator==(const Vec3&v) const {
        return (x==v.x)&&(y==v.y)&&(z==v.z);
    }
    
    bool operator!=(const Vec3&v) const {
        return (x!=v.x)||(y!=v.y)||(z!=v.z);
    }
    
    void setXYZ(float x, float y, float z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }

	bool is_zero() const {
		return (fabs(x) < 0.000001f)
			&& (fabs(y) < 0.000001f)
			&& (fabs(z) < 0.000001f);
	}
	
	const Vec3& operator*=(const float f) {
		x*=f;
		y*=f;
		z*=f;
		return *this;
	}
	
	const Vec3& operator/=(const float f) {
        if (fabs(f) > 0.001f) {
    		x/=f;
	    	y/=f;
		    z/=f;
        }
		return *this;
	}
    
	Vec3 operator + (const Vec3&v) const{
        Vec3 vr = *this;
        vr.x+=v.x;
        vr.y+=v.y;
        vr.z+=v.z;
		return vr;
	}
    
	Vec3 operator - (const Vec3&v) const{
        Vec3 vr = *this;
        vr.x-=v.x;
        vr.y-=v.y;
        vr.z-=v.z;
		return vr;
	}
	Vec3 operator - () const{
        return Vec3 (-x,-y,-z);
	}
    
	const Vec3& operator += (const Vec3&v) {
		if (!v.is_zero()) {
			x+=v.x;
			y+=v.y;
			z+=v.z;
		}
		return *this;
	}

	const Vec3& operator -= (const Vec3&v) {
        if (!v.is_zero()) {
            x-=v.x;
            y-=v.y;
            z-=v.z;
        }
		return *this;
	}
    
    Vec3 operator*(float f) const {
        Vec3 v = *this;
        v.x*=f;
        v.y*=f;
        v.z*=f;
        return v;
    }

    Vec3 operator/(float f) const {
        Vec3 v = *this;
        v.x/=f;
        v.y/=f;
        v.z/=f;
        return v;
    }

    float distLowerThan(const Vec3&v, float d) const {
        return (*this - v).sqLength() < d*d;
    }
    
    float dist(const Vec3&v) const {
        return (*this - v).length();
    }

	float sqDist(const Vec3&v) const {
		return (*this - v).sqLength();
	}
    
    const Vec3 normalized() const {
        return (*this) / length();
    }

    void fillfv(float fv[3]) const {
        fv[0] = x;
        fv[1] = y;
        fv[2] = z;
    }

    float sqLength() const {
        return (x*x+y*y+z*z);
    }
    
    float length() const {
        return sqrt(x*x+y*y+z*z);
    }

    Vec3 product(const Vec3& v) const {
        return Vec3(y*v.z-v.y*z, z*v.x-v.z*x, x*v.y-v.x*y);
    }

    void display() const {
        printf("(%3.1f,%3.1f,%3.1f)\n", x, y, z);
    }
};

class Vec2 : public Vec3
{
  public:
    Vec2(float x = 0.0f, float y = 0.0f) : Vec3(x,y,0.0f) {}
    Vec2(const Vec3 &v3) : Vec3(v3.x,v3.y,0.0f) {}

    void set(float x, float y) {
      this->x = x;
      this->y = y;
    }

    float getX() const { return x; }
    float getY() const { return y; }

    float getDirection() {
      Vec2 v = normalized();
      // calculate the angle from a unit vector
      if( v.x<0 ) return (float)3.1415f - (float)asin(v.y);
      return (float)asin(v.y);
    }
};

static inline Vec2 average(const Vec2 &v1, const Vec2 &v2) {
  return Vec2((v1.x + v2.x) * 0.5f, (v1.y + v2.y) * 0.5f);
}

static const Vec3 X_AXIS(1.0f,0.0f,0.0f);
static const Vec3 Y_AXIS(0.0f,1.0f,0.0f);
static const Vec3 Z_AXIS(0.0f,0.0f,1.0f);

static inline float dot_product(const Vec3 &v1, const Vec3 &v2) {
    return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}

static inline Vec3 vec_product(const Vec3 &v1, const Vec3& v2) {
    return Vec3(v1.y*v2.z-v2.y*v1.z, v1.z*v2.x-v2.z*v1.x, v1.x*v2.y-v2.x*v1.y);
}

// };

#endif
