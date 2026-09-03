#pragma once

#include <string>

namespace drcheck::domain {
    class Layer
    {
    public:
        const std::string& getName() const;

    private:
        explicit Layer(std::string name);

        std::string name;

        friend class LayerRegistry;
    };
}