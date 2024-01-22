#pragma once

#include <SFML/Graphics.hpp>

#include "Vec2.h"

class CTransform {
   public:
    Vec2 pos = {0, 0};
    Vec2 velocity = {0, 0};
    float angle = 0, friction = 0, speed = 0;
    CTransform(const Vec2 _pos, const Vec2 _velocity, float _angle,
               float _friction, float _speed)
        : pos(_pos),
          velocity(_velocity),
          angle(_angle),
          friction(_friction),
          speed(_speed) {}
};

class CShape {
   public:
    sf::CircleShape shape;
    CShape(float radius, int segments, const sf::Color fill,
           const sf::Color outline, float thickness)
        : shape(radius, segments) {
        shape.setRadius(radius);
        shape.setFillColor(fill);
        shape.setOutlineColor(outline);
        shape.setOutlineThickness(thickness);
        shape.setOrigin(radius, radius);
    }
};

class CCollision {
   public:
    float radius = 0;
    CCollision(float _radius) : radius(_radius) {}
};

class CScore {
   public:
    int score = 0;
    CScore(int _score) : score(_score) {}
};

class CLifespan {
   public:
    int total = 0;
    int remaining = 0;
    CLifespan(int _total) : total(_total), remaining(_total) {}
};

class CInput {
   public:
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
    bool shoot = false;
    CInput() {}
};
