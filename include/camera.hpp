#pragma once
#include <initializer_list>

#include "math.hpp"

class Frustum {
   public:
    float near;
    float far;
    float aspect;
    float fov;

    Mat44 mat;

    Frustum(float near, float far, float aspect, float fov)
        : near(near), far(far), aspect(aspect), fov(fov) {
#ifdef CPU_FEATURE_ENABLED
        // Code to be executed when the "cpu" feature is enabled
        float a = 1.0f / (near * std::tan(fov * 0.5));
        // clang-format off
        mat = Mat44(std::initializer_list<real>{
            a,          0,          0, 0, 
            0, aspect * a,          0, 0,
            0,          0,        1.0, 0,
            0,          0, -1.0f/near, 0});
        // clang-format on
#else
        // Code to be executed when the "cpu" feature is not enabled
        // Since glFrustum() accepts only positive values of near and far
        // distances, we need to negate them during the construction of
        // GL_PROJECTION matrix.
        // 所以n和f都要预先加上负号，因此矩阵和我笔记的不太同
        float tan = std::tan(fov * 0.5);
        auto sign = near > 0 ? 1 : (near == 0 ? 0 : -1);
        // clang-format off
        mat = Mat44(std::initializer_list<real>{
            sign / (aspect * tan),          0,                          0,                             0,
                                0, sign / tan,                          0,                             0,
                                0,          0,(near + far) / (near - far), -2 * near * far / (far - near), 
                                0,          0,                          -1,                             0});
        // clang-format on

#endif
    }

    // 视锥剔除
    bool Contain(Vec3& pt) {
        float half_h = near * std::tan(fov * 0.5) / aspect;
        float h_fovy_cos = std::cos(fov / 2);
        float h_fovy_sin = std::sin(fov / 2);
        // 以下都是法线
        return !(
            Dot(Vec3{h_fovy_cos, 0.0, h_fovy_sin}, pt) >= 0.0  // right plane
            || Dot(Vec3{-h_fovy_cos, 0.0, h_fovy_sin}, pt) >= 0.0  // left plane
            || Dot(Vec3{0.0, near, half_h}, pt) >= 0.0             // top plane
            || Dot(Vec3{0.0, -near, half_h}, pt) >= 0.0  // bottom plane
            || pt.z >= -near                             // near plane
            || pt.z <= -far);                            // far plane
    }
};

class Camera {
   private:
    // eye
    Vec3 position_;
    Vec3 rotation_;

    // 重新计算view_mat_
    void RecalculateViewMat();

   public:
    Frustum frustum_;
    Vec3 view_dir_;
    // view matrix
    Mat44 view_mat_;

    Camera(float near, float far, float aspect, float fov);
    ~Camera();
    void MoveTo(Vec3 position);
    void MoveOffset(Vec3 offset);
    Mat44 SetLookAt(Vec3 target);
    void SetRotation(Vec3 rotation);
    void SetUpDirection(Vec3 upDirection);
};

Camera::Camera(float near, float far, float aspect, float fov)
    : frustum_(Frustum(near, far, aspect, fov)),
      position_(Vec3::Zero),
      rotation_(Vec3::Zero),
      view_dir_(-Vec3::Z_Axis),
      view_mat_(Mat44::Eye()) {}

Camera::~Camera() {}

void Camera::RecalculateViewMat() {
    auto rotation = -rotation_;
    auto position = -position_;
    auto rotationMat44 = CreateEularRotate_xyz(rotation);
    view_mat_ = rotationMat44 * CreateTranslate(position);
    view_dir_ = (rotationMat44* Vec4{0.0, 0.0, -1.0, 1.0}).TruncatedToVec3();
}

void Camera::MoveTo(Vec3 position) {
    position_ = position;
    RecalculateViewMat();
}

void Camera::MoveOffset(Vec3 offset) {
    position_ = position_ + offset;
    RecalculateViewMat();
}

void Camera::SetRotation(Vec3 rotation) {
    rotation_ = rotation;
    RecalculateViewMat();
}

Mat44 Camera::SetLookAt(Vec3 target) {
    // look-at / gaze direction
    view_dir_ = Normalize(position_ - target);
    Vec3 back = -view_dir_;
    Vec3 upT = Vec3::Y_Axis;
    Vec3 right = Normalize(Cross(upT, back));
    Vec3 up = Normalize(Cross(back, right));
    // view_mat = rotation_mat * translate_mat
    // clang-format off
    std::initializer_list<Vec4> mat = {
        Vec4{right.x, right.y, right.z, -Dot(position_, right)},
        Vec4{   up.x,    up.y,    up.z,    -Dot(position_, up)},
        Vec4{ back.x,  back.y,  back.z,  -Dot(position_, back)},
        Vec4{    0.0,     0.0,     0.0,                   1.0}};
    // clang-format on
    view_mat_ = Mat44(mat);
    real x = acos(Dot(Vec3::Y_Axis, Vec3{0.0, view_dir_.y, view_dir_.z}));
    real y = acos(Dot(Vec3::Z_Axis, Vec3{view_dir_.x, 0.0, view_dir_.z}));
    real z = acos(Dot(Vec3::X_Axis, Vec3{view_dir_.x, view_dir_.y, 0.0}));
    rotation_ = Vec3{x, y, z};
    return view_mat_;
}
