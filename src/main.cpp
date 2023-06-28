#include <algorithm>
#include <iostream>
#include <memory>

#include "base_renderer.hpp"
#include "cpu_renderer.hpp"
#include "image.hpp"
#include "interactive.hpp"
#include "model.hpp"
#include "obj_loader.hpp"
#include "scanline.hpp"
#include "shader.hpp"
#include "texture.hpp"

const uint32_t WINDOW_WIDTH = 1024;
const uint32_t WINDOW_HEIGHT = 720;

// attribute location
const size_t ATTR_TEXCOORD = 0;  // vec2
const size_t ATTR_NORMAL = 0;    // vec3

// uniforms location
const uint32_t UNIFORM_TEXTURE = 0;  // vec2
const uint32_t UNIFORM_COLOR = 1;    // vec4

std::unique_ptr<IRenderer> CreateRenderer(uint32_t w, uint32_t h,
                                          Camera camera) {
#ifdef CPU_FEATURE_ENABLED
    return std::make_unique<CpuRenderer>(w, h, camera);
#else
// todo
#endif
}

struct StructedModelData {
    std::vector<Vertex> vertices;
    std::optional<uint32_t> mtllib;
    std::optional<std::string> material;
};

std::vector<StructedModelData> RestructModelVertex(
    std::vector<model::Mesh>& meshes) {
    std::vector<StructedModelData> datas;
    for (auto& mesh : meshes) {
        std::vector<Vertex> vertices;
        for (auto& modelVertex : mesh.vertices) {
            auto attr = Attributes();
            attr.varyingVec2[ATTR_TEXCOORD] = modelVertex.texcoord;
            attr.varyingVec3[ATTR_NORMAL] = modelVertex.normal;
            vertices.push_back(Vertex{modelVertex.position, attr});
        }
        datas.push_back(
            StructedModelData{vertices, mesh.mtllib, mesh.material});
    }
    return datas;
}

class RedBirdApp : public App {
   private:
    std::unique_ptr<IRenderer> renderer_;

    float rotation_;
    std::vector<StructedModelData> vertexDatas_;
    std::vector<objloader::Mtllib> mtllibs_;
    TextureStorage textureStorage_;

   public:
    RedBirdApp() : App("Red Bird APP!", WINDOW_WIDTH, WINDOW_HEIGHT) {}

    void OnInit() override {
        rotation_ = 0.0f;
        auto camera = Camera{1.0, 1000.0, 1.0f * WINDOW_WIDTH / WINDOW_HEIGHT,
                             Radians(90.0f)};
        camera.MoveTo(Vec3{0.0, 1.0, 0.0});
        camera.SetRotation(Vec3{Radians(1.0f), 0.0, 0.0});

        // init renderer and textureStorage
        // 开启SDL_IMAGE
        renderer_->Init();
        renderer_ = CreateRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, camera);
        renderer_->SetFrontFace(FrontFace::CCW);
        renderer_->SetFaceCull(FaceCull::Back);
        renderer_->EnableFramework();
        textureStorage_ = TextureStorage();

        // data prepare, from OBJ model
        std::string MODEL_ROOT_DIR = "./resources/Red";
        auto modelResult = model::LoadFromFile(
            std::filesystem::path{MODEL_ROOT_DIR}.append("Red.obj").string(),
            model::PreOperation::None);
        if (!modelResult.has_value()) {
            SDL_Log("load model from %s failed!", MODEL_ROOT_DIR.c_str());
            return;
        }
        auto [meshes, mtllibs] = modelResult.value();
        mtllibs_ = mtllibs;
        vertexDatas_ = RestructModelVertex(meshes);
        for (auto& mtllib : mtllibs) {
            for (auto [_, material] : mtllib.materials) {
                if (material.textureMaps.diffuse.has_value()) {
                    auto diffuseMap = material.textureMaps.diffuse.value();
                    textureStorage_.load(std::filesystem::path{MODEL_ROOT_DIR}
                                             .append(diffuseMap)
                                             .string()
                                             .c_str(),
                                         diffuseMap);
                };
            }
        }

        // vertex changing shader
        renderer_->GetShader().vertexChanging =
            [](Vertex& vertex, Uniforms& Uniforms,
               TextureStorage& textureStorage) { return vertex; };
        renderer_->GetShader().pixelShading =
            [](Attributes& attr, Uniforms& uniforms,
               TextureStorage& textureStorage) {
                auto fragColor = uniforms.varyingVec4.find(UNIFORM_COLOR) ==
                                         uniforms.varyingVec4.end()
                                     ? Vec4{1.0, 1.0, 1.0, 1.0}
                                     : uniforms.varyingVec4[UNIFORM_COLOR];
                auto texcoord = attr.varyingVec2[ATTR_TEXCOORD];
                texcoord.x = std::clamp(texcoord.x, 0.0f, 1.0f);
                texcoord.y = std::clamp(texcoord.y, 0.0f, 1.0f);
                if (uniforms.varyingTexuture.find(UNIFORM_TEXTURE) !=
                    uniforms.varyingTexuture.end()) {
                    auto& textureId = uniforms.varyingTexuture[UNIFORM_TEXTURE];
                    auto textureOpt = textureStorage.GetById(textureId);
                    if (textureOpt.has_value()) {
                        auto& texture = textureOpt.value();
                        fragColor *= TextureSample(texture, texcoord);
                    }
                };
                return fragColor;
            };
    }

    void OnRender() override {
        auto clearColor = Vec4{0.2, 0.2, 0.2, 1.0};
        renderer_->Clear(clearColor);
        renderer_->ClearDepth();

        auto model = CreateTranslate(Vec3{0.0, 0.0, -4.0}) *
                     CreateEularRotate_y(Radians(rotation_)) *
                     CreateScale(Vec3{2.0, 2.0, 2.0});
        for (auto& data : vertexDatas_) {
            // set data into uniform
            auto& uniforms = renderer_->GetUniforms();
            if (data.mtllib.has_value() && data.material.has_value()) {
                auto& mtllib = mtllibs_[data.mtllib.value()];
                auto& materialIndex = data.material.value();
                if (mtllib.materials.find(materialIndex) !=
                    mtllib.materials.end()) {
                    auto& material = mtllib.materials[materialIndex];
                    if (material.ambient.has_value()) {
                        auto& ambient = material.ambient.value();
                        uniforms.varyingVec4[UNIFORM_COLOR] =
                            Vec4::FromVec3(ambient, 1.0f);
                        // uniforms.varyingVec4.insert(std::make_pair(
                        //     UNIFORM_COLOR, Vec4::FromVec3(ambient, 1.0f)));
                    }
                    if (material.textureMaps.diffuse.has_value()) {
                        auto& diffuseTexture =
                            material.textureMaps.diffuse.value();
                        uniforms.varyingTexuture[UNIFORM_TEXTURE] =
                            textureStorage_.GetId(diffuseTexture).value();
                        // uniforms.varyingTexuture.insert(std::make_pair(
                        //     UNIFORM_TEXTURE,
                        //     textureStorage_.GetId(diffuseTexture).value()));
                    }
                }
            }

            renderer_->DrawTriangle(model, data.vertices, textureStorage_);
        }

        rotation_ += 1.0f;
        SwapBuffer(renderer_->GetSurface());
    }
};

int main(int argv, char** args) {
    RedBirdApp app;
    app.Run();
    return 0;
}