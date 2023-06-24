#pragma once

#include <map>
#include <optional>

#include "SDL.h"
#include "SDL_image.h"
#include "math.hpp"

class Texture {
   private:
    SDL_Surface *surface_;

    Uint32 *getPixel(int x, int y) const {
        Uint8 *ptr = (Uint8 *)surface_->pixels;
        return (Uint32 *)(ptr + y * surface_->pitch +
                          x * surface_->format->BytesPerPixel);
    }

    void load(const char *filename) {
        surface_ = SDL_ConvertSurfaceFormat(IMG_Load(filename),
                                            SDL_PIXELFORMAT_RGBA32, 0);
        if (!surface_) {
            SDL_Log("load %s failed", filename);
        }
    }

   public:
    uint32_t id;
    std::string name;

    Texture(const char *filename, uint32_t id, std::string name)
        : id(id), name(name) {
        load(filename);
    }

    uint32_t Width() { return surface_->w; }

    uint32_t Height() { return surface_->h; }

    Color4 GetPixel(int x, int y) const {
        const Uint32 *color = getPixel(x, y);
        Uint8 r, g, b, a;
        SDL_GetRGBA(*color, surface_->format, &r, &g, &b, &a);
        return Color4{r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
    }
};

class TextureStorage {
   private:
    uint32_t cur_id_;
    std::map<uint32_t, Texture> images_;
    std::map<std::string, uint32_t> name_id_map_;

   public:
    void load(const char *filename, std::string name) {
        auto id = cur_id_;
        cur_id_++;
        images_.insert(std::make_pair<>(id, Texture{filename, id, name}));
        name_id_map_.insert(std::make_pair<>(name, id));
    }

    std::optional<Texture> GetById(uint32_t id) {
        if (images_.find(id) == images_.end()) {
            return std::nullopt;
        } else {
            return images_.at(id);
        }
    }

    std::optional<Texture> GetByName(std::string name) {
        if (name_id_map_.find(name) == name_id_map_.end()) {
            return std::nullopt;
        } else {
            return images_.at(name_id_map_.at(name));
        }
    }

    std::optional<uint32_t> GetId(std::string name) {
        if (name_id_map_.find(name) == name_id_map_.end()) {
            return std::nullopt;
        } else {
            return name_id_map_.at(name);
        }
    }
};
