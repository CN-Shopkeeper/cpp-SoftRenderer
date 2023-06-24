#pragma once

#include <algorithm>
#include <optional>
#include <tuple>

#include "math.hpp"
#include "shader.hpp"

class Edge {
   public:
    Vertex v1;
    Vertex v2;
    Edge(Vertex v1, Vertex v2) : v1(v1), v2(v2) {}
};

class Trapezoid {
   public:
    float top;
    float bottom;
    Edge left;
    Edge right;

    Trapezoid(float top, float bottom, Edge left, Edge right)
        : top(top), bottom(bottom), left(left), right(right) {}

    std::tuple<std::optional<Trapezoid>, std::optional<Trapezoid>> FromTriangle(
        Vertex (&vertices)[3]) {
        std::sort(std::begin(vertices), std::end(vertices),
                  [](const Vertex& left, const Vertex& right) {
                      return left.position.y < right.position.y;
                  });
        //   三点共线
        if ((vertices[0].position.x == vertices[1].position.x &&
             vertices[0].position.x == vertices[2].position.x) ||
            (vertices[0].position.y == vertices[1].position.y &&
             vertices[0].position.y == vertices[2].position.y)) {
            return {std::nullopt, std::nullopt};
        }
        // 已经是Trapezoid
        if (vertices[0].position.y == vertices[1].position.y) {
            if (vertices[0].position.x > vertices[1].position.x) {
                std::swap(vertices[0], vertices[1]);
            }
            Trapezoid trap = Trapezoid(
                vertices[0].position.y, vertices[2].position.y,
                Edge(vertices[0], vertices[2]), Edge(vertices[1], vertices[2]));
            return {trap, std::nullopt};
        }
        // 已经是Trapezoid
        if (vertices[1].position.y == vertices[2].position.y) {
            if (vertices[1].position.x > vertices[2].position.x) {
                std::swap(vertices[1], vertices[2]);
            }
            Trapezoid trap = Trapezoid(
                vertices[0].position.y, vertices[2].position.y,
                Edge(vertices[0], vertices[2]), Edge(vertices[0], vertices[2]));
            return {trap, std::nullopt};
        }

        // 一般情况，进行切割
        auto x = (vertices[1].position.y - vertices[0].position.y) /
                     (vertices[2].position.y - vertices[0].position.y) *
                     (vertices[2].position.x - vertices[0].position.x) +
                 vertices[0].position.x;
        if (x > vertices[1].position.x) {
            Trapezoid trap1 = Trapezoid(
                vertices[0].position.y, vertices[1].position.y,
                Edge(vertices[0], vertices[1]), Edge(vertices[0], vertices[2]));
            Trapezoid trap2 = Trapezoid(
                vertices[1].position.y, vertices[2].position.y,
                Edge(vertices[1], vertices[2]), Edge(vertices[0], vertices[2]));
            return {trap1, trap2};
        } else {
            Trapezoid trap1 = Trapezoid(
                vertices[0].position.y, vertices[1].position.y,
                Edge(vertices[0], vertices[2]), Edge(vertices[0], vertices[1]));
            Trapezoid trap2 = Trapezoid(
                vertices[1].position.y, vertices[2].position.y,
                Edge(vertices[0], vertices[2]), Edge(vertices[1], vertices[2]));
            return {trap1, trap2};
        }
    }
};

class Scanline {
   public:
    Vertex vertex;
    Vertex step;
    float y;
    float width;

    // 从Trapezoid中获取扫描线
    Scanline FromTrapezoid(Trapezoid& trap, float init_y) {
        auto t1 = (init_y - trap.left.v1.position.y) /
                  (trap.left.v2.position.y - trap.left.v1.position.y);
        auto t2 = (init_y - trap.right.v1.position.y) /
                  (trap.right.v2.position.y - trap.right.v1.position.y);
        auto vertex_left = LerpVertex(trap.left.v1, trap.left.v2, t1);
        auto vertex_right = LerpVertex(trap.right.v1, trap.right.v2, t2);
        auto width = vertex_right.position.x - vertex_left.position.x;
        auto rh_width = 1.0 / width;
        auto position_step =
            (vertex_right.position - vertex_left.position) * rh_width;
        auto attribute_step = InterpAttributes(
            vertex_left.attributes, vertex_right.attributes,
            [](float value1, float value2, float t) {
                return (value2 - value1) * t;
            },
            rh_width);
        return Scanline(vertex_left, Vertex{position_step, attribute_step},
                        width, init_y);
    }

   private:
    Scanline(Vertex vertex, Vertex step, float y, float width)
        : vertex(vertex), step(step), y(y), width(width) {}
};
