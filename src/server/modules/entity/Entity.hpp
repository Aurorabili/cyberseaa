#ifndef MODULES_ENTITY_HPP_
#define MODULES_ENTITY_HPP_

#include <string>

#include "common/core/UUIDProvider.hpp"
#include "common/utils/IntTypes.hpp"

namespace modules {
class Entity {
public:
    Entity()
    {
        this->m_id = UUIDProvider::nextUUID();
        this->Label = "";
    }
    Entity(const std::string& _label)
    {
        this->m_id = UUIDProvider::nextUUID();
        this->Label = _label;
    }
    Entity(const s64 _id, const std::string& _label)
    {
        this->m_id = _id;
        this->Label = _label;
    }
    virtual ~Entity() = default;

public:
    s64 EntityId() const { return m_id; }

public:
    std::string Label;

private:
    s64 m_id;
};
}

#endif // MODULES_ENTITY_HPP_
