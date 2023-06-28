#pragma once

#include <float.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <initializer_list>
#include <iostream>
/***************************
 * Log
 ****************************/

#define Log(fmt, ...)                                                 \
    printf("%s[%s: %d]: " fmt "\n", __FILE__, __FUNCTION__, __LINE__, \
           ##__VA_ARGS__)

/***************************
 * Math
 ****************************/

using real = float;
constexpr real RealInf = FLT_MAX;

// 定义向量及其相关操作

template <size_t Dim>
class Vector {
   public:
    static const Vector<Dim> One;
    static const Vector<Dim> Zero;

    real data[Dim];

    Vector() = default;

    Vector(real value) {
        for (size_t i = 0; i < Dim; i++) {
            data[i] = value;
        }
    }

    Vector(const std::initializer_list<real> &l) { Assign(l); }

    real &operator[](size_t idx) { return data[idx]; }
    real operator[](size_t idx) const { return data[idx]; }

    void Assign(const std::initializer_list<real> &l) {
        size_t max = std::min(Dim, l.size());
        size_t i = 0;
        for (i = 0; i < max; i++) {
            data[i] = *(l.begin() + i);
        }
        for (; i < Dim; i++) {
            data[i] = 0;
        }
    }
};

template <size_t Dim>
const Vector<Dim> Vector<Dim>::One(1);

template <size_t Dim>
const Vector<Dim> Vector<Dim>::Zero(0);

using Vec2 = Vector<2>;
using Vec3 = Vector<3>;
using Vec4 = Vector<4>;
using Color4 = Vec4;
using Color3 = Vec3;

template <>
class Vector<2> {
   public:
    static const Vector One;
    static const Vector Zero;

    union {
        struct {
            real x, y;
        };
        struct {
            real w, h;
        };
        real data[2];
    };

    Vector() = default;

    Vector(const std::initializer_list<real> &l) { Assign(l); }

    real &operator[](size_t idx) { return data[idx]; }
    real operator[](size_t idx) const { return data[idx]; }

    void Assign(const std::initializer_list<real> &l) {
        size_t max = std::min<size_t>(2, l.size());
        size_t i = 0;
        for (i = 0; i < max; i++) {
            data[i] = *(l.begin() + i);
        }
        for (; i < 2; i++) {
            data[i] = 0;
        }
    }
};

const Vector<2> Vector<2>::One{1, 1};
const Vector<2> Vector<2>::Zero{0, 0};

inline real Cross(const Vector<2> &v1, const Vector<2> &v2) {
    return v1.x * v2.y - v1.y * v2.x;
}

template <>
class Vector<3> {
   public:
    static const Vector One;
    static const Vector Zero;
    static const Vector X_Axis;
    static const Vector Y_Axis;
    static const Vector Z_Axis;

    union {
        struct {
            real x, y, z;
        };
        struct {
            real r, g, b;
        };
        struct {
            real u, v, w;
        };
        struct {
            real alpha, beta, gamma;
        };
        real data[3];
    };

    Vector() = default;

    Vector(const std::initializer_list<real> &l) { Assign(l); }

    real &operator[](size_t idx) { return data[idx]; }
    real operator[](size_t idx) const { return data[idx]; }

    void Assign(const std::initializer_list<real> &l) {
        size_t max = std::min<size_t>(3, l.size());
        size_t i = 0;
        for (; i < max; i++) {
            data[i] = *(l.begin() + i);
        }
        for (; i < 3; i++) {
            data[i] = 0;
        }
    }
};

const Vector<3> Vector<3>::One{1, 1, 1};
const Vector<3> Vector<3>::Zero{0, 0, 0};
const Vector<3> Vector<3>::X_Axis{1, 0, 0};
const Vector<3> Vector<3>::Y_Axis{0, 1, 0};
const Vector<3> Vector<3>::Z_Axis{0, 0, 1};

inline Vector<3> Cross(const Vector<3> &v1, const Vector<3> &v2) {
    return Vector<3>{v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z,
                     v1.x * v2.y - v1.y * v2.x};
}

template <>
class Vector<4> {
   public:
    static const Vector One;
    static const Vector Zero;

    union {
        struct {
            real x, y, z, w;
        };
        struct {
            real r, g, b, a;
        };
        real data[4];
    };

    Vector() = default;

    Vector(const std::initializer_list<real> &l) { Assign(l); }

    real &operator[](size_t idx) { return data[idx]; }
    real operator[](size_t idx) const { return data[idx]; }

    void Assign(const std::initializer_list<real> &l) {
        size_t max = std::min<size_t>(4, l.size());
        size_t i = 0;
        for (; i < max; i++) {
            data[i] = *(l.begin() + i);
        }
        for (; i < 4; i++) {
            data[i] = 0;
        }
    }

    static Vec4 FromVec3(Vec3 &vec, real w) {
        return Vec4{vec.x, vec.y, vec.z, w};
    }

    Vec3 TruncatedToVec3() { return Vec3{x, y, z}; }

    Vec2 TruncatedToVec2() { return Vec2{x, y}; }
};

const Vector<4> Vector<4>::One{1, 1, 1, 1};
const Vector<4> Vector<4>::Zero{0, 0, 0, 0};

template <size_t Dim>
Vector<Dim> operator*(const Vector<Dim> &self, real value) {
    Vector<Dim> v;
    for (size_t i = 0; i < Dim; i++) {
        v.data[i] = self.data[i] * value;
    }
    return v;
}

template <size_t Dim>
Vector<Dim> operator/(const Vector<Dim> &self, real value) {
    Vector<Dim> v;
    for (size_t i = 0; i < Dim; i++) {
        v.data[i] = self.data[i] / value;
    }
    return v;
}

template <size_t Dim>
Vector<Dim> operator*(const Vector<Dim> &self, const Vector<Dim> &o) {
    Vector<Dim> v;
    for (size_t i = 0; i < Dim; i++) {
        v.data[i] = self.data[i] * o.data[i];
    }
    return v;
}

template <size_t Dim>
Vector<Dim> operator/(const Vector<Dim> &self, const Vector<Dim> &o) {
    Vector<Dim> v;
    for (size_t i = 0; i < Dim; i++) {
        v.data[i] = self.data[i] / o.data[i];
    }
    return v;
}

template <size_t Dim>
Vector<Dim> operator+(const Vector<Dim> &self, const Vector<Dim> &o) {
    Vector<Dim> v;
    for (size_t i = 0; i < Dim; i++) {
        v.data[i] = self.data[i] + o.data[i];
    }
    return v;
}

template <size_t Dim>
Vector<Dim> operator-(const Vector<Dim> &self, const Vector<Dim> &o) {
    Vector<Dim> v;
    for (size_t i = 0; i < Dim; i++) {
        v.data[i] = self.data[i] - o.data[i];
    }
    return v;
}

// 负号
template <size_t Dim>
Vector<Dim> operator-(const Vector<Dim> &self) {
    Vector<Dim> v;
    for (size_t i = 0; i < Dim; i++) {
        v.data[i] = -self.data[i];
    }
    return v;
}

template <size_t Dim>
Vector<Dim> &operator*=(Vector<Dim> &self, real value) {
    for (size_t i = 0; i < Dim; i++) {
        self.data[i] *= value;
    }
    return self;
}

template <size_t Dim>
Vector<Dim> &operator/=(const Vector<Dim> &self, real value) {
    for (size_t i = 0; i < Dim; i++) {
        self.data[i] /= value;
    }
    return self;
}

template <size_t Dim>
Vector<Dim> &operator*=(Vector<Dim> &self, const Vector<Dim> &o) {
    for (size_t i = 0; i < Dim; i++) {
        self.data[i] *= o.data[i];
    }
    return self;
}

template <size_t Dim>
Vector<Dim> &operator/=(const Vector<Dim> &self, const Vector<Dim> &o) {
    for (size_t i = 0; i < Dim; i++) {
        self.data[i] /= o.data[i];
    }
    return self;
}

template <size_t Dim>
Vector<Dim> &operator+=(Vector<Dim> &self, const Vector<Dim> &o) {
    for (size_t i = 0; i < Dim; i++) {
        self.data[i] += o.data[i];
    }
    return self;
}

template <size_t Dim>
Vector<Dim> &operator-=(Vector<Dim> &self, const Vector<Dim> &o) {
    for (size_t i = 0; i < Dim; i++) {
        self.data[i] -= o.data[i];
    }
    return self;
}

template <size_t Dim>
real Dot(const Vector<Dim> &v1, const Vector<Dim> &v2) {
    real sum = 0;
    for (size_t i = 0; i < Dim; i++) {
        sum += v1.data[i] * v2.data[i];
    }
    return sum;
}

template <size_t Dim>
bool operator==(const Vector<Dim> &v1, const Vector<Dim> &v2) {
    for (size_t i = 0; i < Dim; i++) {
        if (v1[i] != v2[i]) {
            return false;
        }
    }
    return true;
}

template <size_t Dim>
bool operator!=(const Vector<Dim> &v1, const Vector<Dim> &v2) {
    return !(v1 == v2);
}

template <size_t Dim1, size_t Dim2>
Vector<Dim1> Vec(const Vector<Dim2> &v) {
    Vector<Dim1> result;
    size_t min = std::min(Dim1, Dim2);
    size_t i = 0;
    for (i = 0; i < min; i++) {
        result.data[i] = v.data[i];
    }
    for (; i < Dim1; i++) {
        result.data[i] = 0;
    }
    return result;
}

// 向量的长度及其平方
template <size_t Dim>
real Len2(const Vector<Dim> &v) {
    real result = 0;
    for (auto &elem : v.data) {
        result += elem * elem;
    }
    return result;
}

template <size_t Dim>
real Len(const Vector<Dim> &v) {
    return std::sqrt(Len2(v));
}

template <size_t Dim>
Vector<Dim> operator*(real value, const Vector<Dim> &v) {
    return v * value;
}

template <size_t Dim>
Vector<Dim> operator/(real value, const Vector<Dim> &v) {
    return v / value;
}

template <size_t Dim>
Vector<Dim> Normalize(const Vector<Dim> &v) {
    Vector<Dim> result;
    float len = Len(v);
    for (size_t i = 0; i < Dim; i++) {
        result.data[i] = v.data[i] / len;
    }
    return result;
}

template <size_t Dim>
std::ostream &operator<<(std::ostream &o, const Vector<Dim> &v) {
    o << "Vector<" << Dim << ">(";
    for (size_t i = 0; i < Dim; i++) {
        o << v.data[i];
        if (i != Dim - 1) {
            o << ", ";
        }
    }
    o << ")";
    return o;
}

// matrix
// opengl shader matrix is col-major, so our matrix is the same as it
template <size_t Col, size_t Row>
class Matrix {
   public:
    static Matrix Zero;
    static Matrix One;

    Matrix() = default;

    Matrix(real value) {
        for (auto &elem : data_) {
            elem = value;
        }
    }

    Matrix(const std::initializer_list<real> &l) {
        for (size_t i = 0; i < l.size(); i++) {
            Set(i % Col, i / Col, *(l.begin() + i));
        }
    }

    Matrix(const std::initializer_list<Vector<Row>> &l) {
        for (size_t i = 0; i < l.size(); i++) {
            SetRow(i, *(l.begin() + i));
        }
    }

    // 设置一行
    inline void SetRow(size_t row, const Vector<Col> &a) {
        // assert(row < Row);
        for (size_t i = 0; i < Col; i++) data_[row * Col + i] = a[i];
    }

    // 设置一列
    inline void SetCol(size_t col, const Vector<Row> &a) {
        // assert(col < Col);
        for (size_t i = 0; i < Row; i++) data_[i * Col + col] = a[i];
    }

    real Get(size_t x, size_t y) const { return data_[y + x * Row]; }
    real &Get(size_t x, size_t y) { return data_[y + x * Row]; }

    void Set(size_t x, size_t y, real value) { data_[y + x * Row] = value; }

    Matrix operator*(real value) const {
        Matrix result = *this;
        for (auto &elem : result.data_) {
            elem *= value;
        }
        return result;
    }

    Matrix operator/(real value) const {
        Matrix result = *this;
        for (auto &elem : result.data_) {
            elem /= value;
        }
        return result;
    }

    Matrix operator+(const Matrix &m) const {
        Matrix result = *this;
        for (size_t i = 0; i < Col * Row; i++) {
            result.data_[i] += m.data_[i];
        }
        return result;
    }

    Matrix operator-(const Matrix &m) const {
        Matrix result = *this;
        for (size_t i = 0; i < Col * Row; i++) {
            result.data_[i] -= m.data_[i];
        }
        return result;
    }

    Matrix &operator*=(real value) {
        for (auto &elem : data_) {
            elem *= value;
        }
        return *this;
    }

    Matrix &operator/=(real value) {
        for (auto &elem : data_) {
            elem /= value;
        }
        return *this;
    }

    Matrix &operator+=(const Matrix &m) {
        for (size_t i = 0; i < Col * Row; i++) {
            data_[i] += m.data_[i];
        }
        return *this;
    }

    Matrix &operator-=(const Matrix &m) {
        for (size_t i = 0; i < Col * Row; i++) {
            data_[i] -= m.data_[i];
        }
        return *this;
    }

    Matrix &operator*=(const Matrix &m) {
        static_assert(Col == Row);

        Matrix tmp;
        for (size_t k = 0; k < Col; k++) {
            for (size_t j = 0; j < Col; j++) {
                real sum = 0;
                for (size_t i = 0; i < Col; i++) {
                    sum += Get(i, k) * m.Get(j, i);
                }
                tmp.Set(j, k, sum);
            }
        }

        *this = tmp;
        return *this;
    }

    static Matrix Ones() { return Matrix(1); }

    static Matrix Zeros() { return Matrix(0); }

    // 获取单位矩阵
    static Matrix Eye() {
        static_assert(Col == Row);

        Matrix<Col, Col> result(0);
        for (size_t i = 0, j = 0; i < Col; i++, j++) {
            result.Set(i, j, 1);
        }
        return result;
    }

    void T() {
        for (size_t i = 0; i < Row; i++) {
            for (size_t j = i + 1; j < Col; j++) {
                std::swap(Get(i, j), Get(j, i));
            }
        }
    }

   private:
    real data_[Col * Row];
};

template <size_t Col, size_t Row>
std::ostream &operator<<(std::ostream &o, const Matrix<Col, Row> &m) {
    o << "Matrix[" << std::endl;
    for (size_t y = 0; y < Row; y++) {
        for (size_t x = 0; x < Col; x++) {
            if (x == Col - 1) {
                o << m.Get(x, y);
            } else {
                o << m.Get(x, y) << ", ";
            }
        }
        o << std::endl;
    }
    o << "]";
    return o;
}

template <size_t Col, size_t Row, size_t TCol, size_t TRow>
Matrix<TCol, Row> operator*(const Matrix<Col, Row> &m1,
                            const Matrix<TCol, TRow> &m2) {
    static_assert(Col == TRow);

    Matrix<TCol, Row> result;
    for (size_t k = 0; k < Row; k++) {
        for (size_t j = 0; j < TCol; j++) {
            real sum = 0;
            for (size_t i = 0; i < Col; i++) {
                sum += m1.Get(i, k) * m2.Get(j, i);
            }
            result.Set(j, k, sum);
        }
    }
    return result;
}

template <size_t Col, size_t Row>
Matrix<Row, Col> Transpose(const Matrix<Col, Row> &m) {
    Matrix<Row, Col> result;
    for (size_t i = 0; i < Row; i++) {
        for (size_t j = 0; j < Col; j++) {
            result.Set(i, j, m.Get(j, i));
        }
    }
    return result;
}

template <size_t Col, size_t Row, size_t Dim>
Vector<Row> operator*(const Matrix<Col, Row> &m, const Vector<Dim> &v) {
    Vector<Row> result;
    for (size_t j = 0; j < Row; j++) {
        real sum = 0;
        for (size_t i = 0; i < Col; i++) {
            sum += v.data[i] * m.Get(i, j);
        }
        result.data[j] = sum;
    }
    return result;
}

using Mat22 = Matrix<2, 2>;
using Mat33 = Matrix<3, 3>;
using Mat44 = Matrix<4, 4>;

template <typename T>
T Clamp(T value, T min, T max) {
    return std::min(std::max(value, min), max);
}

template <typename T>
inline T Lerp(T a, T b, real percent) {
    return a + (b - a) * percent;
}

Mat44 CreateEularRotate_x(float angle) {
    real c = std::cos(angle);
    real s = std::sin(angle);
    // clang-format off
    return Mat44 {
        1.0, 0.0, 0.0, 0.0, 
        0.0, c, -s, 0.0, 
        0.0, s, c, 0.0, 
        0.0, 0.0, 0.0, 1.0,
    };
    // clang-format on
}
Mat44 CreateEularRotate_y(float angle) {
    real c = std::cos(angle);
    real s = std::sin(angle);
    // clang-format off
    return Mat44 {
          c, 0.0,   s, 0.0,
        0.0, 1.0, 0.0, 0.0,
         -s, 0.0,   c, 0.0,
        0.0, 0.0, 0.0, 1.0,
    };
    // clang-format on
}
Mat44 CreateEularRotate_z(float angle) {
    real c = std::cos(angle);
    real s = std::sin(angle);
    // clang-format off
    return Mat44 {
          c,  -s, 0.0, 0.0,
          s,   c, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
    };
    // clang-format on
}
// 欧拉旋转角
Mat44 CreateEularRotate_xyz(Vec3 &rotation) {
    return CreateEularRotate_x(rotation.x) * CreateEularRotate_y(rotation.y) *
           CreateEularRotate_z(rotation.z);
}

inline Mat44 CreateTranslate(Vec3 &trans) {
    // clang-format off
  return Mat44{
    1, 0, 0, trans.x,
    0, 1, 0, trans.y,
    0, 0, 1, trans.z,
    0, 0, 0, 1
  };
    // clang-format on
}

inline Mat44 CreateTranslate(Vec3 &&trans) { return CreateTranslate(trans); }

inline Mat44 CreateRotate(Vec3 &rotate) {
    real sinx = std::sin(rotate.x), cosx = std::cos(rotate.x),
         siny = std::sin(rotate.y), cosy = std::cos(rotate.y),
         sinz = std::sin(rotate.z), cosz = std::cos(rotate.z);
    // clang-format off
  return Mat44{
    cosz, -sinz, 0, 0,
    sinz,  cosz, 0, 0,
       0,     0, 1, 0,
       0,     0, 0, 1,
  } * Mat44{
    cosy, 0, siny, 0,
       0, 1,    0, 0,
   -siny, 0, cosy, 0,
       0, 0,    0, 1
  } * Mat44{
    1,    0,     0, 0,
    0, cosx, -sinx, 0,
    0, sinx,  cosx, 0,
    0,    0,     0, 1
  };
    // clang-format on
}

inline Mat44 CreateScale(Vec3 &scale) {
    // clang-format off
  return Mat44{
    scale.x,       0,       0, 0,
          0, scale.y,       0, 0,
          0,       0, scale.z, 0,
          0,       0,       0, 1
  };
    // clang-format on
}

inline Mat44 CreateScale(Vec3 &&scale) { return CreateScale(scale); }

struct Rect {
    Vec2 pos;
    Vec2 size;
};

inline std::ostream &operator<<(std::ostream &o, const Rect &r) {
    printf("Rect(%f, %f, %f, %f)", r.pos.x, r.pos.y, r.size.w, r.size.h);
    return o;
}

inline real Radians(real degrees) { return degrees * M_PI / 180.0f; }

inline real Degrees(real radians) { return radians * 180.0f / M_PI; }

inline bool IsPointInRect(const Vec2 &p, const Rect &r) {
    return p.x >= r.pos.x && p.x <= r.pos.x + r.size.w && p.y >= r.pos.y &&
           p.y <= r.pos.y + r.size.h;
}

inline bool RectsIntersect(const Rect &r1, const Rect &r2, Rect *result) {
    Rect rect;
    rect.pos.x = std::max(r1.pos.x, r2.pos.x);
    rect.pos.y = std::max(r1.pos.y, r2.pos.y);
    rect.pos.w =
        std::min(r1.pos.x + r1.size.w, r2.pos.x + r2.size.w) - rect.pos.x;
    rect.pos.h =
        std::min(r1.pos.y + r1.size.h, r2.pos.y + r2.size.h) - rect.pos.y;
    if (rect.size.w > 0 && rect.size.h > 0) {
        if (result) {
            *result = rect;
        }
        return true;
    }
    return false;
}

class Barycentric {
   public:
    real alpha;
    real beta;
    real gamma;
    Barycentric(Vec2 &pt, std::array<Vec2, 3> &triangle) {
        auto areaTwice =
            Cross(triangle[1] - triangle[0], triangle[2] - triangle[0]);
        alpha = std::abs(Cross(triangle[1] - pt, triangle[2] - pt) / areaTwice);
        beta = std::abs(Cross(triangle[0] - pt, triangle[2] - pt) / areaTwice);
        gamma = std::abs(Cross(triangle[0] - pt, triangle[1] - pt) / areaTwice);
    }

    Barycentric(Vec2 &&pt, std::array<Vec2, 3> &&triangle)
        : Barycentric(pt, triangle) {}

    bool IsValid() { return alpha + beta + gamma <= 1.000001; }
};

// inline Vec3 Barycentric(const Vec2 &v1, const Vec2 &v2, const Vec2 &v3,
//                         const Vec2 &p) {
//     Vec3 result;
//     Vec3 c1{v1.x - v2.x, v1.x - v3.x, p.x - v1.x},
//         c2{v1.y - v2.y, v1.y - v3.y, p.y - v1.y};
//     result = Cross(c1, c2);
//     if (result.z == 0) {
//         // (-1, -1, -1) means a invalid condition, should discard this point
//         return Vec3{-1, -1, -1};
//     }
//     return Vec3{1 - result.x / result.z - result.y / result.z,
//                 result.x / result.z, result.y / result.z};
// }

inline Rect GetTriangleAABB(const Vec2 &v1, const Vec2 &v2, const Vec2 &v3) {
    // 需要include algorithm
    int x1 = std::min({v1.x, v2.x, v3.x}), y1 = std::min({v1.y, v2.y, v3.y}),
        x2 = std::max({v1.x, v2.x, v3.x}), y2 = std::max({v1.y, v2.y, v3.y});

    return Rect{Vec2{real(x1), real(y1)}, Vec2{real(x2 - x1), real(y2 - y1)}};
}

template <typename T>
inline int Sign(T value) {
    return value > 0 ? 1 : (value == 0 ? 0 : -1);
}
