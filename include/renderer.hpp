#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <unordered_map>

#include "SDL.h"
#include "SDL_image.h"

#include "math.hpp"



struct Rect {
  Vec2 pos;
  Vec2 size;
};

inline std::ostream& operator<<(std::ostream& o, const Rect& r) {
  printf("Rect(%f, %f, %f, %f)", r.pos.x, r.pos.y, r.size.w, r.size.h);
  return o;
}

inline real Radians(real degrees) {
  return degrees * M_PI / 180.0f;
}

inline real Degrees(real radians) {
  return radians * 180.0f / M_PI;
}

inline bool IsPointInRect(const Vec2 &p, const Rect &r) {
  return p.x >= r.pos.x && p.x <= r.pos.x + r.size.w &&
         p.y >= r.pos.y && p.y <= r.pos.y + r.size.h;
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

inline Vec3 Barycentric(const Vec2& v1, const Vec2& v2, const Vec2& v3, const Vec2& p) {
  Vec3 result;
  Vec3 c1{v1.x - v2.x, v1.x - v3.x, p.x - v1.x},
       c2{v1.y - v2.y, v1.y - v3.y, p.y - v1.y};
  result = Cross(c1, c2);
  if (result.z == 0) {
    // (-1, -1, -1) means a invalid condition, should discard this point
    return Vec3{-1, -1, -1};
  }
  return Vec3{1 - result.x / result.z - result.y / result.z,
              result.x / result.z,
              result.y / result.z};
}

inline Rect GetTriangleAABB(const Vec2& v1, const Vec2& v2, const Vec2& v3) {
  int x1 = std::min({v1.x, v2.x, v3.x}),
      y1 = std::min({v1.y, v2.y, v3.y}),
      x2 = std::max({v1.x, v2.x, v3.x}),
      y2 = std::max({v1.y, v2.y, v3.y});
  return Rect{Vec2{real(x1), real(y1)},
              Vec2{real(x2 - x1), real(y2 - y1)}};
}

template <typename T>
inline int Sign(T value) {
  return value > 0 ? 1 : (value == 0 ? 0 : -1);
}

inline Mat44 CreateOrtho(real l, real r, real b, real t, real n, real f) {
  return Mat44{
    2 / (r - l),           0,           0, -(l + r) / (r - l),
              0, 2 / (t - b),           0, -(t + b) / (t - b),
              0,           0, 2 / (n - f), -(n + f) / (n - f),
              0,           0,           0,            1,
  };
}

inline Mat44 CreatePersp(real fov, real aspect, real near, real far) {
  real tanHalf = std::tan(fov * 0.5);
  char sign = Sign(near);
  return Mat44{
     sign / (aspect * tanHalf),            0,                            0,                             0,
                            0, sign / tanHalf,                           0,                             0,
                            0,              0,           (near + far) / (near - far),           2 * near * far / (far - near),
                            0,              0,                           1,                             0,
  };
}

inline Mat44 CreateTranslate(real x, real y, real z) {
  return Mat44{
    1, 0, 0, x,
    0, 1, 0, y,
    0, 0, 1, z,
    0, 0, 0, 1
  };
}

inline Mat44 CreateRotate(real x, real y, real z) {
  real sinx = std::sin(x),
       cosx = std::cos(x),
       siny = std::sin(y),
       cosy = std::cos(y),
       sinz = std::sin(z),
       cosz = std::cos(z);
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
}

inline Mat44 CreateScale(real x, real y, real z) {
  return Mat44{
    x, 0, 0, 0,
    0, y, 0, 0,
    0, 0, z, 0,
    0, 0, 0, 1
  };
}


/***********************************
 * Surface - use this to draw points
 ***********************************/

class Surface final {
public:
  Surface(const char *filename) {
    surface_ = SDL_ConvertSurfaceFormat(IMG_Load(filename), SDL_PIXELFORMAT_RGBA32, 0);
    if (!surface_) {
      Log("load %s failed", filename);
      SDL_Log("load %s failed", filename);
    }
  }

  Surface(int w, int h) {
    surface_ =
        SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA32);
    if (!surface_) {
      Log("Create Surface failed: %s", SDL_GetError());
    }
  }

  Surface(const Surface &) = delete;

  ~Surface() { SDL_FreeSurface(surface_); }

  Surface &operator=(const Surface &) = delete;

  inline int Width() const { return surface_->w; }
  inline int Height() const { return surface_->h; }
  inline Vec2 Size() const { return {real(surface_->w), real(surface_->h)}; }
  void PutPixel(int x, int y, const Color4 &color) {
    *getPixel(x, y) = SDL_MapRGBA(surface_->format, color.r * 255,
                                  color.g * 255, color.b * 255, color.a * 255);
  }

  Color4 GetPixel(int x, int y) const {
    const Uint32 *color = getPixel(x, y);
    Uint8 r, g, b, a;
    SDL_GetRGBA(*color, surface_->format, &r, &g, &b, &a);
    return Color4{r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
  }

  inline void Clear(const Color4 &color) {
    SDL_FillRect(surface_, nullptr,
                 SDL_MapRGBA(surface_->format, color.r * 255, color.g * 255,
                             color.b * 255, color.a * 255));
  }

  void Save(const char *filename) {
    auto surface = SDL_ConvertSurfaceFormat(surface_, SDL_PIXELFORMAT_RGB24, 0);
    if (surface) {
      SDL_SaveBMP(surface, filename);
      SDL_FreeSurface(surface);
    } else {
      Log("can't convert surface: %s", SDL_GetError());
    }
  }

  SDL_Surface* GetRaw() const { return surface_; }

private:
  SDL_Surface *surface_;

  Uint32 *getPixel(int x, int y) const {
    Uint8 *ptr = (Uint8 *)surface_->pixels;
    return (Uint32 *)(ptr + y * surface_->pitch +
                      x * surface_->format->BytesPerPixel);
  }
};

/***********************************
 * Bresenham
 ***********************************/
class Bresenham {
public:
  Bresenham(const Vec2 &p1, const Vec2 &p2) : p1_(p1), p2_(p2) {
    dx_ = 2 * abs(p1.x - p2.x);
    dy_ = 2 * abs(p1.y - p2.y);
    sx_ = p1.x < p2.x ? 1 : p1.x == p2.x ? 0 : -1;
    sy_ = p1.y < p2.y ? 1 : p1.y == p2.y ? 0 : -1;
    err_ = dx_ >= dy_ ? -dx_ / 2 : -dy_ / 2;
  }

  inline const Vec2 &CurPoint() const { return p1_; }

  inline bool IsFinished() const { return p1_ == p2_; }

  void Step() {
    if (!IsFinished()) {
      if (dx_ >= dy_) {
        p1_.x += sx_;
        err_ += dy_;
        if (err_ >= 0) {
          p1_.y += sy_;
          err_ -= dx_;
        }
      } else {
        p1_.y += sy_;
        err_ += dx_;
        if (err_ >= 0) {
          p1_.x += sx_;
          err_ -= dy_;
        }
      }
    }
  }

private:
  Vec2 p1_;
  Vec2 p2_;
  int dx_;
  int dy_;
  int sx_;
  int sy_;
  int err_;
};

/***********************************
 * Shader
 ***********************************/
struct ShaderContext {
  std::unordered_map<int, real> varyingFloat;
  std::unordered_map<int, Vec2> varyingVec2;
  std::unordered_map<int, Vec3> varyingVec3;
  std::unordered_map<int, Vec4> varyingVec4;

  void Clear() {
    varyingFloat.clear();
    varyingVec2.clear();
    varyingVec3.clear();
    varyingVec4.clear();
  }
};

using VertexShader = std::function<Vec4(int index, ShaderContext &output)>;
using FragmentShader = std::function<Vec4(ShaderContext &input)>;

// like opengl shader function `texture`
inline Color4 TextureSample(const Surface* const surface, Vec2 texcoord) {
  texcoord.x = Clamp<real>(texcoord.x, 0, 1);
  texcoord.y = Clamp<real>(texcoord.y, 0, 1);

  // nearest sample
  return surface->GetPixel(texcoord.x * surface->Width(), texcoord.y * surface->Height());
}

/***********************************
 * Buffer2D
 ***********************************/
class Buffer2D {
public:
  Buffer2D(int w, int h): w_(w), h_(h) {
    data_ = new real[w * h];
    Fill(0);
  }

  void Fill(real value) {
    for (int i = 0; i < w_ * h_; i++) {
      data_[i] = value;
    }
  }

  real& Get(int x, int y) {
    return data_[x * h_ + y];
  }

  real Get(int x, int y) const {
    return data_[x * h_ + y];
  }

  void Set(int x, int y, real value) {
    data_[x * h_ + y] = value;
  }

  int Width() const { return w_; }
  int Height() const { return h_; }

  ~Buffer2D() {
    delete[] data_;
  }

private:
  real* data_;
  int w_;
  int h_;
};

// a small help function to output Buffer2D
inline std::shared_ptr<Surface> Buffer2D2Surface(const Buffer2D& buf) {
  Surface* surface = new Surface(buf.Width(), buf.Height());
  for (int i = 0; i < buf.Width(); i++) {
    for (int j = 0; j < buf.Height(); j++) {
      const real& c = buf.Get(i, j);
      surface->PutPixel(i, j, Color4{c, c, c, 1});
    }
  }
  return std::shared_ptr<Surface>(surface);
}

/***********************************
 * Renderer
 ***********************************/

enum FaceCull {
  CW = 1,
  CCW,
};

class Renderer final {
public:
  static void Init() {
    IMG_Init(IMG_INIT_PNG|IMG_INIT_JPG);
  }

  static void Quit() {
    IMG_Quit();
  }

  Renderer(int w, int h)
      : drawColor_{0, 0, 0, 0} {
    framebuffer_.reset(new Surface(w, h));
    depthBuffer_ = new Buffer2D(w, h);
  }

  ~Renderer() {
    delete depthBuffer_;
  }

  void SetDrawColor(const Color4 &c) { drawColor_ = c; }
  void SetClearColor(const Color4 &c) { clearColor_ = c; }

  std::shared_ptr<Surface> GetFramebuffer() { return framebuffer_; }

  void SetFaceCull(FaceCull fc) { faceCull_ = fc; }

  void Clear() {
    framebuffer_->Clear(clearColor_);
    depthBuffer_->Fill(0);
  }

  void DrawPixel(int x, int y) {
    if (IsPointInRect(Vec2{real(x), real(y)},
                      Rect{Vec2{0, 0}, framebuffer_->Size()})) {
      framebuffer_->PutPixel(x, y, drawColor_);
    }
  }

  void SetViewport(int x, int y, int w, int h) {
    viewport_ = Mat44::Zeros();
    viewport_.Set(0, 0, w / 2.0f);
    viewport_.Set(1, 1, h / 2.0f);
    viewport_.Set(3, 0, w / 2.0f + x);
    viewport_.Set(3, 1, h / 2.0f + y);
    viewport_.Set(2, 2, 0.5);
    viewport_.Set(3, 2, 1);
    viewport_.Set(3, 3, 1);
  }

  const Mat44& GetViewport() const {
    return viewport_;
  }

  void DrawLine(int x1, int y1, int x2, int y2) {
    Bresenham bresenham(Vec2{real(x1), real(y1)}, Vec2{real(x2), real(y2)});
    while (!bresenham.IsFinished()) {
      DrawPixel(bresenham.CurPoint().x, bresenham.CurPoint().y);
      bresenham.Step();
    }
    DrawPixel(bresenham.CurPoint().x, bresenham.CurPoint().y);
  }

  void SetVertexShader(VertexShader shader) { vertexShader_ = shader; }
  void SetFragmentShader(FragmentShader shader) { fragmentShader_ = shader; }

  void Save(const char *filename) { framebuffer_->Save(filename); }

  void SaveDepthBuf(const char* filename) {
    auto surface = Buffer2D2Surface(*depthBuffer_);
    surface->Save(filename);
  }

  void EnableFaceCull(bool e) { enableFaceCull_ = e; }
  void EnableDepthTest(bool e) { enableDepthTest_ = e; }

  bool DrawPrimitive() {
    if (!vertexShader_) {
      return false;
    }

    for (int i = 0; i < 3; i++) {
      Vertex& vertex = vertices_[i];

      // 1. clear shader context
      vertex.context.Clear();

      // 2. run Vertex Shader
      vertex.pos = vertexShader_(i, vertices_[i].context);

      vertex.rhw = 1.0 / (vertex.pos.w == 0 ? 1e-5 : vertex.pos.w);
    }

    // 3. clipping, if AABB not intersect with screen, clip it 
    for (int i = 0; i < 3; i++) {
      real absw = std::abs(vertices_[i].pos.w);
      if (vertices_[i].pos.x < -absw || vertices_[i].pos.x > absw ||
          vertices_[i].pos.y < -absw || vertices_[i].pos.y > absw) {
        return false;
      }
    }

    // 4. face culling, cull the CCW face
    if (enableFaceCull_) {
      real result = Cross(Vec<2>(vertices_[1].pos - vertices_[0].pos),
                          Vec<2>(vertices_[2].pos - vertices_[1].pos));

      if (faceCull_ == CCW && result >= 0) {
        return false; 
      } else if (faceCull_ == CW && result <= 0) {
        return false;
      }
    }

    for (auto& vertex : vertices_) {
      // 4. perspective divide
      vertex.pos *= vertex.rhw;

      // 5. viewport transform and prepare to step into rasterization
      vertex.spf = Vec<3>(viewport_ * vertex.pos);

      vertex.spi.x = int(vertex.spf.x + 0.5f);
      vertex.spi.y = int(vertex.spf.y + 0.5f);
    }

    // small optimization, if triangle area == 0, quit
    if (Cross(vertices_[0].spi - vertices_[1].spi,
              vertices_[0].spi - vertices_[2].spi) == 0) {
      return false;
    }

    // calculate bounding box for rasterization
    Rect boundingRect = GetTriangleAABB(vertices_[0].spi, vertices_[1].spi, vertices_[2].spi);
    int minX = std::max<int>(boundingRect.pos.x, 0),
        minY = std::max<int>(boundingRect.pos.y, 0),
        maxX = std::min<int>(boundingRect.pos.x + boundingRect.size.w, framebuffer_->Width()),
        maxY = std::min<int>(boundingRect.pos.y + boundingRect.size.h, framebuffer_->Height());

    // 7. rasterization
    for (int i = minX; i < maxX; i++) {
      for (int j = minY; j < maxY; j++) {
        Vec2 p{i + 0.5f, j+ 0.5f};
        if (!IsPointInRect(p, boundingRect)) {
          continue;
        }

        // 7.1 barycentric calculate
        Vec3 barycentric = Barycentric(vertices_[0].spi,
                                       vertices_[1].spi,
                                       vertices_[2].spi,
                                       p);

        real rhw = vertices_[0].rhw * barycentric.alpha + vertices_[1].rhw * barycentric.beta + vertices_[2].rhw * barycentric.gamma;
        float w = 1.0f / ((rhw != 0.0f)? rhw : 1.0f);

        barycentric.alpha *= vertices_[0].rhw * w;
        barycentric.beta *= vertices_[1].rhw * w;
        barycentric.gamma *= vertices_[2].rhw * w;

        if (barycentric.alpha < 0 && barycentric.beta < 0 && barycentric.gamma < 0) {
          return false;
        }

        if (barycentric.alpha < 0 || barycentric.beta < 0 || barycentric.gamma < 0) {
          continue;
        }

        // get z, and make it into [0, 1]
        real z = 1.0 / (barycentric.alpha / vertices_[0].spf.z + barycentric.beta / vertices_[1].spf.z + barycentric.gamma / vertices_[2].spf.z);

        // 7.2 update depth buffer( camera look at -z, but depth buffer store positive value, so we take the opposite of 1.0 / rhw)
        if (enableDepthTest_) {
          if (z <= depthBuffer_->Get(i, j)) {
            continue;
          }
          depthBuffer_->Set(i, j, z);
        }


        // 7.3 interpolation other varying properties 
        ShaderContext input;
        ShaderContext& i0 = vertices_[0].context,
                       i1 = vertices_[1].context,
                       i2 = vertices_[2].context;
        for (auto& [key, value] : i0.varyingFloat) {
          input.varyingFloat[key] = i0.varyingFloat[key] * barycentric.alpha +
                                    i1.varyingFloat[key] * barycentric.beta +
                                    i2.varyingFloat[key] * barycentric.gamma;
        }
        for (auto& [key, value] : i0.varyingVec2) {
          input.varyingVec2[key] = i0.varyingVec2[key] * barycentric.alpha +
                                    i1.varyingVec2[key] * barycentric.beta +
                                    i2.varyingVec2[key] * barycentric.gamma;
        }
        for (auto& [key, value] : i0.varyingVec3) {
          input.varyingVec3[key] = i0.varyingVec3[key] * barycentric.alpha +
                                    i1.varyingVec3[key] * barycentric.beta +
                                    i2.varyingVec3[key] * barycentric.gamma;
        }
        for (auto& [key, value] : i0.varyingVec4) {
          input.varyingVec4[key] = i0.varyingVec4[key] * barycentric.alpha +
                                    i1.varyingVec4[key] * barycentric.beta +
                                    i2.varyingVec4[key] * barycentric.gamma;
        }

        // 8. run Fragment Shader 
        Vec4 color{0, 0, 0, 0};
        if (fragmentShader_) {
          color = fragmentShader_(input);
          framebuffer_->PutPixel(i, j, color);
        }
      }
    }
    return true;
  }

private:
  struct Vertex {
    ShaderContext context;
    real rhw;
    Vec4 pos;
    Vec3 spf;
    Vec2 spi;
  };

  Vertex vertices_[3];
  std::shared_ptr<Surface> framebuffer_;
  Color4 drawColor_;
  Color4 clearColor_;

  VertexShader vertexShader_ = nullptr;
  FragmentShader fragmentShader_ = nullptr;
  Buffer2D* depthBuffer_ = nullptr;
  Mat44 viewport_;
  FaceCull faceCull_ = CCW;
  bool enableFaceCull_ = false;
  bool enableDepthTest_ = true;
};