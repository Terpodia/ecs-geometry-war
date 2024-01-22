#pragma once

#include <memory>

#include "Components.h"

class Entity {
    std::string m_tag;
    size_t m_id;
    bool m_alive = true;
    Entity(const std::string t, const size_t i);

   public:
    std::shared_ptr<CTransform> cTransform;
    std::shared_ptr<CShape> cShape;
    std::shared_ptr<CCollision> cCollision;
    std::shared_ptr<CScore> cScore;
    std::shared_ptr<CLifespan> cLifespan;
    std::shared_ptr<CInput> cInput;
    const bool isAlive() const;
    const std::string& tag() const;
    const size_t& id() const;
    void destroy();

    friend class EntityManager;
};
