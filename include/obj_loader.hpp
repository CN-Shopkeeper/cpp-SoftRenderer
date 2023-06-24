#pragma once

#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <map>
#include <optional>
#include <queue>
#include <string>
#include <vector>

#include "math.hpp"
namespace objloader {

class FileContent {
   public:
    std::vector<std::string> lines;

    FileContent(std::vector<std::string> lines) : lines(lines) {}

    static std::optional<FileContent> fromFile(
        std::filesystem::path &filename) {
        std::ifstream file(filename);
        if (!file) {
            return std::nullopt;
        }
        std::string line;
        std::vector<std::string> lines;
        while (std::getline(file, line)) {
            lines.push_back(line);
            line.clear();
        }
        return FileContent(lines);
    }
};

class Vertex {
   public:
    uint32_t vertex;
    std::optional<uint32_t> normal;
    std::optional<uint32_t> texcoord;
    Vertex(uint32_t vertex, std::optional<uint32_t> normal,
           std::optional<uint32_t> texcoord)
        : vertex(vertex), normal(normal), texcoord(texcoord) {}
};

class Face {
   public:
    std::vector<Vertex> vertices;
    Face(std::vector<Vertex> vertices) : vertices(vertices) {}
};

class Model {
   public:
    std::vector<Face> faces;
    std::string name;
    std::optional<uint32_t> mtllib;
    std::optional<std::string> material;
    uint8_t smoothShade;

    Model(std::vector<Face> faces, std::string name,
          std::optional<uint32_t> mtllib, std::optional<std::string> material,
          uint8_t smmothShade)
        : faces(faces),
          name(name),
          mtllib(mtllib),
          material(material),
          smoothShade(smmothShade) {}
};

class MtlTextureMaps {
   public:
    std::optional<std::string> ambient;
    std::optional<std::string> diffuse;
    std::optional<std::string> speculatColor;
    std::optional<std::string> specularHighlight;
    std::optional<std::string> alpha;
    std::optional<std::string> refl;
    std::optional<std::string> bump;

    MtlTextureMaps(std::optional<std::string> ambient,
                   std::optional<std::string> diffuse,
                   std::optional<std::string> speculatColor,
                   std::optional<std::string> specularHighlight,
                   std::optional<std::string> alpha,
                   std::optional<std::string> refl,
                   std::optional<std::string> bump)
        : ambient(ambient),
          diffuse(diffuse),
          speculatColor(speculatColor),
          specularHighlight(specularHighlight),
          alpha(alpha),
          refl(refl),
          bump(bump) {}
};

class Material {
   public:
    std::string name;
    std::optional<Vec3> ambient;
    std::optional<Vec3> diffuse;
    std::optional<Vec3> specular;
    std::optional<Vec3> emissiveCoefficient;
    std::optional<float> speculatExponent;
    std::optional<float> dissolve;
    std::optional<Vec3> transmissionFilter;
    std::optional<float> opticalDensity;
    std::optional<uint8_t> illum;

    MtlTextureMaps textureMaps;

    Material(std::string name)
        : name(name),
          ambient(std::nullopt),
          diffuse(std::nullopt),
          specular(std::nullopt),
          emissiveCoefficient(std::nullopt),
          speculatExponent(std::nullopt),
          dissolve(std::nullopt),
          transmissionFilter(std::nullopt),
          opticalDensity(std::nullopt),
          illum(std::nullopt),
          textureMaps(MtlTextureMaps(std::nullopt, std::nullopt, std::nullopt,
                                     std::nullopt, std::nullopt, std::nullopt,
                                     std::nullopt)) {}
};

class Mtllib {
   public:
    std::map<std::string, Material> materials;
    Mtllib(std::map<std::string, Material> materials) : materials(materials) {}
};

class SceneData {
   public:
    std::vector<Vec3> vertices;
    std::vector<Vec3> normals;
    std::vector<Vec2> texcoords;
    std::vector<Mtllib> materials;
    std::vector<Model> models;
};

// 去掉字符串的空格并保存为vector
std::vector<std::string> SplitWhiteSpace(std::string &str) {
    auto buffer = std::istringstream(str);
    return std::vector<std::string>{std::istream_iterator<std::string>(buffer),
                                    {}};
}

std::queue<std::string> SplitWhiteSpaceQueue(std::string &str) {
    auto vec = SplitWhiteSpace(str);
    return std::queue<std::string>{
        std::deque<std::string>{vec.begin(), vec.end()}};
}

class TokenType {
   public:
    enum Type { Token, Nextline, Eof };
    Type type;
    TokenType(Type type) : type(type) {}
    TokenType(std::string str) : type(Type::Token), str(str) {}
    std::string str;
};

class TokenRequester {
   private:
    TokenRequester(FileContent &content)
        : content(content),
          tokens(SplitWhiteSpaceQueue(content.lines[0])),
          line(0) {}

   public:
    FileContent &content;
    std::queue<std::string> tokens;
    uint64_t line;

    static std::optional<TokenRequester> New(FileContent &content) {
        if (content.lines.empty()) {
            return std::nullopt;
        }
        return TokenRequester(content);
    }

    TokenType Request() {
        if (tokens.empty()) {
            line++;
            if (line >= content.lines.size()) {
                return TokenType(TokenType::Type::Eof);
            } else {
                tokens = SplitWhiteSpaceQueue(content.lines[line]);
                return TokenType(TokenType::Type::Nextline);
            }
        } else {
            auto token = tokens.front();
            tokens.pop();
            return TokenType(token);
        }
    }
};

// 忽略至新的一行
// cpp的宏不支持正则表达式，所以用函数代替
inline void IgnoreUntil(TokenType &token, TokenRequester &requester) {
    while (token.type != TokenType::Nextline && token.type != TokenType::Eof) {
        token = requester.Request();
    }
}

inline std::optional<std::string> ParseAsString(TokenType &token,
                                                TokenRequester &requester) {
    token = requester.Request();
    if (token.type == TokenType::Type::Token) {
        std::string ret = token.str;
        return ret;
    } else {
        return std::nullopt;
    }
}

template <size_t Dim>
inline std::optional<Vector<Dim>> parseAsVector(TokenType &token,
                                                TokenRequester &requester) {
    Vector<Dim> v;
    for (int i = 0; i < Dim; i++) {
        token = requester.Request();
        if (token.type == TokenType::Type::Token) {
            std::string num = token.str;
            v.data[i] = ::atof(num.c_str());
        } else {
            return std::nullopt;
        }
    }
    token = requester.Request();
    return v;
}

template <typename T>
inline std::optional<T> parseAsT(TokenType &token, TokenRequester &requester) {
    token = requester.Request();
    if (token.type == TokenType::Type::Token) {
        std::string num = ::atof(num.c_str());
        return (T)ret;
    } else {
        return std::nullopt;
    }
}

std::vector<std::string> split(std::string str, std::string delimiter) {
    std::vector<std::string> ret;
    size_t pos = 0;
    std::string token;
    while ((pos = str.find(delimiter)) != std::string::npos) {
        token = str.substr(0, pos);
        ret.push_back(token);
        str.erase(0, pos + delimiter.length());
    }
    return ret;
}

class ObjParser {
   public:
    SceneData scene;
    std::filesystem::path &dirpath;
    TokenRequester &tokenRequester;

    ObjParser(std::filesystem::path &dirpath, TokenRequester &tokenRequester)
        : dirpath(dirpath),
          tokenRequester(tokenRequester),
          scene(SceneData()) {}

    void parse() {
        auto token = tokenRequester.Request();
        bool parseFinish = false;
        while (!parseFinish) {
            switch (token.type) {
                case TokenType::Type::Token:
                    if (token.str == "#") {
                        IgnoreUntil(token, tokenRequester);
                    } else if (token.str == "g" || token.str == "o") {
                        auto nameOpt = ParseAsString(token, tokenRequester);
                        if (nameOpt.has_value()) {
                            scene.models.push_back(Model{
                                std::vector<Face>{}, nameOpt.value(),
                                scene.materials.empty()
                                    ? std::optional<uint32_t>(std::nullopt)
                                    : std::optional<uint32_t>(
                                          scene.materials.size() - 1),
                                std::optional<std::string>(std::nullopt), 0

                            });
                        }
                    } else if (token.str == "v") {
                        auto v3Opt = parseAsVector<3>(token, tokenRequester);
                        if (v3Opt.has_value()) {
                            scene.vertices.push_back(v3Opt.value());
                        }
                    } else if (token.str == "vt") {
                        auto v2Opt = parseAsVector<2>(token, tokenRequester);
                        if (v2Opt.has_value()) {
                            scene.texcoords.push_back(v2Opt.value());
                        }
                    } else if (token.str == "vn") {
                        auto v3Opt = parseAsVector<3>(token, tokenRequester);
                        if (v3Opt.has_value()) {
                            scene.normals.push_back(v3Opt.value());
                        }
                    } else if (token.str == "f") {
                        token = tokenRequester.Request();
                        std::vector<Vertex> vertices;
                        bool finish = false;
                        while (!finish) {
                            if (token.type == TokenType::Type::Token) {
                                auto indices = split(token.str, "/");
                                if (indices.size() == 3) {
                                    uint32_t vertex =
                                        ::atoi(indices[0].c_str()) - 1;
                                    std::optional<uint32_t> texcoord =
                                        indices[1].empty()
                                            ? std::optional<uint32_t>(
                                                  std::nullopt)
                                            : std::optional<uint32_t>(
                                                  ::atoi(indices[1].c_str()) -
                                                  1);
                                    std::optional<uint32_t> normal =
                                        indices[2].empty()
                                            ? std::optional<uint32_t>(
                                                  std::nullopt)
                                            : std::optional<uint32_t>(
                                                  ::atoi(indices[2].c_str()) -
                                                  1);
                                    vertices.push_back(
                                        Vertex{vertex, normal, texcoord});
                                }
                            } else {
                                // 吃掉换行
                                finish = true;
                            }
                            token = tokenRequester.Request();
                        }
                        scene.models.back().faces.push_back(Face{vertices});
                    } else if (token.str == "mtllib") {
                        token = tokenRequester.Request();
                        if (token.type == TokenType::Type::Token) {
                            auto parentPath = dirpath.parent_path();
                            auto filepath = parentPath.append(token.str);
                            auto fileContentOpt =
                                FileContent::fromFile(filepath);
                            if (fileContentOpt.has_value()) {
                                auto mtllibTokenRequester =
                                    TokenRequester::New(fileContentOpt.value());
                                if (mtllibTokenRequester.has_value()) {
                                    auto mtlibParser = MtllibParser(
                                        mtllibTokenRequester.value());
                                    scene.materials.push_back(
                                        mtlibParser.parse());
                                }
                            }
                            token = tokenRequester.Request();
                        }
                    } else if (token.str == "usemtl") {
                        auto nameOpt = ParseAsString(token, tokenRequester);
                        if (nameOpt.has_value()) {
                            scene.models.back().material = nameOpt.value();
                        }
                    } else if (token.str == "s") {
                        auto u8Opt = parseAsT<uint8_t>(token, tokenRequester);
                        if (u8Opt.has_value()) {
                            scene.models.back().smoothShade = u8Opt.value();
                        }
                    } else {
                        // 尝试跳到下一行
                        IgnoreUntil(token, tokenRequester);
                    }
                    break;
                case TokenType::Type::Eof:
                    parseFinish = true;
                    break;
                case TokenType::Type::Nextline:
                    token = tokenRequester.Request();
                    break;
                default:
                    break;
            }
        }
    }
};

class MtllibParser {
   public:
    TokenRequester &tokenRequester;
    MtllibParser(TokenRequester &tokenRequester)
        : tokenRequester(tokenRequester) {}
    Mtllib parse() {
        Mtllib mtllib = Mtllib{std::map<std::string, Material>{}};
        std::optional<Material> mtl = std::optional<Material>{std::nullopt};
        auto token = tokenRequester.Request();
        bool finish = false;
        while (!finish) {
            switch (token.type) {
                case TokenType::Type::Token:
                    if (token.str == "#") {
                        IgnoreUntil(token, tokenRequester);
                    } else if (token.str == "newmtl") {
                        if (mtl.has_value()) {
                            mtllib.materials.insert(
                                std::make_pair(mtl.value().name, mtl.value()));
                        }
                        auto strOpt = ParseAsString(token, tokenRequester);
                        if (strOpt.has_value()) {
                            mtl = std::optional<Material>(
                                Material(strOpt.value()));
                        }
                    } else if (token.str == "Ns") {
                        if (mtl.has_value()) {
                            mtl.value().speculatExponent =
                                parseAsT<float>(token, tokenRequester);
                        }
                    } else if (token.str == "Ka") {
                        if (mtl.has_value()) {
                            mtl.value().ambient =
                                parseAsVector<3>(token, tokenRequester);
                        }
                    } else if (token.str == "Kd") {
                        if (mtl.has_value()) {
                            mtl.value().diffuse =
                                parseAsVector<3>(token, tokenRequester);
                        }
                    } else if (token.str == "Ks") {
                        if (mtl.has_value()) {
                            mtl.value().specular =
                                parseAsVector<3>(token, tokenRequester);
                        }
                    } else if (token.str == "Ke") {
                        if (mtl.has_value()) {
                            mtl.value().emissiveCoefficient =
                                parseAsVector<3>(token, tokenRequester);
                        }
                    } else if (token.str == "Tf") {
                        if (mtl.has_value()) {
                            mtl.value().transmissionFilter =
                                parseAsVector<3>(token, tokenRequester);
                        }
                    } else if (token.str == "Ni") {
                        if (mtl.has_value()) {
                            mtl.value().opticalDensity =
                                parseAsT<float>(token, tokenRequester);
                        }
                    } else if (token.str == "d") {
                        if (mtl.has_value()) {
                            mtl.value().dissolve =
                                parseAsT<float>(token, tokenRequester);
                        }
                    } else if (token.str == "Tr") {
                        if (mtl.has_value()) {
                            auto fOpt = parseAsT<float>(token, tokenRequester);
                            if (fOpt.has_value()) {
                                mtl.value().dissolve = 1.0f - fOpt.value();
                            } else {
                                mtl.value().dissolve = std::nullopt;
                            }
                        }
                    } else if (token.str == "illum") {
                        if (mtl.has_value()) {
                            mtl.value().illum =
                                parseAsT<uint8_t>(token, tokenRequester);
                        }
                    } else if (token.str == "map_Ka") {
                        if (mtl.has_value()) {
                            mtl.value().textureMaps.ambient =
                                ParseAsString(token, tokenRequester);
                        }
                    } else if (token.str == "map_Kd") {
                        if (mtl.has_value()) {
                            mtl.value().textureMaps.diffuse =
                                ParseAsString(token, tokenRequester);
                        }
                    } else if (token.str == "map_Ks") {
                        if (mtl.has_value()) {
                            mtl.value().textureMaps.speculatColor =
                                ParseAsString(token, tokenRequester);
                        }
                    } else if (token.str == "map_Ns") {
                        if (mtl.has_value()) {
                            mtl.value().textureMaps.specularHighlight =
                                ParseAsString(token, tokenRequester);
                        }
                    } else if (token.str == "map_d") {
                        if (mtl.has_value()) {
                            mtl.value().textureMaps.alpha =
                                ParseAsString(token, tokenRequester);
                        }
                    } else if (token.str == "map_refl") {
                        if (mtl.has_value()) {
                            mtl.value().textureMaps.refl =
                                ParseAsString(token, tokenRequester);
                        }
                    } else if (token.str == "map_Bump") {
                        if (mtl.has_value()) {
                            mtl.value().textureMaps.bump =
                                ParseAsString(token, tokenRequester);
                        }
                    } else {
                        IgnoreUntil(token, tokenRequester);
                    }
                    break;
                case TokenType::Type::Eof:
                    if (mtl.has_value()) {
                        mtllib.materials.insert(
                            std::make_pair(mtl.value().name, mtl.value()));
                    }
                    mtl = std::nullopt;
                    finish = true;
                    break;
                case TokenType::Type::Nextline:
                    token = tokenRequester.Request();
                    break;
                default:
                    break;
            }
        }
        return mtllib;
    }
};

std::optional<SceneData> LoadFromFile(std::string &filename) {
    auto filepath = std::filesystem::path(filename);
    auto fcOpt = FileContent::fromFile(filepath);
    if (fcOpt.has_value()) {
        auto trOpt = TokenRequester::New(fcOpt.value());
        if (trOpt.has_value()) {
            auto parser = ObjParser(filepath, trOpt.value());
            parser.parse();
            return parser.scene;
        }
        return std::nullopt;
    }
    return std::nullopt;
}

}  // namespace objloader