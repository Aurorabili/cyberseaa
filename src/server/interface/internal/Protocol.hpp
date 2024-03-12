#pragma once

#include "common/utils/IntTypes.hpp"
#include <string>

namespace Interface {
namespace Internal {
    class Protocol {
    public:
        Protocol() = default;
        virtual ~Protocol() = default;

        std::string name() const
        {
            return m_name;
        }

        virtual std::string serialize() = 0;
        virtual void deserialize(const std::string& data) = 0;

    private:
        std::string m_name;

        ///< deprecated (2024-03-11) - For compatibility with old protocol discrimination. Do not use it now.
        ///< @deprecated
        u8 m_protocolType;
    };
}
}