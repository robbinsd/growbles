#include "Matrix.h"
#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <assert.h>

Matrix::Matrix()
{
    LoadIdentity();
}

Matrix::Matrix(const btMatrix3x3 &mat){
    LoadIdentity();
    for(int row = 0; row < 3; row ++){
        for(int col = 0; col < 3; col ++){
            (*this)[row][col] = mat[row][col];
        }
    }
}

void
Matrix::LoadIdentity()
{
    a.Set(1.0, 0.0, 0.0, 0.0);
    b.Set(0.0, 1.0, 0.0, 0.0);
    c.Set(0.0, 0.0, 1.0, 0.0);
    d.Set(0.0, 0.0, 0.0, 1.0);

}

void
Matrix::Set(const GLfloat* matrix)
{
    // We work with the transpose to use our vector constructors
    Matrix matPrime;
    matPrime.a.Set(matrix[0], matrix[1], matrix[2], matrix[3]);
    matPrime.b.Set(matrix[4], matrix[5], matrix[6], matrix[7]);
    matPrime.c.Set(matrix[8], matrix[9], matrix[10], matrix[11]);
    matPrime.d.Set(matrix[12], matrix[13], matrix[14], matrix[15]);

    *this = matPrime.Transpose();
}
void
Matrix::Set(aiMatrix4x4 mat)
{
    // aiMatrices are row-major
    mat.Transpose();
    Set((GLfloat *)&mat);
}

void
Matrix::Get(GLfloat* matrix)
{
    matrix[0] = a.x;
    matrix[1] = b.x;
    matrix[2] = c.x;
    matrix[3] = d.x;

    matrix[4] = a.y;
    matrix[5] = b.y;
    matrix[6] = c.y;
    matrix[7] = d.y;

    matrix[8] = a.z;
    matrix[9] = b.z;
    matrix[10] = c.z;
    matrix[11] = d.z;

    matrix[12] = a.w;
    matrix[13] = b.w;
    matrix[14] = c.w;
    matrix[15] = d.w;
}

void
Matrix::Get3x3(GLfloat* matrix)
{
    matrix[0] = a.x;
    matrix[1] = b.x;
    matrix[2] = c.x;

    matrix[3] = a.y;
    matrix[4] = b.y;
    matrix[5] = c.y;

    matrix[6] = a.z;
    matrix[7] = b.z;
    matrix[8] = c.z;
}

Vector
Matrix::GetRow(unsigned row)
{
    assert(row < 4);
    if (row == 0)
        return a;
    else if (row == 1)
        return b;
    else if (row == 2)
        return c;
    return d;
}

void
Matrix::SetRow(unsigned row, Vector v)
{
    assert(row < 4);
    if (row == 0)
        a = v;
    else if (row == 1)
        b = v;
    else if (row == 2)
        c = v;
    else
        d = v;
}

void
Matrix::Rotate(float angle, float x, float y, float z)
{
    Matrix newMat;
    angle *= M_PI / 180.0f;

    GLfloat l = sqrt(x * x + y * y + z * z);
    assert(l != 0.0f);
    x /= l;
    y /= l;
    z /= l;
    GLfloat c = cos(angle);
    GLfloat s = sin(angle);

    newMat.a.x = x * x * (1.0f - c) + c;
    newMat.b.x = y * x * (1.0f - c) + z * s;
    newMat.c.x = x * z * (1.0f - c) - y * s;
    newMat.a.y = x * y * (1.0f - c) - z * s;
    newMat.b.y = y * y * (1.0f - c) + c;
    newMat.c.y = y * z * (1.0f - c) + x * s;
    newMat.a.z = x * z * (1.0f - c) + y * s;
    newMat.b.z = y * z * (1.0f - c) - x * s;
    newMat.c.z = z * z * (1.0f - c) + c;

    *this = MMProduct(newMat);
}

void
Matrix::Translate(float x, float y, float z)
{
    Matrix newMat;
    newMat.a.w = x;
    newMat.b.w = y;
    newMat.c.w = z;

    *this = MMProduct(newMat);
}

#ifdef _WIN32
#undef far
#undef near
#endif

void
Matrix::Ortho(float left, float right,
              float bottom, float top,
              float near, float far)
{
    // Generate the ortho matrix
    Matrix ortho;
    ortho.a.x = 2.0f / (right - left);
    ortho.b.y = 2.0f / (top - bottom);
    ortho.c.z = -2.0f / (far - near);
    ortho.a.w = -(right + left) / (right - left);
    ortho.b.w = -(top + bottom) / (top - bottom);
    ortho.c.w = -(far + near) / (far - near);

    // Apply the matrix
    *this = MMProduct(ortho);
}

void
Matrix::LookAt(Vector& eye, Vector& center, Vector& up)
{
    // Adapted from:
    // http://pyopengl.sourceforge.net/documentation/manual/gluLookAt.3G.html

    // Generate f
    Vector F = center - eye;
    F.w = 0.0f;
    Vector f = F.Unit();

    // Generate s
    Vector upUnit = up.Unit();
    Vector s = f.Cross(upUnit);

    // Generate u
    Vector u = s.Cross(f);

    // Make sure the last components are zero
    f.w = s.w = u.w = 0.0f;

    // Put them in a matrix
    Matrix lookAt;
    lookAt.a = s;
    lookAt.b = u;
    lookAt.c = f.Scale(-1.0);

    // Translate to the eye position
    lookAt.Translate(-eye.x, -eye.y, -eye.z);

    // Apply the matrix
    *this = MMProduct(lookAt);
}

Vector
Matrix::MVProduct(Vector& vec)
{
    Vector rv;
    rv.x = a.Dot(vec);
    rv.y = b.Dot(vec);
    rv.z = c.Dot(vec);
    rv.w = d.Dot(vec);
    return rv;
}

Matrix
Matrix::MMProduct(Matrix& mat)
{
    // We work with transposes to use our dot product routines
    Matrix rvPrime;
    Matrix matPrime = mat.Transpose();

    rvPrime.a = this->MVProduct(matPrime.a);
    rvPrime.b = this->MVProduct(matPrime.b);
    rvPrime.c = this->MVProduct(matPrime.c);
    rvPrime.d = this->MVProduct(matPrime.d);

    return rvPrime.Transpose();
}

Matrix
Matrix::Transpose()
{
    Matrix rv;
    rv.a.Set(a.x, b.x, c.x, d.x);
    rv.b.Set(a.y, b.y, c.y, d.y);
    rv.c.Set(a.z, b.z, c.z, d.z);
    rv.d.Set(a.w, b.w, c.w, d.w);
    return rv;
}

void
Matrix::Dump()
{
    printf("(%f %f %f %f)\n", a.x, a.y, a.z, a.w);
    printf("(%f %f %f %f)\n", b.x, b.y, b.z, b.w);
    printf("(%f %f %f %f)\n", c.x, c.y, c.z, c.w);
    printf("(%f %f %f %f)\n", d.x, d.y, d.z, d.w);
}

GLfloat
Matrix::Determinant3()
{
  return a.x * (b.y * c.z - b.z * c.y) -
         a.y * (b.x * c.z - b.z * c.x) +
         a.z * (b.x * c.y - b.y * c.x);
}

Matrix
Matrix::Inverse()
{
    // Set up our new matrix, making sure that the bottom row
    // and outer column are zero.
    Matrix rv;
    rv.d.w = 0.0f;

    // Compute the inverse determinant of the whole matrix
    GLfloat idet = 1.0 / Determinant3();

    // The inverse of a matrix can be computed in terms of
    // cross products. For more details, see:
    // http://en.wikipedia.org/wiki/Invertible_matrix
    Matrix trans = Transpose();
    rv.a = trans.b.Cross(trans.c).Scale(idet);
    rv.b = trans.c.Cross(trans.a).Scale(idet);
    rv.c = trans.a.Cross(trans.b).Scale(idet);

    // All done!
    return rv;
}
