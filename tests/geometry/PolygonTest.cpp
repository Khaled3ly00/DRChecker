#include <gtest/gtest.h>

#include "drcheck/geometry/Polygon.h"
#include "drcheck/geometry/Constants.h"

#include <stdexcept>

using namespace drcheck::geometry;

TEST(PolygonTest, InitializesWithVerticesAndGetsCorrectVertexCount)
{
    std::vector<Point> vertices = {
        Point(0.0, 0.0),
        Point(3.0, 0.0),
        Point(3.0, 4.0)
    };

    Polygon polygon(vertices);

    EXPECT_EQ(polygon.getVertexCount(), 3);
}

TEST(PolygonTest, RejectsFewerThanThreeVertices)
{
    EXPECT_THROW(
        Polygon({
            Point(0.0, 0.0),
            Point(5.0, 0.0)
            }),
        std::invalid_argument
    );
}

TEST(PolygonTest, RejectsZeroLengthEdges)
{
	EXPECT_THROW(
		Polygon({
			Point(0.0, 0.0),
			Point(5.0, 0.0),
			Point(5.0, 0.0), // Zero-length edge
			Point(0.0, 5.0)
			}),
		std::invalid_argument
	);
}

TEST(PolygonTest, RejectsNonZeroAreaSelfIntersectingPolygon)
{
	EXPECT_THROW(
		Polygon({
			Point(0.0, 0.0),
			Point(4.0, 4.0),
			Point(0.0, 4.0),
			Point(5.0, 0.0)
			}),
		std::invalid_argument
	);
}

TEST(PolygonTest, RejectsDegeneratePolygon)
{
	EXPECT_THROW(
		Polygon({
			Point(0.0, 0.0),
			Point(5.0, 0.0),
			Point(10.0, 0.0) // Collinear points
			}),
		std::invalid_argument
	);
}

TEST(PolygonTest, AcceptsSmallNonDegeneratePolygon)
{
	EXPECT_NO_THROW(
		Polygon({
			Point(0.0, 0.0),
			Point(1e-5, 0.0),
			Point(0.0, 1e-5)
		})
	);
}

TEST(PolygonTest, GetsCorrectEdges)
{
    std::vector<Point> vertices = {
        Point(0.0, 0.0),
        Point(3.0, 0.0),
        Point(3.0, 4.0)
    };

    Polygon polygon(vertices);

    std::vector<Segment> edges = polygon.getEdges();
    EXPECT_EQ(edges.size(), 3);
}

TEST(PolygonTest, CalculatesRectangleArea)
{
    std::vector<Point> vertices = {
        Point(0.0, 0.0),
        Point(8.0, 0.0),
        Point(8.0, 5.0),
        Point(0.0, 5.0)
    };
    Polygon polygon(vertices);

    EXPECT_DOUBLE_EQ(polygon.area(), 40.0);
}

TEST(PolygonTest, DetectsCounterClockwiseOrientation)
{
    std::vector<Point> vertices = {
        Point(0.0, 0.0),
        Point(8.0, 0.0),
        Point(8.0, 5.0),
        Point(0.0, 5.0)
    };
    Polygon polygon(vertices);

    EXPECT_EQ(polygon.getOrientation(), Orientation::CounterClockwise);
}

TEST(PolygonTest, DetectsClockwiseOrientation)
{
    std::vector<Point> vertices = {
        Point(0.0, 0.0),
        Point(0.0, 5.0),
        Point(8.0, 5.0),
        Point(8.0, 0.0)
    };
    Polygon polygon(vertices);

    EXPECT_EQ(polygon.getOrientation(), Orientation::Clockwise);
}

TEST(PolygonTest, CalculatesBoundingBox)
{   
    std::vector<Point> vertices = {
        Point(2.0, 1.0),
        Point(8.0, 3.0),
        Point(6.0, 9.0),
        Point(1.0, 6.0)
    };
    Polygon polygon(vertices);

    BoundingBox box = polygon.getBoundingBox();

    EXPECT_DOUBLE_EQ(box.getMinX(), 1.0);
    EXPECT_DOUBLE_EQ(box.getMaxX(), 8.0);

    EXPECT_DOUBLE_EQ(box.getMinY(), 1.0);
    EXPECT_DOUBLE_EQ(box.getMaxY(), 9.0);
}

TEST(PolygonTest, ContainsPointInside)
{
    Polygon polygon({
        Point(0.0, 0.0),
        Point(10.0, 0.0),
        Point(10.0, 5.0),
        Point(0.0, 5.0)
        });

    Point point(5.0, 2.0);

    EXPECT_TRUE(polygon.contains(point));
}

TEST(PolygonTest, RejectsPointOutside)
{
    Polygon polygon({
        Point(0.0, 0.0),
        Point(10.0, 0.0),
        Point(10.0, 5.0),
        Point(0.0, 5.0)
        });

    Point point(15.0, 2.0);

    EXPECT_FALSE(polygon.contains(point));
}

TEST(PolygonTest, ContainsPointAtVertex)
{
    Polygon polygon({
        Point(0.0, 0.0),
        Point(10.0, 0.0),
        Point(10.0, 5.0),
        Point(0.0, 5.0)
        });

    Point point(10.0, 5.0);

    EXPECT_TRUE(polygon.contains(point));
}

TEST(PolygonTest, ContainsPointOnEdge)
{
	Polygon polygon({
		Point(0.0, 0.0),
		Point(10.0, 0.0),
		Point(10.0, 5.0),
		Point(0.0, 5.0)
		});
	Point point(5.0, 0.0);
	EXPECT_TRUE(polygon.contains(point));
}

TEST(PolygonTest, ContainsPointWithinBoundaryTolerance)
{
	Polygon polygon({
		Point(0.0, 0.0),
		Point(10.0, 0.0),
		Point(10.0, 5.0),
		Point(0.0, 5.0)
	});

	const Point point(
		10.0 + EPSILON * 0.5,
		2.0
	);

	EXPECT_TRUE(polygon.contains(point));
}

TEST(PolygonTest, ContainsPointInConcavePolygon) {
	Polygon polygon({
		Point(0.0, 0.0),
		Point(5.0, 5.0),
		Point(10.0, 0.0),
		Point(10.0, 10.0),
		Point(0.0, 10.0)
		});
	Point point(4.0, 4.0);
	EXPECT_TRUE(polygon.contains(point));
}

TEST(PolygonTest, DoesNotContainPointInConcavePolygon) {
    Polygon polygon({
        Point(0.0, 0.0),
        Point(5.0, 5.0),
        Point(10.0, 0.0),
        Point(10.0, 10.0),
        Point(0.0, 10.0)
        });
    Point point(5.0, 2.0);
    EXPECT_FALSE(polygon.contains(point));
}

TEST(PolygonTest, DetectsOverlappingPolygons)
{
    Polygon first({
        Point(0.0, 0.0),
        Point(6.0, 0.0),
        Point(6.0, 4.0),
        Point(0.0, 4.0)
        });

    Polygon second({
        Point(4.0, 2.0),
        Point(10.0, 2.0),
        Point(10.0, 6.0),
        Point(4.0, 6.0)
        });

    EXPECT_TRUE(first.intersects(second));
}

TEST(PolygonTest, SeparatedPolygonsDoNotIntersect)
{
    Polygon first({
        Point(0.0, 0.0),
        Point(4.0, 0.0),
        Point(4.0, 4.0),
        Point(0.0, 4.0)
        });

    Polygon second({
        Point(10.0, 0.0),
        Point(14.0, 0.0),
        Point(14.0, 4.0),
        Point(10.0, 4.0)
        });

    EXPECT_FALSE(first.intersects(second));
}

TEST(PolygonTest, DetectsContainedPolygon)
{
    Polygon outer({
        Point(0.0, 0.0),
        Point(10.0, 0.0),
        Point(10.0, 10.0),
        Point(0.0, 10.0)
        });

    Polygon inner({
        Point(3.0, 3.0),
        Point(7.0, 3.0),
        Point(7.0, 7.0),
        Point(3.0, 7.0)
        });

    EXPECT_TRUE(outer.intersects(inner));
}

TEST(PolygonTest, DetectsContainedPolygonReverseCall)
{
    Polygon outer({
        Point(0.0, 0.0),
        Point(10.0, 0.0),
        Point(10.0, 10.0),
        Point(0.0, 10.0)
        });

    Polygon inner({
        Point(3.0, 3.0),
        Point(7.0, 3.0),
        Point(7.0, 7.0),
        Point(3.0, 7.0)
        });

    EXPECT_TRUE(inner.intersects(outer));
}

TEST(PolygonTest, IntersectionIsSymmetric)
{
    Polygon first({
        Point(0.0, 0.0),
        Point(6.0, 0.0),
        Point(6.0, 6.0),
        Point(0.0, 6.0)
        });

    Polygon second({
        Point(4.0, 4.0),
        Point(8.0, 4.0),
        Point(8.0, 8.0),
        Point(4.0, 8.0)
        });

    EXPECT_EQ(
        first.intersects(second),
        second.intersects(first)
    );
}

TEST(PolygonTest, TouchingEdgesCountAsIntersection)
{
    Polygon first({
        Point(0.0, 0.0),
        Point(5.0, 0.0),
        Point(5.0, 5.0),
        Point(0.0, 5.0)
        });

    Polygon second({
        Point(5.0, 0.0),
        Point(10.0, 0.0),
        Point(10.0, 5.0),
        Point(5.0, 5.0)
        });

    EXPECT_TRUE(first.intersects(second));
}

TEST(PolygonTest, TouchingVertexCountsAsIntersection)
{
    Polygon first({
        Point(0.0, 0.0),
        Point(5.0, 0.0),
        Point(5.0, 5.0),
        Point(0.0, 5.0)
        });

    Polygon second({
        Point(5.0, 5.0),
        Point(9.0, 5.0),
        Point(9.0, 9.0),
        Point(5.0, 9.0)
        });

    EXPECT_TRUE(first.intersects(second));
}
TEST(PolygonTest, NearTouchingPolygonsIntersectSymmetrically)
{
	Polygon first({
		Point(0.0, 0.0),
		Point(1.0, 0.0),
		Point(1.0, 1.0),
		Point(0.0, 1.0)
	});

	const double secondMinX = 1.0 + EPSILON * 0.5;
	Polygon second({
		Point(secondMinX, 0.0),
		Point(2.0, 0.0),
		Point(2.0, 1.0),
		Point(secondMinX, 1.0)
	});

	EXPECT_TRUE(first.intersects(second));
	EXPECT_TRUE(second.intersects(first));
	EXPECT_NEAR(first.distanceTo(second).distance, 0.0, EPSILON);
	EXPECT_NEAR(second.distanceTo(first).distance, 0.0, EPSILON);
}

// HEAVY TESTS FOR POLYGON DISTANCE CALCULATION (MAINLY USED IN DRC)
TEST(PolygonTest, CalculatesDistanceBetweenSeparatedPolygons)
{
    Polygon first({
        Point(0.0, 0.0),
        Point(5.0, 0.0),
        Point(5.0, 5.0),
        Point(0.0, 5.0)
        });

    Polygon second({
        Point(8.0, 0.0),
        Point(13.0, 0.0),
        Point(13.0, 5.0),
        Point(8.0, 5.0)
        });

    const auto result = first.distanceTo(second);

    EXPECT_NEAR(result.distance, 3.0, EPSILON);
    ASSERT_TRUE(result.firstEdgeIndex < first.getEdges().size());
    EXPECT_EQ(result.firstEdgeIndex, 1);
    ASSERT_TRUE(result.secondEdgeIndex < second.getEdges().size());
    EXPECT_EQ(result.secondEdgeIndex, 3);

    const auto firstEdges = first.getEdges();
    const auto secondEdges = second.getEdges();
    EXPECT_TRUE(firstEdges[result.firstEdgeIndex].contains(result.firstPoint));
    EXPECT_TRUE(secondEdges[result.secondEdgeIndex].contains(result.secondPoint));
}

TEST(PolygonTest, IntersectingPolygonsHaveZeroDistance)
{
    Polygon first({
        Point(0.0, 0.0),
        Point(6.0, 0.0),
        Point(6.0, 6.0),
        Point(0.0, 6.0)
        });

    Polygon second({
        Point(4.0, 4.0),
        Point(8.0, 4.0),
        Point(8.0, 8.0),
        Point(4.0, 8.0)
        });

    const auto result = first.distanceTo(second);

    EXPECT_NEAR(result.distance, 0.0, EPSILON);
    ASSERT_TRUE(result.firstEdgeIndex < first.getEdges().size());
    ASSERT_TRUE(result.secondEdgeIndex < second.getEdges().size());
    const auto firstEdges = first.getEdges();
    const auto secondEdges = second.getEdges();
    EXPECT_TRUE(firstEdges[result.firstEdgeIndex].contains(result.firstPoint));
    EXPECT_TRUE(secondEdges[result.secondEdgeIndex].contains(result.secondPoint));
}

TEST(PolygonTest, PolygonDistanceIsSymmetric)
{
	Polygon first({
		Point(0.0, 0.0),
		Point(5.0, 0.0),
		Point(5.0, 5.0),
		Point(0.0, 5.0)
		});
	Polygon second({
		Point(8.0, 0.0),
		Point(13.0, 0.0),
		Point(13.0, 5.0),
		Point(8.0, 5.0)
		});

    EXPECT_NEAR(first.distanceTo(second).distance, second.distanceTo(first).distance, EPSILON);
}

TEST(PolygonTest, DistanceToContainedPolygonIsZero)
{
	Polygon outer({
		Point(0.0, 0.0),
		Point(10.0, 0.0),
		Point(10.0, 10.0),
		Point(0.0, 10.0)
		});
	Polygon inner({
		Point(3.0, 3.0),
		Point(7.0, 3.0),
		Point(7.0, 7.0),
		Point(3.0, 7.0)
		});

    const auto result = outer.distanceTo(inner);

    EXPECT_NEAR(result.distance, 0.0, EPSILON);
    ASSERT_TRUE(result.firstEdgeIndex < outer.getEdges().size());
    ASSERT_TRUE(result.secondEdgeIndex < inner.getEdges().size());
}

TEST(PolygonTest, DistanceToTouchingPolygonsIsZero)
{
	Polygon first({
		Point(0.0, 0.0),
		Point(5.0, 0.0),
		Point(5.0, 5.0),
		Point(0.0, 5.0)
		});
	Polygon second({
		Point(5.0, 0.0),
		Point(10.0, 0.0),
		Point(10.0, 5.0),
		Point(5.0, 5.0)
		});
	
    const auto result = first.distanceTo(second);

    EXPECT_NEAR(result.distance, 0.0, EPSILON);
    ASSERT_TRUE(result.firstEdgeIndex < first.getEdges().size());
    ASSERT_TRUE(result.secondEdgeIndex < second.getEdges().size());
}

TEST(PolygonTest, DistanceToTouchingVertexPolygonsIsZero)
{
    Polygon first({
        Point(0.0, 0.0),
        Point(5.0, 0.0),
        Point(5.0, 5.0),
        Point(0.0, 5.0)
        });
    Polygon second({
        Point(5.0, 5.0),
        Point(9.0, 5.0),
        Point(9.0, 9.0),
        Point(5.0, 9.0)
        });

    const auto result = first.distanceTo(second);

    EXPECT_NEAR(result.distance, 0.0, EPSILON);
    ASSERT_TRUE(result.firstEdgeIndex < first.getEdges().size());
    ASSERT_TRUE(result.secondEdgeIndex < second.getEdges().size());
}

TEST(PolygonTest, DistanceToConcavePolygon)
{
    Polygon first({
        Point(0.0, 11.0),
        Point(10.0, 11.0),
        Point(10.0, 15.0),
        Point(0.0, 16.0)
        });
    Polygon second({
        Point(0.0, 0.0),
        Point(5.0, 5.0),
        Point(10.0, 0.0),
        Point(10.0, 10.0),
        Point(0.0, 10.0)
        });

    const auto result = first.distanceTo(second);

    EXPECT_NEAR(result.distance, 1.0, EPSILON);
    ASSERT_TRUE(result.firstEdgeIndex < first.getEdges().size());
    ASSERT_TRUE(result.secondEdgeIndex < second.getEdges().size());
}

TEST(PolygonTest, CalculatesMinimumWidthOfRectangle)
{
    Polygon polygon({
        Point(0.0, 0.0),
        Point(12.0, 0.0),
        Point(12.0, 4.0),
        Point(0.0, 4.0)
        });
    const auto result = polygon.minWidth();
    const auto edges = polygon.getEdges();

    EXPECT_NEAR(result.distance, 4.0, EPSILON);
    EXPECT_NEAR(Point::vectorBetween(result.firstPoint, result.secondPoint).length(), result.distance, EPSILON);
    // First point returned must be on First edge returned
    EXPECT_TRUE(edges[result.firstEdgeIndex].contains(result.firstPoint));
    EXPECT_TRUE(edges[result.secondEdgeIndex].contains(result.secondPoint));
}

TEST(PolygonTest, CalculatesMinimumWidthOfTallRectangle)
{
    Polygon polygon({
        Point(0.0, 0.0),
        Point(3.0, 0.0),
        Point(3.0, 15.0),
        Point(0.0, 15.0)
        });

    EXPECT_NEAR(polygon.minWidth().distance, 3.0, EPSILON);
}

TEST(PolygonTest, CalculatesMinimumWidthOfSquare)
{
    Polygon polygon({
        Point(0.0, 0.0),
        Point(5.0, 0.0),
        Point(5.0, 5.0),
        Point(0.0, 5.0)
        });

    EXPECT_NEAR(polygon.minWidth().distance, 5.0, EPSILON);
}

TEST(PolygonTest, MinimumWidthDoesNotDependOnVertexOrientation)
{
    Polygon polygon({
        Point(0.0, 0.0),
        Point(0.0, 4.0),
        Point(12.0, 4.0),
        Point(12.0, 0.0)
        });

    EXPECT_NEAR(polygon.minWidth().distance, 4.0, EPSILON);
}

TEST(PolygonTest, MinimumWidthRejectsUnsupportedPolygon)
{
    Polygon polygon({
        Point(0.0, 0.0),
        Point(5.0, 0.0),
        Point(2.5, 5.0)
        });

    EXPECT_THROW(polygon.minWidth().distance, std::logic_error);
}

// TESTS for Orthogonal (Manhatan) Polygon Minimum Width Calculation

TEST(PolygonTest, CalculatesLShapeMinimumWidth)
{
    Polygon polygon({
        Point(0.0, 0.0),
        Point(4.0, 0.0),
        Point(4.0, 5.0),
        Point(10.0, 5.0),
        Point(10.0, 8.0),
        Point(0.0, 8.0)
        });

    EXPECT_NEAR(polygon.minWidth().distance, 3.0, EPSILON);
}

TEST(PolygonTest, CalculatesUShapeMinimumWidth)
{
	Polygon polygon({
		Point(0.0, 0.0),
		Point(10.0, 0.0),
		Point(10.0, 3.0),
		Point(4.0, 3.0),
		Point(4.0, 7.0),
		Point(10.0, 7.0),
		Point(10.0, 10.0),
		Point(0.0, 10.0)
		});
	EXPECT_NEAR(polygon.minWidth().distance, 3.0, EPSILON);
}

TEST(PolygonTest, CalculatesComplexOrthogonalPolygonMinimumWidth)
{
	Polygon polygon({
		Point(0.0, 0.0),
		Point(12.0, 0.0),
		Point(12.0, 3.0),
		Point(8.0, 3.0),
		Point(8.0, 6.0),
		Point(12.0, 6.0),
		Point(12.0, 9.0),
		Point(4.0, 9.0),
		Point(4.0, 6.0),
		Point(0.0, 6.0)
		});
	EXPECT_NEAR(polygon.minWidth().distance, 3.0, EPSILON);
}

TEST(PolygonTest, MinimumWidthRejectsNonOrthogonalPolygon)
{
	Polygon polygon({
        Point(1.0, 1.0),
        Point(5.0, 0.0),
        Point(8.0, 3.0),
        Point(6.0, 7.0),
        Point(3.0, 6.0),
        Point(0.0, 3.0)
		});
	EXPECT_THROW(polygon.minWidth().distance, std::logic_error);
}

TEST(PolygonTest, MinmumWidthPolygonWithANotch) {
	Polygon polygon({
        Point(0.0, 0.0),
        Point(0.0, 6.0),
        Point(6.0, 6.0),
        Point(6.0, 0.0),
        Point(4.0, 0.0),
        Point(4.0, 2.0),
        Point(2.0, 2.0),
        Point(2.0, 0.0)
		});
	EXPECT_NEAR(polygon.minWidth().distance, 2.0, EPSILON);
}

TEST(PolygonTest, MinimumWidthOfPolygonWithMultipleConcavities) {
	Polygon polygon({
        Point(0.0, 0.0),
        Point(0.0, 6.0),
        Point(2.0, 6.0),
        Point(2.0, 4.0),
        Point(3.0, 4.0),
        Point(3.0, 6.0),
        Point(5.0, 6.0),
        Point(5.0, 3.0),
        Point(4.0, 3.0),
        Point(4.0, 1.0),
        Point(6.0, 1.0),
        Point(6.0, 0.0)
		});
	EXPECT_NEAR(polygon.minWidth().distance, 1.0, EPSILON);
}

TEST(PolygonTest, MinimumWidthOfHPolygon) {
    Polygon polygon({
        Point(0.0, 0.0),
        Point(6.0, 0.0),
        Point(6.0, 2.0),
        Point(4.0, 2.0),
        Point(4.0, 4.0),
        Point(6.0, 4.0),
        Point(6.0, 6.0),
        Point(0.0, 6.0),
        Point(0.0, 4.0),
        Point(3.0, 4.0),
        Point(3.0, 2.0),
        Point(0.0, 2.0)
        });
    EXPECT_NEAR(polygon.minWidth().distance, 1.0, EPSILON);
}

TEST(PolygonTest, InnerPolygonCompletelyInsideConcavePolygon) {
    Polygon firstPolygon({
        Point(2.0, 4.0),
        Point(2.0, 6.0),
        Point(4.0, 6.0),
        Point(4.0, 4.0)
        });

    Polygon secondPolygon({
        Point(0.0, 0.0),
        Point(2.0, 0.0),
        Point(2.0, 2.0),
        Point(4.0, 2.0),
        Point(4.0, 0.0),
        Point(6.0, 0.0),
        Point(6.0, 8.0),
        Point(0.0, 8.0)
        });
    EXPECT_FALSE(firstPolygon.contains(secondPolygon));
    EXPECT_TRUE(secondPolygon.contains(firstPolygon));
}

// Tests for witness reporting (closest verticies and edges)

TEST(PolygonTest, DistanceBetweenSeparatedRectangles)
{
    Polygon first({
        Point(0, 0),
        Point(4, 0),
        Point(4, 4),
        Point(0, 4)
        });

    Polygon second({
        Point(7, 0),
        Point(10, 0),
        Point(10, 4),
        Point(7, 4)
        });

    const auto result = first.distanceTo(second);

    EXPECT_NEAR(result.distance, 3.0, EPSILON);
    EXPECT_LT(result.firstEdgeIndex, first.getEdges().size());
    EXPECT_LT(result.secondEdgeIndex, second.getEdges().size());
    EXPECT_NEAR(Point::vectorBetween(result.firstPoint,result.secondPoint).length(), 3.0, EPSILON);
    EXPECT_NEAR(result.firstPoint.getX(), 4.0, EPSILON);
    EXPECT_NEAR(result.secondPoint.getX(), 7.0, EPSILON);
}

TEST(PolygonTest, DistanceToContainedPolygonIsZeroWithWitness)
{
    Polygon outer({
        Point(0.0, 0.0),
        Point(10.0, 0.0),
        Point(10.0, 10.0),
        Point(0.0, 10.0)
        });
    Polygon inner({
        Point(4.0, 4.0),
        Point(8.0, 4.0),
        Point(8.0, 7.0),
        Point(4.0, 7.0)
        });

    const auto result = outer.distanceTo(inner);

    EXPECT_NEAR(result.distance, 0.0, EPSILON);
    EXPECT_LT(result.firstEdgeIndex, outer.getEdges().size());
    EXPECT_LT(result.secondEdgeIndex, inner.getEdges().size());
    EXPECT_NEAR(result.firstPoint.getX(), 10.0, EPSILON);
    EXPECT_NEAR(result.secondPoint.getX(), 8.0, EPSILON);
}

TEST(PolygonTest, DistanceToContainedPolygonValueWithWitness)
{
    Polygon outer({
        Point(0.0, 0.0),
        Point(10.0, 0.0),
        Point(10.0, 10.0),
        Point(0.0, 10.0)
        });
    Polygon inner({
        Point(4.0, 4.0),
        Point(8.0, 4.0),
        Point(8.0, 7.0),
        Point(4.0, 7.0)
        });

    const auto result = outer.distanceTo(inner, false);

    EXPECT_NEAR(result.distance, 2.0, EPSILON);
    EXPECT_LT(result.firstEdgeIndex, outer.getEdges().size());
    EXPECT_LT(result.secondEdgeIndex, inner.getEdges().size());
    EXPECT_NEAR(result.firstPoint.getX(), 10.0, EPSILON);
    EXPECT_NEAR(result.secondPoint.getX(), 8.0, EPSILON);
}

TEST(PolygonTest, SeparatedRectanglesWithUniqueWitness)
{
    Polygon first({
        Point(0, 0),
        Point(4, 0),
        Point(4, 4),
        Point(0, 4)
        });

    Polygon second({
        Point(1, 6),
        Point(3, 6),
        Point(3, 10),
        Point(1, 10)
        });

    const auto result = first.distanceTo(second);

    EXPECT_NEAR(result.distance, 2.0, EPSILON);
    EXPECT_EQ(result.firstEdgeIndex, 2);
    EXPECT_EQ(result.secondEdgeIndex, 0);
    EXPECT_NEAR(Point::vectorBetween(result.firstPoint, result.secondPoint).length(), 2.0, EPSILON);
    EXPECT_NEAR(result.firstPoint.getY(), 4.0, EPSILON);
    EXPECT_NEAR(result.secondPoint.getY(), 6.0, EPSILON);
}
