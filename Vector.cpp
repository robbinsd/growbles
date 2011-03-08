#include "Vector.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

Vector::Vector() : x(0.0)
                 , y(0.0)
                 , z(0.0)
                 , w(1.0)
{
}

Vector::Vector(float xx, float yy, float zz, float ww) : x(xx)
                                                       , y(yy)
                                                       , z(zz)
                                                       , w(ww)
{
}

Vector::Vector(btVector3 vec) : x(vec.x())
                              , y(vec.y())
                              , z(vec.z())
                              , w(1.0)
{
}

void
Vector::Set(float x, float y, float z, float w)
{
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
}

void
Vector::Set(aiVector3D& vec)
{
    x = vec.x;
    y = vec.y;
    z = vec.z;
    w = 1.0;
}

Vector
Vector::Unit() const
{
    // Scale appropriately
    Vector rv = *this;
    return rv.Scale(1.0 / Norm3());
}

float
Vector::Norm3() const
{
    return sqrt(x * x + y * y + z * z);
}

bool
Vector::Equals3(Vector& other) const
{
    Vector difference = *this - other;
    float diffNorm = difference.Norm3();

    return diffNorm < VEC_EPS;
}

float
Vector::Dot(const Vector& other) const
{
    return  x * other.x +
            y * other.y +
            z * other.z +
            w * other.w;
}

float
Vector::Dot3(const Vector& other) const
{
    return  x * other.x +
            y * other.y +
            z * other.z;
}

Vector
Vector::OrthoA3() const
{
    Vector zero;
    assert(!Equals3(zero));

    // Find a linearly independent vector
    Vector crossIn(1.0, 0.0, 0.0, 0.0);
    if (Unit().Equals3(crossIn))
        crossIn.y = 1.0;

    return Cross(crossIn).Unit();
}

Vector
Vector::OrthoB3() const
{
    return Cross(OrthoA3()).Unit();
}

bool
Vector::OrthogonalTo3(const Vector& other) const
{
    return fabs(Dot3(other)) < VEC_EPS;
}

Vector
Vector::Cross(const Vector& other) const
{
  // Setup
  Vector rv;
  rv.w = 0.0f;

  // Compute the cross product
  rv.x = y * other.z - z * other.y;
  rv.y = -(x * other.z - z * other.x);
  rv.z = x * other.y - y * other.x;

  return rv;
}

Vector
Vector::Scale(float factor)
{
    Vector rv;
    rv.x = x * factor;
    rv.y = y * factor;
    rv.z = z * factor;
    rv.w = w * factor;
    return rv;
}

const Vector
Vector::operator+(const Vector& other) const
{
    Vector rv;
    rv.x = x + other.x;
    rv.y = y + other.y;
    rv.z = z + other.z;
    rv.w = w + other.w;
    return rv;
}

const Vector
Vector::operator-(const Vector& other) const
{
    Vector rv;
    rv.x = x - other.x;
    rv.y = y - other.y;
    rv.z = z - other.z;
    rv.w = w - other.w;
    return rv;
}
//
// Generates a random float in the range [0, 1]
float
Vector::RandomFloat()
{
#ifdef _WIN32
    return (float) ((double) rand()) / (double) RAND_MAX;
#else
    return (float) ((double) random()) / (double) RAND_MAX;
#endif
}

// Generates a random float in the range [-mag, mag]
float
Vector::RandomFloat(float mag)
{
    return (RandomFloat() * 2.0 - 1.0) * mag;
}

Vector
Vector::Random(float xMag, float yMag, float zMag)
{
    Vector rv;
    rv.x = RandomFloat(xMag);
    rv.y = RandomFloat(yMag);
    rv.z = RandomFloat(zMag);

    return rv;
}


const void
Vector::Dump()
{
    printf("(%f, %f, %f, %f)\n", x, y, z, w);
}
