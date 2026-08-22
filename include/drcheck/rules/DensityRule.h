#pragma once

#include "Rule.h"
#include "drcheck/domain/Layer.h"
#include "drcheck/geometry/BoundingBox.h"

#include <optional>

namespace drcheck::rules {
	enum class DensityLimit
	{
		Minimum,
		Maximum
	};
	class DensityRule : public Rule
	{
	public:
		DensityRule(domain::Layer layer, DensityLimit limit, double requiredDensity, double windowSize, double windowStep);
		DensityRule(domain::Layer layer, DensityLimit limit, double requiredDensity, double windowSize, double windowStep, std::optional<geometry::BoundingBox> analysisWindow);

		std::vector<domain::Violation> check(const std::vector<domain::Shape>& shapes, const spatial::LayerSpatialIndex& spatialIndex) const override;

		domain::Layer getLayer() const;
		DensityLimit getLimit() const;
		double getRequiredDensity() const;
		double getWindowSize() const;
		double getWindowStep() const;
		const std::optional<geometry::BoundingBox>& getAnalysisWindow() const;
	
	private:
		domain::Layer layer;
		DensityLimit limit;
		double requiredDensity;
		double windowSize;
		double windowStep;
		std::optional<geometry::BoundingBox>analysisWindow;

		geometry::BoundingBox resolveAnalysisWindow(const std::vector<domain::Shape>& shapes) const;
		std::vector<geometry::BoundingBox> generateSamplingWindows(const geometry::BoundingBox& region) const;
		double calculateDensity(const geometry::BoundingBox& window, const spatial::LayerSpatialIndex& spatialIndex) const;
		bool violatesDensity(double actualDensity) const;
	};
}