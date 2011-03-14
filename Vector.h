#ifndef VECTOR_H
#define VECTOR_H

#include "Framework.h"

#define VEC_EPS 0.0001f

/*
 * Simple 4-dimensional vector implementation.
 */
struct Vector {

    float x, y, z, w;
    
    /*
    * Access a mutable reference to one of the Vector's elements
    */
    float &operator[](int i){   assert(i>=0 && i < 4); return (&x)[i]; }
    /*
    * Access a const reference to one of the Vector's elements
    */
    const float &operator[](int i) const{   assert(i>=0 && i < 4); return (&x)[i]; }

    /*
    * Constructor.
    */
    Vector();
    Vector(float xx, float yy, float zz, float ww);
    Vector(const btVector3 &vec);

    /*
    * Batch setter.
    */
    void Set(float x, float y, float z, float w);
    void Set(aiVector3D& vec);

    /*
     * Gets our contents as a GLfloat.
     */
    GLfloat* Get() { return (GLfloat*)this; }

    /*
     * Gets a unit vector in the same direction as this vector.
     */
    Vector Unit() const;

    /*
     * Returns the 3D norm of this vector.
     */
    float Norm3() const;

    /*
     * 3D Vector equality.
     */
    bool Equals3(Vector& other) const;

    /*
    * Standard dot product routine.
    */
    float Dot(const Vector& other) const;

    /*
     * 3x3 dot routine. Ignore the w coordinate.
     */
    float Dot3(const Vector& other) const;

    /*
     * Returns an unit vector orthogonal to this one.
     */
    Vector OrthoA3() const;

    /*
     * Returns a unit vector orthogonal to this one as
     * well as the result of OrthoA3().
     */
    Vector OrthoB3() const;

    /*
     * Determines if the two 3D vectors are orthogonal.
     */
    bool OrthogonalTo3(const Vector& other) const;

    /*
     * Cross product, ignoring the w component.
     * The w component of the result is set to 0.
     */
    Vector Cross(const Vector& other) const;

    /*
    * Scale.
    */
    Vector Scale(GLfloat factor);

    /*
    * Addition.
    */
    const Vector operator+(const Vector& other) const;

    /*
    * Subtraction.
    */
    const Vector operator-(const Vector& other) const;

    /*
     * Produces a random float in the range [0,1]
     */
    static float RandomFloat();

    /*
     * Produces a random float in the range [-mag, mag]
     */
    static float RandomFloat(float mag);

    /*
     * Static method to generate a random vector.
     *
     * Each component will be uniformly distributed in the range
     * [-*Mag, *Mag]
     */
    static Vector Random(float xMag, float yMag, float zMag);

    /*
     * Debugging tool. Dumps to stdout.
     */
    const void Dump();
};

#endif /* VECTOR_H */
