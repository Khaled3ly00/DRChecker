#include "drcheck/io/GDSLayoutParser.h"

#include "drcheck/geometry/Point.h"
#include "drcheck/geometry/Polygon.h"

#include <gdstk/gdstk.hpp>
#include <unordered_set>
#include <stdexcept>
#include <utility>

namespace drcheck::io {
    namespace {
        void appendPolygon(const gdstk::Polygon& gdsPolygon, const domain::LayerRegistry& layerRegistry, std::size_t& shapeId, std::vector<domain::Shape>& shapes)
        {
            const int gdsLayer = static_cast<int>(gdstk::get_layer(gdsPolygon.tag));
            const int datatype = static_cast<int>(gdstk::get_type(gdsPolygon.tag));
            const domain::Layer* layer = layerRegistry.resolveGDS(gdsLayer, datatype);

            std::vector<geometry::Point> vertices;
            vertices.reserve(gdsPolygon.point_array.count);

            for (std::uint64_t i = 0; i < gdsPolygon.point_array.count; ++i)
            {
                const gdstk::Vec2& point = gdsPolygon.point_array[i];
                vertices.emplace_back(point.x, point.y);
            }

            // Protection against a repeated closing point
            if (vertices.size() > 1 && vertices.front().getX() == vertices.back().getX() && vertices.front().getY() == vertices.back().getY())
            {
                vertices.pop_back();
            }

            geometry::Polygon polygon(std::move(vertices));

            shapes.emplace_back(shapeId++,layer, std::move(polygon));
        }
		// Helper function to detect recursive hierarchy in GDSII cells (DFS traversal)
        bool hasRecursiveHierarchy(const gdstk::Cell* cell, std::unordered_set<const gdstk::Cell*>& visiting, std::unordered_set<const gdstk::Cell*>& visited)
        {
            if (visiting.contains(cell))
            {
                return true;
            }

            if (visited.contains(cell))
            {
                return false;
            }

            visiting.insert(cell);

            for (std::uint64_t i = 0; i < cell->reference_array.count; ++i)
            {
                const gdstk::Reference* reference = cell->reference_array[i];

                if (reference->type != gdstk::ReferenceType::Cell)
                {
                    continue;
                }

                if (hasRecursiveHierarchy(reference->cell, visiting, visited))
                {
                    return true;
                }
            }

            visiting.erase(cell);
            visited.insert(cell);

            return false;
        }
		// Helper function to select the top-level cell from the GDSII library based on the provided name or default selection
        const gdstk::Cell* selectTopCell(const gdstk::Array<gdstk::Cell*>& topCells, const std::optional<std::string>& topCellName)
        {
            if (topCellName.has_value())
            {
                for (std::uint64_t i = 0; i < topCells.count; ++i)
                {
                    if (topCells[i]->name == topCellName.value())
                    {
                        return topCells[i];
                    }
                }

                throw std::invalid_argument("Requested GDSII top-level cell was not found: " + topCellName.value());
            }

            if (topCells.count == 0)
            {
                throw std::invalid_argument("GDSII layout contains no top-level cell");
            }

            if (topCells.count > 1)
            {
                throw std::invalid_argument("GDSII layout contains multiple top-level cells; a top-level cell must be selected");
            }

            return topCells[0];
        }
    }
	// RAII guard to ensure that the gdstk::Library is freed when it goes out of scope
    class GDSLibraryGuard
    {
    public:
        explicit GDSLibraryGuard(gdstk::Library& library)
            : library(library)
        {
        }

        ~GDSLibraryGuard()
        {
            library.free_all();
        }

        GDSLibraryGuard(const GDSLibraryGuard&) = delete;
        GDSLibraryGuard& operator=(const GDSLibraryGuard&) = delete;

    private:
        gdstk::Library& library;
    };
	// RAII guard to ensure that the gdstk::Array<gdstk::Polygon*> is cleared and freed when it goes out of scope
    class GDSPolygonArrayGuard
    {
    public:
        explicit GDSPolygonArrayGuard(gdstk::Array<gdstk::Polygon*>& polygons)
            : polygons(polygons)
        {
        }

        ~GDSPolygonArrayGuard()
        {
            for (std::uint64_t i = 0; i < polygons.count; ++i)
            {
                polygons[i]->clear();
                gdstk::free_allocation(polygons[i]);
            }

            polygons.clear();
        }

        GDSPolygonArrayGuard(const GDSPolygonArrayGuard&) = delete;
        GDSPolygonArrayGuard& operator=(const GDSPolygonArrayGuard&) = delete;

    private:
        gdstk::Array<gdstk::Polygon*>& polygons;
    };

    std::vector<domain::Shape> GDSLayoutParser::load(const std::string& filePath, const domain::LayerRegistry& layerRegistry, const std::optional<std::string>& topCellName)
    {
        gdstk::ErrorCode errorCode = gdstk::ErrorCode::NoError;

        gdstk::Library library = gdstk::read_gds(filePath.c_str(), 1e-6, 0, nullptr, &errorCode);

        GDSLibraryGuard libraryGuard(library);

        if (errorCode != gdstk::ErrorCode::NoError)
        {
            throw std::runtime_error("Failed to read GDSII file: " + filePath);
        }

		gdstk::Array<gdstk::Cell*> topCells{}; // Array to hold top-level cells in the GDSII library
        gdstk::Array<gdstk::RawCell*> topRawCells{};

        library.top_level(topCells, topRawCells);

        const gdstk::Cell* topCell = selectTopCell(topCells, topCellName);

		// check for recursive hierarchy in the GDSII layout
        std::unordered_set<const gdstk::Cell*> visiting;
        std::unordered_set<const gdstk::Cell*> visited;
        if (hasRecursiveHierarchy(topCell, visiting, visited))
        {
            throw std::invalid_argument("GDSII layout contains recursive cell references");
        }

        std::vector<domain::Shape> shapes;
        shapes.reserve(topCell->polygon_array.count);

        std::size_t shapeId = 1;

		// Append all polygons in the top-level cell to the shapes vector
        for (std::uint64_t i = 0;i < topCell->polygon_array.count; ++i)
        {
            appendPolygon(*topCell->polygon_array[i], layerRegistry, shapeId, shapes);
        }

		// Append all paths in the top-level cell to the shapes vector
        for (std::uint64_t i = 0; i < topCell->flexpath_array.count; ++i)
        {
            gdstk::Array<gdstk::Polygon*> pathPolygons{};

            GDSPolygonArrayGuard pathPolygonGuard(pathPolygons);

            const gdstk::ErrorCode pathError = topCell->flexpath_array[i]->to_polygons(false, 0, pathPolygons);

            if (pathError != gdstk::ErrorCode::NoError)
            {
                throw std::runtime_error("Failed to convert GDSII PATH to polygon");
            }

            for (std::uint64_t j = 0;j < pathPolygons.count; ++j)
            {
                appendPolygon(*pathPolygons[j], layerRegistry, shapeId, shapes);
            }
        }
		// Append (SREF) all polygons in the referenced cells to the shapes vector
        for (std::uint64_t i = 0; i < topCell->reference_array.count; ++i)
        {
            const gdstk::Reference* reference = topCell->reference_array[i];

            gdstk::Array<gdstk::Polygon*> referencedPolygons{};

            GDSPolygonArrayGuard polygonGuard(referencedPolygons);

            reference->get_polygons(
                true,   // Expand repetitions / AREFs
                false,  // Do not include paths yet
                -1,      // Depth: include all nested reference levels
                false,  // No layer filter
                0,      // Ignored because filter == false
                referencedPolygons
            );

            for (std::uint64_t j = 0; j < referencedPolygons.count; ++j)
            {
                appendPolygon(*referencedPolygons[j], layerRegistry, shapeId, shapes);
            }
        }

        topCells.clear();
        topRawCells.clear();

        return shapes;
    }
}