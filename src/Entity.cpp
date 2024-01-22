#include "Entity.h"

Entity::Entity(const std::string t, const size_t i) : m_tag(t), m_id(i) {}

const bool Entity::isAlive() const { return m_alive; }

const std::string& Entity::tag() const { return m_tag; }

const size_t& Entity::id() const { return m_id; }

void Entity::destroy() { m_alive = false; }
