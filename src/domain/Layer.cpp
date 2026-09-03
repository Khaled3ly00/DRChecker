#include "drcheck/domain/Layer.h"

#include <stdexcept>

namespace drcheck::domain {
    Layer::Layer(std::string name)
        : name(std::move(name))
    {
    }

    const std::string& Layer::getName() const
    {
        return name;
    }
}