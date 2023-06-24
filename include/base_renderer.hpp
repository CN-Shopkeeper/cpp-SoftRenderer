#pragma once
#include <optional>
#include <tuple>

#include "camera.hpp"
#include "image.hpp"
#include "line.hpp"
#include "math.hpp"
#include "shader.hpp"
#include "texture.hpp"

class Viewport {
   public:
    int x;
    int y;
    int w;
    int h;
};

enum FaceCull { Front, Back, None };

enum FrontFace { CW, CCW };

class IRenderer {
   public:
    virtual ~IRenderer() {}
    static void Init() { IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG); }

    static void Quit() { IMG_Quit(); }
    virtual void Clear(Vec4 color) = 0;
    virtual void ClearDepth() = 0;
    virtual uint32_t GetCanvaWidth() = 0;
    virtual uint32_t GetCanvaHeight() = 0;
    virtual void DrawTriangle(Mat44 model, std::vector<Vertex> vertices,
                              TextureStorage &texture_storage) = 0;
    virtual std::vector<uint8_t> GerRenderedImage() = 0;
    virtual Shader GetShader() = 0;
    virtual Uniforms GetUniforms() = 0;
    virtual Camera GetCamera() = 0;
    virtual void SetCamera(Camera Camera) = 0;
    // 以及其他的方法
    virtual void SetFrontFace(FrontFace front_face);
    virtual FrontFace GetFrontFace();
    virtual void SetFaceCull(FaceCull face_cull);
    virtual FaceCull GetFaceCull();
    virtual void EnableFramework();
    virtual void DisableFramework();
};

// Bresenham对象，用于绘制线段，可用cohen sutherland算法切割
class Bresenham {
   private:
    int final_x_;
    int x_, y_;
    bool steep_;
    int e_;
    int sy_, sx_;
    int desc_, step_;
    Bresenham(int x, int y, int final_x, int e, int desc, int step, int sx,
              int sy, bool steep)
        : x_(x),
          y_(y),
          final_x_(final_x),
          e_(e),
          desc_(desc),
          step_(step),
          sx_(sx),
          sy_(sy),
          steep_(steep) {}

   public:
    // 构造Bresenham对象
    static std::optional<Bresenham> New(const Vec2 &p0, const Vec2 &p1,
                                        const Vec2 &min, const Vec2 &max);
    inline bool Finished() { return x_ == final_x_; }
    inline Vec2 CurrPoint() { return Vec2{x_ * 1.0f, y_ * 1.0f}; }
    // Bresenham向前一步
    std::optional<Vec2> Step();
};

class CohenSutherland {
   private:
    inline static uint8_t INSIDE = 0;
    inline static uint8_t LEFT = 1;
    inline static uint8_t RIGHT = 2;
    inline static uint8_t BOTTOM = 4;
    inline static uint8_t TOP = 8;

    static uint8_t compute_outcode(const Vec2 &p, const Vec2 &min,
                                   const Vec2 &max) {
        return (p.x < min.x ? LEFT : (p.x > max.x ? RIGHT : INSIDE)) |
               (p.y < min.y ? BOTTOM : (p.y > max.y ? TOP : INSIDE));
    }

   public:
    static std::optional<std::tuple<Vec2, Vec2>> CohenSutherlandLineClip(
        const Vec2 &p1, const Vec2 &p2, const Vec2 &rect_min,
        const Vec2 &rect_max);
};

std::optional<Bresenham> Bresenham::New(const Vec2 &p0, const Vec2 &p1,
                                        const Vec2 &min, const Vec2 &max) {
    // 切割之后
    auto result = CohenSutherland::CohenSutherlandLineClip(p0, p1, min, max);
    if (result == std::nullopt) {
        return std::nullopt;
    } else {
        auto [v0, v1] = result.value();

        auto x0 = v0.x;
        auto y0 = v0.y;
        auto x1 = v1.x;
        auto y1 = v1.y;

        float dx = std::abs(x1 - x0);
        float dy = std::abs(y1 - y0);
        auto sx = x1 > x0 ? 1 : -1;
        auto sy = y1 > y0 ? 1 : -1;
        auto x = x0;
        auto y = y0;
        bool steep = dx < dy;

        auto final_x = dx < dy ? y1 : x1;
        if (dx < dy) {
            std::swap(dx, dy);
            std::swap(x, y);
            std::swap(sx, sy);
        }

        auto e = -dx;
        auto const step = 2 * dy;
        auto const desc = -2 * dx;

        return Bresenham(x, y, final_x, e, desc, step, sx, sy, steep);
    }
}

std::optional<Vec2> Bresenham::Step() {
    if (Finished()) {
        return std::nullopt;
    } else {
        Vec2 result =
            steep_ ? Vec2{y_ * 1.0f, x_ * 1.0f} : Vec2{x_ * 1.0f, y_ * 1.0f};
        e_ += step_;
        if (e_ >= 0) {
            y_ += sy_;
            e_ += desc_;
        }
        x_ += sx_;
        return result;
    }
}

std::optional<std::tuple<Vec2, Vec2>> CohenSutherland::CohenSutherlandLineClip(
    const Vec2 &p1, const Vec2 &p2, const Vec2 &rect_min,
    const Vec2 &rect_max) {
    auto pt1 = p1;
    auto pt2 = p2;

    auto outcode1 = compute_outcode(pt1, rect_min, rect_max);
    auto outcode2 = compute_outcode(pt2, rect_min, rect_max);

    while (true) {
        // 完全在屏幕外，直接舍弃
        if ((outcode1 & outcode2) != 0) {
            return std::nullopt;
        }
        // 完全在屏幕内
        else if ((outcode1 | outcode2) == 0) {
            return std::make_tuple(pt1, pt2);
        }
        auto p = Vec2::Zero;

        auto outcode = std::max(outcode1, outcode2);
        if ((outcode & TOP) != 0) {
            p.x = pt1.x +
                  (pt2.x - pt1.x) * (rect_max.y - pt1.y) / (pt2.y - pt1.y);
            p.y = rect_max.y;
        } else if ((outcode & BOTTOM) != 0) {
            p.x = pt1.x +
                  (pt2.x - pt1.x) * (rect_min.y - pt1.y) / (pt2.y - pt1.y);
            p.y = rect_min.y;
        } else if ((outcode & RIGHT) != 0) {
            p.y = pt1.y +
                  (pt2.y - pt1.y) * (rect_max.x - pt1.x) / (pt2.x - pt1.x);
            p.x = rect_max.x;
        } else if ((outcode & LEFT) != 0) {
            p.y = pt1.y +
                  (pt2.y - pt1.y) * (rect_min.x - pt1.x) / (pt2.x - pt1.x);
            p.x = rect_min.x;
        }

        if (outcode == outcode1) {
            pt1 = p;
            outcode1 = compute_outcode(pt1, rect_min, rect_max);
        } else {
            pt2 = p;
            outcode2 = compute_outcode(pt2, rect_min, rect_max);
        }
    }
}

Vec4 TextureSample(Texture &texture, Vec2 &texcoord) {
    uint32_t x = texcoord.x * (texture.Width() - 1);
    uint32_t y = texcoord.y * (texture.Height() - 1);
    return texture.GetPixel(x, y);
}

bool ShouldCull(Vec3 (&positions)[3], Vec3 &view_dir, FrontFace face,
                FaceCull cull) {
    auto norm = Cross(positions[1] - positions[0], positions[2] - positions[1]);
    bool is_front_face;
    switch (face) {
        case FrontFace::CW:
            is_front_face = Dot(norm, view_dir) > 0.0f;
            break;
        default:
            is_front_face = Dot(norm, view_dir) <= 0.0f;
            break;
    }
    switch (cull) {
        case FaceCull::Front:
            return is_front_face;
        case FaceCull::Back:
            return !is_front_face;
        default:
            return false;
    }
}

void RasterizeLine(Line &line, PixelShading &shading, Uniforms &uniforms,
                   TextureStorage &texture_storage,
                   ColorAttachment &color_attachment,
                   DepthAttachment &depth_attachment) {
    auto p0 = line.start.position.TruncatedToVec2();
    auto p1 = line.end.position.TruncatedToVec2();
    auto bresenhamOpt =
        Bresenham::New(line.start.position.TruncatedToVec2(),
                       line.end.position.TruncatedToVec2(), Vec2::Zero,
                       Vec2{(color_attachment.width - 1) * 1.0f,
                            (color_attachment.height - 1) * 1.0f});
    if (bresenhamOpt != std::nullopt) {
        auto bresenham = bresenhamOpt.value();
        auto position = bresenham.Step();
        auto vertex = line.start;
        while (position != std::nullopt) {
            unsigned int x = position.value().x;
            unsigned int y = position.value().y;
            auto rhw = vertex.position.z;
            float z = 1.0 / rhw;

            if (depth_attachment.Get(x, y) < z) {
                auto attr = vertex.attributes;
                AttributesForeach(attr, [=](float value) { return value * z; });
                auto color =
                    shading(vertex.attributes, uniforms, texture_storage);
                color_attachment.Set(x, y, color);
                depth_attachment.Set(x, y, z);
            }
            vertex.position += line.step.position;
            vertex.attributes = InterpAttributes(
                vertex.attributes, line.step.attributes,
                [](float value1, float value2, float) {
                    return value1 + value2;
                },
                0.0);
            position = bresenham.Step();
        }
    }
}
