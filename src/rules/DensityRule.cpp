#include "drcheck/rules/DensityRule.h"
#include "drcheck/spatial/LayerSpatialIndex.h"
#include "drcheck/rules/Constants.h"

#include <stdexcept>
#include <utility>
#include <algorithm>

namespace drcheck::rules {
    // Delegate constructor for non specified analysis window
    DensityRule::DensityRule(const domain::Layer* layer, DensityLimit limit, double requiredDensity, double windowSize, double windowStep)
        :DensityRule(layer, limit, requiredDensity, windowSize, windowStep, std::nullopt)
    {
    }

    DensityRule::DensityRule(const domain::Layer* layer, DensityLimit limit, double requiredDensity, double windowSize, double windowStep, std::optional<geometry::BoundingBox> analysisWindow)
        :layer(layer), limit(limit), requiredDensity(requiredDensity), windowSize(windowSize), windowStep(windowStep), analysisWindow(std::move(analysisWindow))
    {
        if (layer == nullptr)
        {
            throw std::invalid_argument("layer cannot be null");
        }
        if (requiredDensity < 0.0 || requiredDensity > 1.0)
        {
            throw std::invalid_argument("Required density must be between 0 and 1");
        }
        if (windowSize <= 0.0)
        {
            throw std::invalid_argument("Density window size must be positive");
        }

        if (windowStep <= 0.0)
        {
            throw std::invalid_argument("Density window step must be positive");
        }
    }

    const domain::Layer* DensityRule::getLayer() const
    {
        return layer;
    }

    DensityLimit DensityRule::getLimit() const
    {
        return limit;
    }

    double DensityRule::getRequiredDensity() const
    {
        return requiredDensity;
    }

    double DensityRule::getWindowSize() const
    {
        return windowSize;
    }

    double DensityRule::getWindowStep() const
    {
        return windowStep;
    }

    const std::optional<geometry::BoundingBox>& DensityRule::getAnalysisWindow() const
    {
        return analysisWindow;
    }

    // Returns analysis window either whole layout (merging) or specified by user
    geometry::BoundingBox DensityRule::resolveAnalysisWindow(const std::vector<domain::Shape>& shapes) const {
        if (analysisWindow.has_value())
        {
            return analysisWindow.value();
        }
        if (shapes.empty())
        {
            throw std::invalid_argument("Cannot determine density analysis window from an empty layout");
        }

        geometry::BoundingBox bounds = shapes[0].getPolygon().getBoundingBox();

        for (std::size_t i = 1; i < shapes.size(); ++i)
        {
            bounds = bounds.mergedWith(shapes[i].getPolygon().getBoundingBox());
        }

        return bounds;
    }
    // Generates sampling windows
    // For overflow(partial) windows a special smaller window is created
    std::vector<geometry::BoundingBox> DensityRule::generateSamplingWindows(const geometry::BoundingBox& region) const
    {
        std::vector<geometry::BoundingBox> windows;
        
        for (double y = region.getMinY(); y < region.getMaxY() - DRC_LENGTH_TOLERANCE; y += windowStep)
        {
            const double windowMaxY = std::min(y + windowSize, region.getMaxY());
            for (double x = region.getMinX(); x < region.getMaxX() - DRC_LENGTH_TOLERANCE; x += windowStep)
            {
                const double windowMaxX = std::min(x + windowSize, region.getMaxX());
                windows.emplace_back(x, y, windowMaxX, windowMaxY);
            }
        }
        return windows;
    }
    double DensityRule::calculateDensity(const geometry::BoundingBox& window, const spatial::LayerSpatialIndex& spatialIndex) const
    {
        // Spatial Indexing for shapes with specific layer in this sample window
        const auto candidates = spatialIndex.query(layer,window);
        // Calculate area of polygons inside the window
        double coveredArea = 0.0;
        for (const domain::Shape* shape : candidates)
        {
            coveredArea += shape->getPolygon().areaInsideWindow(window);
        }

        const double windowWidth = window.getMaxX() - window.getMinX();
        const double windowHeight = window.getMaxY() - window.getMinY();

        const double windowArea = windowWidth * windowHeight;

        return coveredArea / windowArea;
    }
    bool DensityRule::violatesDensity (double actualDensity) const
    {
        switch (limit)
        {
        case DensityLimit::Minimum:
            return actualDensity < requiredDensity;

        case DensityLimit::Maximum:
            return actualDensity > requiredDensity;
        }

        throw std::logic_error("Unknown density limit type");
    }
    std::vector<domain::Violation> DensityRule::check(const std::vector<domain::Shape>& shapes, const spatial::LayerSpatialIndex& spatialIndex) const
    {
        std::vector<domain::Violation> violations;
        // Generate analysis window
        const geometry::BoundingBox analysisWindow = resolveAnalysisWindow(shapes);
        // Generate sample windows 
        const auto sampleWindows = generateSamplingWindows(analysisWindow);

        const domain::ViolationType violationType = limit == DensityLimit::Minimum ? domain::ViolationType::MinDensity : domain::ViolationType::MaxDensity;
        const std::string message = limit == DensityLimit::Minimum ? "Minimum density violation" : "Maximum density violation";
        // loop through windows and calculate their densities

        for (const auto& window : sampleWindows)
        {
            const double actualDensity = calculateDensity(window, spatialIndex);

            if (!violatesDensity(actualDensity))
            {
                continue;
            }
            // aggregate designated initializer used in c++20 to initialize members by member name
            // without needing to initialize all members
            const domain::ViolationMarker marker{
                .region = window,
                .firstLayer = getLayer()
            };
            const std::string msg = message + " on layer : " + layer->getName() + " required " + std::to_string(requiredDensity) + " actual " + std::to_string(actualDensity);
            violations.emplace_back(violationType, std::vector<std::size_t>{}, msg, actualDensity, requiredDensity, marker);
        }
        return violations;
    }
}