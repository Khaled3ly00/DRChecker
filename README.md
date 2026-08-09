# DRCheck

A C++ Design Rule Checker (DRC) project for simplified IC layouts.

The project is being developed as a portfolio project focused on:

- Modern C++
- Object-oriented design
- Computational geometry
- Unit testing with GoogleTest
- Spatial indexing and performance optimization
- EDA / IC verification concepts

---

## Current Status

The project is currently in the **geometry core / foundation phase**.

The main geometry primitives and algorithms required for later DRC rules are now implemented.

### Implemented Geometry

#### `Point`

- Coordinate storage
- `vectorBetween()`
- `orientationValue()`
- `getOrientation()`

The reusable point-level geometry methods are implemented as `static` member functions.

#### `Vector`

- Length
- Dot product
- Cross product

#### `BoundingBox`

- Axis-aligned bounding box representation
- Bounding-box overlap detection

#### `Segment`

- Endpoint storage
- Segment length
- Bounding-box generation
- Point containment
- Segment intersection
- Point-to-segment distance
- Segment-to-segment distance
- Private helper for proper segment intersection

#### `Polygon`

- Constructor-enforced validity for minimum vertex count
- Vertex storage
- Edge generation
- Signed area using the shoelace formula
- Absolute area
- Polygon orientation
- Axis-aligned bounding box
- Point containment using ray casting
- Polygon-to-polygon intersection
- Polygon-to-polygon distance
- Minimum width for axis-aligned rectangles

---

## Project Structure

```text
drcheck/
├── CMakeLists.txt
│
├── include/
│   └── drcheck/
│       └── geometry/
│           ├── Point.h
│           ├── Vector.h
│           ├── BoundingBox.h
│           ├── Segment.h
│           └── Polygon.h
│           └── Constants.h
│
├── src/
│   └── geometry/
│       ├── Point.cpp
│       ├── Vector.cpp
│       ├── BoundingBox.cpp
│       ├── Segment.cpp
│       └── Polygon.cpp
│
├── tests/
│   └── geometry/
│       ├── PointTest.cpp
│       ├── VectorTest.cpp
│       ├── BoundingBoxTest.cpp
│       ├── SegmentTest.cpp
│       └── PolygonTest.cpp
│
│
└── README.md
```

---

## Geometry Design

The geometry layer is contained inside:

```cpp
namespace drcheck::geometry
```

The project uses classes to keep behavior close to the object it belongs to.

Examples:

```cpp
segment.intersects(otherSegment);
segment.distanceTo(point);
segment.distanceTo(otherSegment);

polygon.contains(point);
polygon.intersects(otherPolygon);
polygon.distanceTo(otherPolygon);
polygon.area();
polygon.minWidth();

boundingBox.overlaps(otherBoundingBox);
```

Internal implementation details are kept private where appropriate.

For example, `Segment::properIntersection()` is private because callers only need to know whether two segments intersect.

Likewise, the current rectangle check used by `Polygon::minWidth()` is an implementation detail.

---

## Polygon Validity Strategy

The project currently follows the design:

> A `Polygon` object should not exist with fewer than three vertices.

The constructor validates the minimum vertex count and throws if the input does not satisfy that invariant.

There is currently no public `isValid()` method.

More advanced validity checks such as self-intersection or duplicate consecutive vertices may be added later if needed.

---

## Point-in-Polygon

`Polygon::contains()` uses the **ray-casting algorithm**.

A horizontal ray is conceptually projected from the query point toward the right.

Each time the ray crosses a polygon edge, the inside state toggles.

- Odd number of crossings: point is inside
- Even number of crossings: point is outside

Points lying directly on polygon edges or vertices are treated as inside.

---

## Polygon Intersection

Two polygons are considered intersecting when:

1. Any edge from the first polygon intersects an edge from the second polygon, or
2. One polygon is completely contained inside the other.

A bounding-box overlap check is performed first as a cheap broad-phase rejection.

This handles:

- Proper overlap
- Boundary crossing
- Shared edges
- Shared vertices
- One polygon fully contained in another

---

## Geometry Distance

Distance calculations are built incrementally:

```text
Point → Segment
        ↓
Segment → Segment
        ↓
Polygon → Polygon
```

### Point-to-Segment

Projection is used to determine whether the closest point lies:

- Before the segment start
- On the segment interior
- After the segment end

### Segment-to-Segment

If the segments intersect, the distance is zero.

Otherwise, the minimum distance is found from the four endpoint-to-segment cases.

### Polygon-to-Polygon

If the polygons intersect, the distance is zero.

Otherwise, the minimum is taken over all edge-pair segment distances.

This operation will later support the **minimum spacing DRC rule**.

---

## Minimum Width

`Polygon::minWidth()` currently supports only **axis-aligned rectangles**.

The current implementation:

1. Verifies that the polygon has exactly four vertices.
2. Verifies that every edge is horizontal or vertical.
3. Rejects zero-length edges.
4. Verifies that horizontal and vertical edges alternate.
5. Uses the lengths of two adjacent rectangle edges.
6. Returns the shorter of those two lengths.

Example:

```text
+------------------+
|                  |
|                  |  4
|                  |
+------------------+
        12
```

The minimum width is:

```text
4
```

This implementation is intentionally limited.

For general orthogonal polygons, minimum-width checking will later require examining relevant opposing boundaries rather than simply taking the shortest boundary edge.

---

## Testing

GoogleTest is used for unit testing.

Tests are organized by geometry class.

Current test areas include:

### Point

- Coordinate storage
- Vector creation
- Clockwise orientation
- Counterclockwise orientation
- Collinear points

### Vector

- Length
- Dot product
- Cross product

### BoundingBox

- Overlap
- No overlap
- Boundary touching
- Width
- Height

### Segment

- Point containment
- Non-collinear point rejection
- Collinear outside-point rejection
- Proper intersection
- Shared endpoint
- Collinear overlap
- Collinear separation
- Degenerate segment behavior
- Point-to-segment distance
- Segment-to-segment distance
- Distance symmetry

### Polygon

- Rectangle area
- Signed area
- Clockwise orientation
- Counterclockwise orientation
- Bounding box
- Point containment
- Boundary-point containment
- Polygon intersection
- Polygon containment intersection case
- Polygon distance
- Distance symmetry
- Minimum rectangle width
- Clockwise rectangle minimum width
- Unsupported polygon rejection for `minWidth()`

---

## Building

Requirements:

- CMake 3.20+
- C++17 compiler
- Git, if GoogleTest is fetched through CMake

Configure the project:

```bash
cmake -S . -B build
```

Build:

```bash
cmake --build build
```

---

## Running Tests

Run all tests through CTest:

```bash
ctest --test-dir build --output-on-failure
```

GoogleTest filtering can also be used.

Run all polygon tests:

```bash
drcheck_geometry_tests --gtest_filter=PolygonTest.*
```

Run one specific test:

```bash
drcheck_geometry_tests --gtest_filter=SegmentTest.DetectsProperIntersection
```

---

## Current Geometry API

```text
Point
├── getX()
├── getY()
├── static vectorBetween()
├── static orientationValue()
└── static getOrientation()

Vector
├── length()
├── dot()
└── cross()

BoundingBox
└── overlaps()

Segment
├── length()
├── boundingBox()
├── contains()
├── intersects()
├── distanceTo(Point)
└── distanceTo(Segment)

Polygon
├── getVertices()
├── vertexCount()
├── getEdges()
├── signedArea()
├── area()
├── orientation()
├── boundingBox()
├── contains()
├── intersects()
├── distanceTo(Polygon)
└── minWidth()
```

---

## Planned Development

### Geometry Hardening

Before moving fully into the engine:

- Review epsilon handling
- Review degenerate geometry cases
- Strengthen constructor invariants where useful
- Expand GoogleTest edge-case coverage
- Later generalize minimum-width handling beyond rectangles

### Domain Layer

Implement:

- `Layer`
- `Shape`
- `Violation`

### Layout Loading

- JSON layout format
- `LayoutParser`
- Hand-crafted example layouts
- Known violation test cases

### Rule Engine

Implement:

- Abstract `Rule` interface
- `MinWidthRule`
- `MinSpacingRule`
- `EnclosureRule`
- `RuleFactory`
- `DRCEngine`

### Performance

- Quadtree spatial index
- Range / nearby queries
- Replace brute-force pairwise shape comparisons
- Sweep-line optimization
- Benchmarking against brute force

### I/O and Integration

- JSON violation reports
- Optional SVG visualization
- Optional Tcl rule decks
- Cross-platform build verification
- CI

---

## Long-Term Goal

The final tool will:

1. Load simplified IC layout geometry.
2. Build an in-memory layout representation.
3. Apply configurable design rules.
4. Detect geometric violations.
5. Generate violation reports.
6. Compare brute-force checking against spatially optimized implementations.

The project is intended to demonstrate both **EDA-domain understanding** and **strong C++ software design**, including OOP, testing, algorithms, data structures, and performance analysis.
