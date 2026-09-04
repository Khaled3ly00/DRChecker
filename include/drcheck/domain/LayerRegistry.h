#pragma once

#include "drcheck/domain/Layer.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <map>
#include <utility>

namespace drcheck::domain {

	class LayerRegistry {
	public:
		LayerRegistry() = default;

		LayerRegistry(const LayerRegistry&) = delete;
		LayerRegistry& operator=(const LayerRegistry&) = delete;
		LayerRegistry(LayerRegistry&&) = delete;
		LayerRegistry& operator=(LayerRegistry&&) = delete;

		const Layer* declare(const std::string& name);
		const Layer* resolve(const std::string& name) const;

		void mapGDS(const Layer* layer, int gdsLayer, int gdsDatatype);
		const Layer* resolveGDS(int gdsLayer, int gdsDatatype) const;

	private:
		std::unordered_map<std::string, std::unique_ptr<Layer>> layers;
		// std::pair doesn't have a hash function, so we use std::map instead of std::unordered_map
		std::map<std::pair<int, int>, const Layer*> gdsMappings;
	};
}