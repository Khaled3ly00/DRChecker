#pragma once

#include "drcheck/domain/Layer.h"

#include <memory>
#include <string>
#include <unordered_map>

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

	private:
		std::unordered_map<std::string, std::unique_ptr<Layer>> layers;
	};
}