#pragma once
#include "base_renderer.hpp"
#include "math.hpp"

Attributes GetCorrectedAttribute(float z, std::vector<Vertex> vertices,
                                 Barycentric &barycentric);

class GpuRenderer : public IRenderer {
   private:
    ColorAttachment colorAttachment_;
    DepthAttachment depthAttachment_;
    Camera camera_;
    Viewport viewport_;
    Shader shader_;
    Uniforms uniforms_;
    FrontFace frontFace_;
    FaceCull cull_;
    bool enableFramework_;

    void rasterizeTriangle(Mat44 &model, std::vector<Vertex> &vertices,
                           TextureStorage &textureStorage) {
        // call vertex changing function to change vertex position and set
        // attribtues
        for (auto &v : vertices) {
            v = shader_.CallVertexChanging(v, uniforms_, textureStorage);
        }
        // Model and View transform
        for (auto &v : vertices) {
            v.position = camera_.view_mat_ * model * v.position;
        }

        std::vector<Vec3> positions;
        std::transform(vertices.begin(), vertices.end(),
                       std::back_inserter(positions),
                       [](Vertex v) { return v.position.TruncatedToVec3(); });

        // face cull
        if (ShouldCull(positions, camera_.view_dir_, frontFace_, cull_)) {
            return;
        }

        for (auto &v : vertices) {
            // project transform
            v.position = camera_.frustum_.mat * v.position;
            // save truely z
            // 这里w=z，所以不需要动
            v.position.z = -v.position.w;

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

        // find AABB for triangle
        auto aabbMinX = FLT_MAX;
        auto aabbMaxX = -FLT_MAX;
        auto aabbMinY = FLT_MAX;
        auto aabbMaxY = -FLT_MAX;
        for (auto &v : vertices) {
            aabbMinX = std::min(aabbMinX, v.position.x);
            aabbMinY = std::min(aabbMinY, v.position.y);
            aabbMaxX = std::max(aabbMaxX, v.position.x);
            aabbMaxY = std::max(aabbMaxY, v.position.y);
        }
        Vec2 aabbMax = Vec2{aabbMaxX, aabbMaxY};
        Vec2 aabbMin = Vec2{aabbMinX, aabbMinY};
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
            // walk through all pixel in AABB and set color
            // 这里循环中x和y不能用auto
            for (int x = aabbMin.x; x <= aabbMax.x; x++) {
                for (int y = aabbMin.y; y <= aabbMax.y; y++) {
                    auto barycentric = Barycentric(
                        Vec2{x * 1.0f, y * 1.0f},
                        std::array<Vec2, 3>{Vec2{vertices[0].position.x,
                                                 vertices[0].position.y},
                                            Vec2{vertices[1].position.x,
                                                 vertices[1].position.y},
                                            Vec2{vertices[2].position.x,
                                                 vertices[2].position.y}});
                    //  如果在三角形内
                    if (barycentric.IsValid()) {
                        //
                        auto invZ = barycentric.alpha / vertices[0].position.z +
                                    barycentric.beta / vertices[1].position.z +
                                    barycentric.gamma / vertices[2].position.z;
                        auto z = 1.0f / invZ;
                        // depth test and near plane
                        if (z < camera_.frustum_.near &&
                            depthAttachment_.Get(x, y) <= z) {
                            auto attr =
                                GetCorrectedAttribute(z, vertices, barycentric);
                            auto color = shader_.CallPixelShading(
                                attr, uniforms_, textureStorage);
                            colorAttachment_.Set(x, y, color);
                            depthAttachment_.Set(x, y, z);
                        }
                    }
                }
            }
        }
    }

   public:
    GpuRenderer(GpuRenderer &r) = default;

    GpuRenderer(uint32_t w, uint32_t h, Camera camera)
        : colorAttachment_(ColorAttachment{w, h}),
          depthAttachment_(DepthAttachment{w, h}),
          camera_(camera),
          viewport_(Viewport{0, 0, w, h}),
          shader_(Shader{}),
          uniforms_(Uniforms{}),
          frontFace_(FrontFace::CW),
          cull_(FaceCull::None),
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
            rasterizeTriangle(model, vertices_, textureStorage);
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

Attributes GetCorrectedAttribute(float z, std::vector<Vertex> vertices,
                                 Barycentric &barycentric) {
    Attributes attr = Attributes();
    for (int i = 0; i < attr.varyingFloat.size(); i++) {
        attr.varyingFloat[i] =
            (vertices[0].attributes.varyingFloat[i] * barycentric.alpha /
                 vertices[0].position.z +
             vertices[1].attributes.varyingFloat[i] * barycentric.beta /
                 vertices[1].position.z +
             vertices[2].attributes.varyingFloat[i] * barycentric.gamma /
                 vertices[2].position.z) *
            z;
        attr.varyingVec2[i] = (vertices[0].attributes.varyingVec2[i] *
                                   barycentric.alpha / vertices[0].position.z +
                               vertices[1].attributes.varyingVec2[i] *
                                   barycentric.beta / vertices[1].position.z +
                               vertices[2].attributes.varyingVec2[i] *
                                   barycentric.gamma / vertices[2].position.z) *
                              z;
        attr.varyingVec3[i] = (vertices[0].attributes.varyingVec3[i] *
                                   barycentric.alpha / vertices[0].position.z +
                               vertices[1].attributes.varyingVec3[i] *
                                   barycentric.beta / vertices[1].position.z +
                               vertices[2].attributes.varyingVec3[i] *
                                   barycentric.gamma / vertices[2].position.z) *
                              z;
        attr.varyingVec4[i] = (vertices[0].attributes.varyingVec4[i] *
                                   barycentric.alpha / vertices[0].position.z +
                               vertices[1].attributes.varyingVec4[i] *
                                   barycentric.beta / vertices[1].position.z +
                               vertices[2].attributes.varyingVec4[i] *
                                   barycentric.gamma / vertices[2].position.z) *
                              z;
    }

    return attr;
}
