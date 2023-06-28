#pragma once
#include "base_renderer.hpp"
#include "scanline.hpp"

enum RasterizeResult { Ok, Discard, GenerateNewFace };

class CpuRenderer : public IRenderer {
   private:
    ColorAttachment colorAttachment_;
    DepthAttachment depthAttachment_;
    Camera camera_;
    Viewport viewport_;
    Shader shader_;
    Uniforms uniforms_;
    FrontFace frontFace_;
    FaceCull cull_;

    std::vector<Vertex> clipedTrangles_;
    bool enableFramework_;

    void drawScanline(Scanline &scanline, TextureStorage &textureStorage) {
        auto &vertex = scanline.vertex;
        auto y = scanline.y;
        while (scanline.width > 0.0) {
            auto rhw = vertex.position.z;
            auto z = 1.0f / rhw;
            auto x = vertex.position.x;
            if (x >= 0.0 && x < colorAttachment_.width) {
                if (depthAttachment_.Get(x, y) <= z) {
                    auto attr = vertex.attributes;
                    AttributesForeach(attr,
                                      [=](float value) { return value / rhw; });
                    auto color = shader_.CallPixelShading(attr, uniforms_,
                                                          textureStorage);
                    colorAttachment_.Set(x, y, color);
                    depthAttachment_.Set(x, y, z);
                }
            }

            scanline.width -= 1.0;
            vertex.position += scanline.step.position;
            vertex.attributes = InterpAttributes(
                vertex.attributes, scanline.step.attributes,
                [](float value1, float value2, float) {
                    return value1 + value2;
                },
                0.0);
        }
    }

    void drawTrapezoid(Trapezoid &trap, TextureStorage &textureStorage) {
        int top = std::max(std::ceil(trap.top), 0.0f);
        int bottom = (int)(std::min(std::ceil(trap.bottom),
                                    colorAttachment_.height - 1.0f)) -
                     1;
        float y = top;

        VertexRhwInit(trap.left.v1);
        VertexRhwInit(trap.left.v2);
        VertexRhwInit(trap.right.v1);
        VertexRhwInit(trap.right.v2);

        while (y <= bottom) {
            auto scanline = Scanline::FromTrapezoid(trap, y);
            drawScanline(scanline, textureStorage);
            y += 1;
        }
    }

    RasterizeResult rasterizeTriangle(Mat44 &model,
                                      std::vector<Vertex> &vertices,
                                      TextureStorage &textureStorage) {
        // call vertex changing function to change vertex position and set
        // attribtues
        for (auto &v : vertices) {
            v = shader_.CallVertexChanging(v, uniforms_, textureStorage);
        }

        // Model transform
        for (auto &v : vertices) {
            v.position = model * v.position;
        }

        std::vector<Vec3> positions;
        std::transform(vertices.begin(), vertices.end(),
                       std::back_inserter(positions),
                       [](Vertex v) { return v.position.TruncatedToVec3(); });

        // face cull
        if (ShouldCull(positions, camera_.view_dir_, frontFace_, cull_)) {
            return RasterizeResult::Discard;
        }

        // view transform
        for (auto &v : vertices) {
            v.position = camera_.view_mat_ * v.position;
        }

        std::transform(vertices.begin(), vertices.end(),
                       std::back_inserter(positions),
                       [](Vertex v) { return v.position.TruncatedToVec3(); });

        // frustum clip
        bool frustumClip = true;
        for (auto &p : positions) {
            if (camera_.frustum_.Contain(p)) {
                frustumClip = false;
                break;
            }
        }
        if (frustumClip) {
            return RasterizeResult::Discard;
        }

        // near plane clip
        bool nearPlaneClip = false;
        for (auto &p : positions) {
            if (p.z > camera_.frustum_.near) {
                nearPlaneClip = true;
                break;
            }
        }
        if (nearPlaneClip) {
            auto [face1, face2Opt] =
                NearPlaneClip(vertices, camera_.frustum_.near);
            for (auto &v : face1) clipedTrangles_.push_back(v);
            if (face2Opt.has_value()) {
                for (auto &v : face2Opt.value()) clipedTrangles_.push_back(v);
            }
            return RasterizeResult::GenerateNewFace;
        }

        for (auto &v : vertices) {
            // project transform
            v.position = camera_.frustum_.mat * v.position;
            // save truely z
            // frustum的矩阵中w=-z/near，需要还原回去
            v.position.z = -v.position.w * camera_.frustum_.near;
            // perspective divide
            v.position.x /= v.position.w;
            v.position.y /= v.position.w;
            v.position.w = 1.0;
            // Viewport transform
            v.position.x =
                (v.position.x + 1.0) * 0.5 * (viewport_.w - 1.0) + viewport_.x;
            v.position.y = viewport_.h -
                           (v.position.y + 1.0) * 0.5 * (viewport_.h - 1.0) +
                           viewport_.y;
        }

        if (enableFramework_) {
            // draw line framework
            for (int i = 0; i < 3; i++) {
                auto v1 = vertices[i];
                auto v2 = vertices[(i + 1) % 3];
                v1.position.z = 1.0 / v1.position.z;
                v2.position.z = 1.0 / v2.position.z;
                Line line = Line{v1, v2};
                RasterizeLine(line, shader_.pixelShading, uniforms_,
                              textureStorage, colorAttachment_,
                              depthAttachment_);
            }
        } else {
            auto [trap1Opt, trap2Opt] = Trapezoid::FromTriangle(vertices);
            if (trap1Opt.has_value()) {
                drawTrapezoid(trap1Opt.value(), textureStorage);
            }
            if (trap2Opt.has_value()) {
                drawTrapezoid(trap2Opt.value(), textureStorage);
            }
        }
        return RasterizeResult::Ok;
    }

   public:
    CpuRenderer(CpuRenderer &r) = default;

    CpuRenderer(uint32_t w, uint32_t h, Camera camera)
        : colorAttachment_(ColorAttachment{w, h}),
          depthAttachment_(DepthAttachment{w, h}),
          camera_(camera),
          viewport_(Viewport{0, 0, w, h}),
          shader_(Shader{}),
          uniforms_(Uniforms{}),
          frontFace_(FrontFace::CW),
          cull_(FaceCull::None),
          clipedTrangles_(std::vector<Vertex>()),
          enableFramework_(false) {}

    void Clear(Vec4 &color) override { colorAttachment_.Clear(color); }

    uint32_t GetCanvaWidth() override { return colorAttachment_.width; }

    uint32_t GetCanvaHeight() override { return colorAttachment_.height; }

    std::vector<uint8_t> GetRenderedImage() override {
        return colorAttachment_.data;
    }

    void DrawTriangle(Mat44 &model, std::vector<Vertex> &vertices,
                      TextureStorage &textureStorage) override {
        for (int i = 0; i < vertices.size() / 3; i++) {
            std::vector<Vertex> vertices_ = std::vector<Vertex>{
                vertices[i * 3], vertices[i * 3 + 1], vertices[i * 3 + 2]};
            auto result = rasterizeTriangle(model, vertices_, textureStorage);

            switch (result) {
                case RasterizeResult::Ok:
                case RasterizeResult::Discard:
                    break;
                case RasterizeResult::GenerateNewFace:
                    for (int i = 0; i < clipedTrangles_.size() / 3; i++) {
                        auto vertices_ = std::vector<Vertex>{
                            clipedTrangles_[i * 3], clipedTrangles_[i * 3 + 1],
                            clipedTrangles_[i * 3 + 2]};
                        auto result_ =
                            rasterizeTriangle(model, vertices_, textureStorage);
                        switch (result_) {
                            case RasterizeResult::Ok:

                                break;
                            case RasterizeResult::Discard:
                            case RasterizeResult::GenerateNewFace:
                                SDL_Log(
                                    "discard or generate new face from clipped "
                                    "face");
                                break;
                            default:
                                break;
                        }
                        clipedTrangles_.clear();
                    }
                default:
                    break;
            }
        }
    }

    Shader &GetShader() override { return shader_; }

    Uniforms &GetUniforms() override { return uniforms_; }

    void ClearDepth() override {
        // FLT_MIN is equal to 0
        depthAttachment_.Clear(-FLT_MAX);
    }

    Camera &GetCamera() override { return camera_; }

    void SetCamera(Camera camera) override { camera_ = camera; }

    void SetFrontFace(FrontFace frontFace) override { frontFace_ = frontFace; }

    FrontFace &GetFrontFace() override { return frontFace_; }

    void SetFaceCull(FaceCull cull) override { cull_ = cull; }

    FaceCull &GetFaceCull() override { return cull_; }

    void EnableFramework() override { enableFramework_ = true; }

    void DisableFramework() override { enableFramework_ = false; }

    SDL_Surface *GetSurface() override {
        return colorAttachment_.ConvertToSurface();
    }
};