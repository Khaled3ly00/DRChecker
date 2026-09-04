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

    void LayerRegistry::mapGDS(const Layer* layer, int gdsLayer, int datatype)
    {
        if (layer == nullptr)
        {
            throw std::invalid_argument("Cannot map GDS pair to a null layer");
        }

        if (gdsLayer < 0 || datatype < 0)
        {
            throw std::invalid_argument("GDS layer and datatype must be non-negative");
        }

        const Layer* registeredLayer = resolve(layer->getName());

        if (registeredLayer != layer)
        {
            throw std::invalid_argument("Layer does not belong to this LayerRegistry");
        }

        const auto key = std::make_pair(gdsLayer, datatype);

        if (gdsMappings.contains(key))
        {
            throw std::invalid_argument("GDS layer/datatype pair is already mapped");
        }

        gdsMappings.emplace(key, layer);
    }

    const Layer* LayerRegistry::resolveGDS(int gdsLayer, int datatype) const
    {
        if (gdsLayer < 0 || datatype < 0)
        {
            throw std::invalid_argument("GDS layer and datatype must be non-negative");
        }

        const auto key = std::make_pair(gdsLayer, datatype);

        const auto match =gdsMappings.find(key);

        if (match == gdsMappings.end())
        {
            throw std::invalid_argument("No layer mapped to GDS layer " + std::to_string(gdsLayer) + " datatype " + std::to_string(datatype));
        }

        return match->second;
    }

}