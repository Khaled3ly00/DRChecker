#include "drcheck/layout/LayoutNormalizer.h"
#include <clipper2/clipper.h>

#include <queue>
#include <stdexcept>
#include <utility>
#include <vector>
#include <algorithm>

namespace {

    constexpr int CLIPPER_PRECISION = 5;

    // Helper function to convert Polygon points to Clipper Path
    Clipper2Lib::PathD toClipperPath(const drcheck::geometry::Polygon& polygon)
    {
        Clipper2Lib::PathD path;
        path.reserve(polygon.getVertexCount());

        for (const auto& vertex : polygon.getVertices())
        {
            path.emplace_back(vertex.getX(), vertex.getY());
        }
        // Use consistent counter-clockwise orientation for all subject polygons
        // so they behave correctly together with the NonZero fill rule.
        if (polygon.getOrientation() == drcheck::geometry::Orientation::Clockwise)
        {
            std::reverse(path.begin(), path.end());
        }

        return path;
    }

    // Helper function to convert Clipper Path to Polygon points 
    drcheck::geometry::Polygon toPolygon(const Clipper2Lib::PathD& path)
    {
        std::vector<drcheck::geometry::Point> vertices;
        vertices.reserve(path.size());

        for (const auto& point : path)
        {
            vertices.emplace_back(point.x, point.y);
        }

        return drcheck::geometry::Polygon(std::move(vertices));
    }
}

namespace drcheck::layout {
	bool LayoutNormalizer::areMergeable(const domain::Shape& first, const domain::Shape& second) {
		if (first.getLayer() != second.getLayer())
		{
			return false;
		}

		const geometry::Polygon& firstPolygon = first.getPolygon();
		const geometry::Polygon& secondPolygon = second.getPolygon();

		return firstPolygon.overlaps(secondPolygon) || firstPolygon.sharesBoundarySegment(secondPolygon);
	}

    // Finds all shapes that belong to the same mergeable connected component.
    //
    // The search uses Breadth-First Search (BFS):
    // - Each shape is treated as a graph node.
    // - Two shapes are connected when areMergeable() returns true.
    // - LayerSpatialIndex is used to limit candidate checks to nearby shapes.
    // - visited prevents processing the same shape more than once.
    //
    // This allows transitive connectivity to be discovered.
    // For example, if A merges with B and B merges with C, then A, B, and C
    // are returned in the same component even if A does not directly touch C.
    // Polygons with touching vertices are not considered mergable
    std::vector<const domain::Shape*> LayoutNormalizer::collectConnectedComponent(
        const domain::Shape& start, const spatial::LayerSpatialIndex& spatialIndex, std::unordered_set<const domain::Shape*>& visited)
    {
        std::vector<const domain::Shape*> component;

        // BFS queue containing shapes that have been discovered
        // but whose neighboring shapes have not yet been explored.
        std::queue<const domain::Shape*> pending;

        // The starting shape belongs to this component and must not
        // be discovered again later.
        visited.insert(&start);
        pending.push(&start);

        while (!pending.empty())
        {
            // Process the oldest discovered shape first (BFS order).
            const domain::Shape* current = pending.front();
            pending.pop();

            component.push_back(current);

            // Query only spatially nearby shapes on the same layer.
            // Shapes that touch or overlap current must have overlapping
            // bounding boxes, so no expansion is required here.
            const auto candidates = spatialIndex.query(current->getLayer(), current->getPolygon().getBoundingBox());

            for (const domain::Shape* candidate : candidates)
            {
                // Skip shapes that already belong to a previously discovered
                // part of this or another connected component.
                if (visited.contains(candidate))
                {
                    continue;
                }

                // Spatial proximity alone does not mean the shapes should
                // be merged. Apply the normalization connectivity policy.
                if (!areMergeable(*current, *candidate))
                {
                    continue;
                }

                // This candidate belongs to the same connected component.
                // Mark it immediately so it cannot be added to the queue twice.
                visited.insert(candidate);
                pending.push(candidate);
            }
        }

        return component;
    }
    
    // Merges list of shapes provided
    domain::Shape LayoutNormalizer::mergeComponent(std::size_t normalizedId, const std::vector<const domain::Shape*>& component)
    {
        Clipper2Lib::PathsD paths;
        paths.reserve(component.size());

        for (const domain::Shape* shape : component)
        {
            paths.push_back(toClipperPath(shape->getPolygon()));
        }
        // Merge polygons
        const Clipper2Lib::PathsD solution = Clipper2Lib::Union(paths, Clipper2Lib::FillRule::NonZero, CLIPPER_PRECISION);

        if (solution.size() != 1)
        {
            throw std::logic_error("Normalized shape union produced unsupported multiple contours or holes");
        }
        // Generated Polygon
        geometry::Polygon mergedPolygon = toPolygon(solution.front());
        // first shape in components to retrieve layer and purpose
        const domain::Shape& firstShape = *component.front();
        // construct merged shape and return
        return domain::Shape(normalizedId, firstShape.getLayer(), firstShape.getPurpose(), std::move(mergedPolygon));
    }

    std::vector<domain::Shape> LayoutNormalizer::normalize(const std::vector<domain::Shape>& shapes)
    {
        std::size_t normalizedId = 1;
        if (shapes.empty())
        {
            return {};
        }

        spatial::LayerSpatialIndex spatialIndex(shapes);

        std::unordered_set<const domain::Shape*> visited;

        std::vector<domain::Shape> normalizedShapes;
        normalizedShapes.reserve(shapes.size());

        for (const domain::Shape& shape : shapes)
        {
            if (visited.contains(&shape))
            {
                continue;
            }

            const auto component = collectConnectedComponent(shape, spatialIndex, visited);

            if (component.size() == 1)
            {
                const domain::Shape& original = *component.front();

                normalizedShapes.emplace_back(normalizedId, original.getLayer(), original.getPurpose(), original.getPolygon());

                ++normalizedId;
            }
            else
            {
                normalizedShapes.push_back(mergeComponent(normalizedId, component));
                ++normalizedId;
            }
        }

        return normalizedShapes;
    }

}