#ifndef MATRIX_H
#define MATRIX_H

#include "Vector.h"

/*
 * Simple 4x4 Matrix implementation.
 *
 * Note that, internally, we use row-major ordering
 * to simplify matrix-vector products.
 */
class Matrix {

    // Matrix rows
    Vector a, b, c, d;

    public:

    /*
    * Constructor. Initializes a matrix to the identity.
    */
    Matrix();

    /*
    * Sets from a column-major array.
    */
    void Set(const GLfloat* matrix);

    /*
     * Sets from an aiMatrix4x4.
     */
    void Set(aiMatrix4x4 mat);

    /*
    * Gets to a column-major array.
    */
    void Get(GLfloat* matrix);

    /*
     * Gets the upper 3x3 matrix in a column-major array.
     */
    void Get3x3(GLfloat* matrix);

    /*
     * Gets a row of the matrix. Zero-indexed.
     */
    Vector GetRow(unsigned row);

    /*
     * Sets a row of the matrix. Zero-indexed.
     */
    void SetRow(unsigned row, Vector v);

    /*
     * Loads the identity.
     */
    void LoadIdentity();

    /*
     * Postmultiply us by a rotation matrix.
     */
    void Rotate(float angle, float x, float y, float z);

    /*
     * Postmultiply us by a translation matrix.
     */
    void Translate(float x, float y, float z);

    /*
     * Postmultiply by an orthographic projection matrix.
     */
    void Ortho(float left, float right,
               float bottom, float top,
               float near, float far);

    /*
     * Postmultiply by a view matrix equivalent to the one
     * obtained with gluLookAt().
     */
    void LookAt(Vector& eye, Vector& center, Vector& up);

    /*
    * Matrix-Vector product.
    */
    Vector MVProduct(Vector& vec);

    /*
    * Generates the composition of this matrix
    * with another (returns this * mat)
    */
    Matrix MMProduct(Matrix& mat);

    /*
     * Sets us to our transpose.
     */
    Matrix Transpose();

    /*
     * Determinant of the upper-right 3x3 matrix.
     */
    GLfloat Determinant3();

    /*
    * 3x3 Inverse. Returns a matrix whose top 3x3 entries
    * are the inverse of this matrix's top 3x3 entries, and
    * whose other entries are set to zero.
    */
    Matrix Inverse();

    /*
    * For debugging, dumps a matrix to stdout.
    */
    void Dump();
};

#endif /* MATRIX_H */
