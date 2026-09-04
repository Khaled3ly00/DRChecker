#include "drcheck/io/GDSLayoutParser.h"

#include "drcheck/geometry/Point.h"
#include "drcheck/geometry/Polygon.h"

#include <gdstk/gdstk.hpp>

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

    std::vector<domain::Shape> GDSLayoutParser::load(const std::string& filePath, const domain::LayerRegistry& layerRegistry)
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

        if (topCells.count != 1)
        {
            topCells.clear();
            topRawCells.clear();

            throw std::invalid_argument("GDSII file must contain exactly one top-level cell");
        }

        gdstk::Cell* topCell = topCells[0];

        if (topCell->reference_array.count != 0)
        {
            topCells.clear();
            topRawCells.clear();

            throw std::invalid_argument("GDSII hierarchy is not supported yet");
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

        topCells.clear();
        topRawCells.clear();

        return shapes;
    }
}