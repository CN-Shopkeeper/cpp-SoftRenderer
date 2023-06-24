#pragma once
#include "shader.hpp"

class Line {
   private:
    Vertex generateStep(Vertex start, Vertex end) {
        float dx = std::abs(end.position.x - start.position.x);
        float dy = std::abs(end.position.y - start.position.y);
        float t = dx >= dy ? 1.0 / std::abs(end.position.x - start.position.x)
                           : std::abs(end.position.y - start.position.y);
        step = Vertex{(end.position - start.position) * t,
                      InterpAttributes(
                          start.attributes, end.attributes,
                          [](float value1, float value2, float t) {
                              return (value2 - value1) * t;
                          },
                          t)};
        return step;
    }

   public:
    Vertex start;
    Vertex end;
    Vertex step;

    Line(Vertex start, Vertex end)
        : start(start), end(end), step(generateStep(start, end)) {}
};