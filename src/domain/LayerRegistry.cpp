#include "drcheck/domain/LayerRegistry.h"

#include <stdexcept>

namespace drcheck::domain {

    const Layer* LayerRegistry::declare(const std::string& name)
    {
        if (name.empty())
        {
            throw std::invalid_argument("Layer name cannot be empty");
        }

        if (layers.contains(name))
        {
            throw std::invalid_argument("Layer already declared: " + name);
        }

        auto layer = std::unique_ptr<Layer>(new Layer(name));
		// raw pointer to the layer, which will be returned to the caller
        const Layer* layerPtr = layer.get();

        layers.emplace(name, std::move(layer));

        return layerPtr;
    }
	// third const means that method doesn't modify the object, so it can be called on const objects
    const Layer* LayerRegistry::resolve(const std::string& name) const
    {
        const auto it = layers.find(name);

        if (it == layers.end())
        {
            throw std::invalid_argument("Layer is not declared: " + name);
        }

        return it->second.get();
    }

}