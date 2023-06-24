#pragma once

#include <cassert>
#include <tuple>
#include <vector>

#include "math.hpp"
#include "obj_loader.hpp"
namespace model {
class Vertex {
   public:
    Vec3 position;
    Vec3 normal;
    Vec2 texcoord;
    Vec4 color;

    Vertex(Vec3 position, Vec3 normal, Vec2 texcoord, Vec4 color)
        : position(position),
          normal(normal),
          texcoord(texcoord),
          color(color) {}
};

enum PreOperation {
    None = 0x00,
    RecalcNormal = 0x01,
};

class Mesh {
   public:
    std::vector<Vertex> vertices;
    std::optional<std::string> name;
    std::optional<uint32_t> mtllib;
    std::optional<std::string> material;

    Mesh(std::optional<std::string> name) : name(name) {}
    // Mesh(std::vector<Vertex> vertices, std::optional<std::string> name,
    //      std::optional<uint32_t> mtllib, std::optional<std::string> material)
    //     : vertices(vertices), name(name), mtllib(mtllib), material(material)
    //     {}
};

std::optional<std::tuple<std::vector<Mesh>, std::vector<objloader::Mtllib>>>
LoadFromFile(std::string& filename, PreOperation preOperation) {
    std::vector<Mesh> meshes;
    auto sceneOpt = objloader::LoadFromFile(filename);
    if (sceneOpt.has_value()) {
        auto& scene = sceneOpt.value();
        for (auto model : scene.models) {
            Mesh mesh = Mesh(model.name);
            for (auto face : model.faces) {
                for (auto vtx : face.vertices) {
                    auto positon = scene.vertices[vtx.vertex];
                    auto normal = vtx.normal.has_value()
                                      ? scene.normals[vtx.normal.value()]
                                      : Vec3::Zero;
                    auto texcoord = vtx.texcoord.has_value()
                                        ? scene.texcoords[vtx.texcoord.value()]
                                        : Vec2::Zero;
                    mesh.vertices.push_back(Vertex{positon, normal, texcoord,
                                                   Vec4{1.0, 1.0, 1.0, 1.0}});
                }
            }
            mesh.material = model.material;
            mesh.mtllib = model.mtllib;
            meshes.push_back(mesh);
        }
        if (((uint8_t)preOperation & (uint8_t)PreOperation::RecalcNormal) !=
            0) {
            for (auto mesh : meshes) {
                assert(mesh.vertices.size() % 3 == 0);
                for (int i = 0; i < mesh.vertices.size() / 3; i++) {
                    auto& v1 = mesh.vertices[i * 3];
                    auto& v2 = mesh.vertices[i * 3 + 1];
                    auto& v3 = mesh.vertices[i * 3 + 2];
                    auto norm = Normalize(Cross((v3.position - v2.position),
                                                (v2.position - v1.position)));
                    v1.normal = norm;
                    v2.normal = norm;
                    v3.normal = norm;
                }
            }
        }
        return std::tuple<std::vector<Mesh>, std::vector<objloader::Mtllib>>{
            meshes, scene.materials};
    }
    return std::nullopt;
}

}  // namespace model