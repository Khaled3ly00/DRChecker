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

The project has completed the **geometry core / foundation phase** and has started the **domain model phase**.

The geometry core is now hardened, tested, and supports the main operations required by the upcoming DRC rule layer.

### Implemented Geometry

#### `Point`

- Coordinate storage
- Explicit proximity comparison through `isNear()`
- `vectorBetween()`
- `orientationValue()`
- Scale-aware `getOrientation()`

#### `Vector`

- Length
- Dot product
- Cross product

#### `BoundingBox`

- Axis-aligned bounding box representation
- Constructor-enforced coordinate ordering
- Exact overlap detection by default
- Optional tolerance for geometry broad-phase checks

#### `Segment`

- Endpoint storage
- Constructor rejection of degenerate and near-degenerate segments
- Segment length
- Bounding-box generation
- Point containment
- Segment intersection
- Point-to-segment distance
- Segment-to-segment distance
- Horizontal / vertical classification
- Private helper for proper segment intersection

#### `Polygon`

- Constructor-enforced validity:
  - At least three vertices
  - No zero-length edges
  - Nonzero area
  - No self-intersection
- Vertex storage
- Edge generation
- Signed area using the shoelace formula
- Polygon orientation
- Axis-aligned bounding box
- Point containment using ray casting
- Polygon-to-polygon intersection
- Polygon-to-polygon distance
- Minimum local width for orthogonal (Manhattan) polygons
- Rejection of unsupported non-orthogonal minimum-width calculations

---

## Project Structure

```text
drcheck/
├── CMakeLists.txt
│
├── include/
│   └── drcheck/
│       ├── geometry/
│       │   ├── Constants.h
│       │   ├── Point.h
│       │   ├── Vector.h
│       │   ├── BoundingBox.h
│       │   ├── Segment.h
│       │   └── Polygon.h
│       │
│       └── domain/
│           ├── Layer.h
│           ├── Shape.h
│           └── Violation.h
│
├── src/
│   ├── geometry/
│   │   ├── Point.cpp
│   │   ├── Vector.cpp
│   │   ├── BoundingBox.cpp
│   │   ├── Segment.cpp
│   │   └── Polygon.cpp
│   │
│   └── domain/
│       ├── Shape.cpp
│       └── Violation.cpp
│
├── tests/
│   ├── geometry/
│   │   ├── PointTest.cpp
│   │   ├── VectorTest.cpp
│   │   ├── BoundingBoxTest.cpp
│   │   ├── SegmentTest.cpp
│   │   └── PolygonTest.cpp
│   │
│   └── domain/
│       ├── ShapeTest.cpp
│       └── ViolationTest.cpp
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
polygon.minWidth();

boundingBox.overlaps(otherBoundingBox);
boundingBox.overlaps(otherBoundingBox, EPSILON);
```

Internal implementation details are kept private where appropriate.

The public `Polygon::minWidth()` interface is intentionally kept general. The current implementation selects the orthogonal algorithm, while future non-Manhattan width support can be added internally without changing callers.

---

## Floating-Point Tolerance Policy

`EPSILON` represents an absolute coordinate-distance tolerance.

- `Point::isNear()` compares coordinate differences explicitly instead of overloading mathematical equality.
- Point orientation scales the tolerance by the longest distance among the three input points before comparing it with the cross product.
- Polygon area validation scales the tolerance by polygon boundary length.
- `BoundingBox::overlaps()` remains exact by default.
- Geometry broad-phase checks pass `EPSILON` explicitly so near-boundary cases can still reach tolerant narrow-phase logic.

This keeps tolerance handling dimensionally consistent while preserving exact bounding-box behavior for callers that do not request tolerance.

---

## Polygon Validity Strategy

The project follows the design:

> A `Polygon` object should not exist in an invalid state.

The constructor enforces:

- At least three vertices
- No consecutive duplicate or near-duplicate vertices
- Nonzero area under the scale-aware area tolerance
- No self-intersection

Construction throws `std::invalid_argument` when an invariant is violated.

There is no public `isValid()` method.

---

## Point-in-Polygon

`Polygon::contains()` uses the **ray-casting algorithm**.

A horizontal ray is projected conceptually from the query point toward the right.

- Odd number of crossings: point is inside
- Even number of crossings: point is outside

Points on polygon edges or vertices are treated as inside. Near-boundary behavior is kept consistent with the geometry tolerance policy.

---

## Polygon Intersection

Two polygons are considered intersecting when:

1. Any edge from the first polygon intersects an edge from the second polygon, or
2. One polygon is completely contained inside the other.

A tolerance-aware bounding-box overlap check is performed first as a broad-phase rejection.

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

Projection determines whether the closest point lies:

- Before the segment start
- On the segment interior
- After the segment end

### Segment-to-Segment

If the segments intersect, distance is zero.

Otherwise, the minimum is taken from endpoint-to-segment distances.

### Polygon-to-Polygon

If polygons intersect, distance is zero.

Otherwise, the minimum is taken over all edge-pair segment distances.

This will support the future **minimum spacing DRC rule**.

---

## Minimum Width

`Polygon::minWidth()` now supports **orthogonal (Manhattan) polygons**, including concave rectilinear shapes.

The current algorithm:

1. Verifies that every polygon edge is horizontal or vertical.
2. Examines pairs of parallel edges.
3. Finds positive projection overlap between candidate boundaries.
4. Samples the region between the boundaries.
5. Uses `Polygon::contains()` to ensure the candidate represents material inside the polygon.
6. Computes the separation between valid opposing boundaries.
7. Returns the smallest valid candidate width.

This generalizes the earlier rectangle-only implementation.

Examples covered include:

- Rectangles
- L-shapes
- U-shapes
- Notched orthogonal polygons
- Clockwise and counterclockwise vertex ordering

### Non-Manhattan Geometry

The geometry model itself is **not restricted to Manhattan polygons**.

Real IC layouts may contain non-Manhattan geometry such as 45° edges, particularly in specialized routing, analog/RF structures, seal rings, or imported geometry.

The current orthogonal minimum-width algorithm does not attempt to handle these shapes.

For unsupported non-orthogonal polygons:

```cpp
polygon.minWidth();
```

currently throws `std::logic_error` rather than silently returning an incorrect value.

Future work can add a general minimum-width algorithm using edge directions, normals, projections, and appropriate local-width reasoning while preserving the public `minWidth()` API.

---

## Domain Model

The domain layer adds IC-layout meaning on top of the geometry layer.

### `Layer`

`Layer` is represented as a strongly typed `enum class`.

Current values include:

```cpp
Layer::Metal1
Layer::Metal2
Layer::Poly
Layer::Diffusion
Layer::Via12
```

`Via12` explicitly represents the via layer connecting Metal1 and Metal2.

### `Shape`

A `Shape` combines identity, layer information, and geometry:

```text
Shape
├── ID
├── Layer
└── Polygon
```

`Shape` uses **composition**, not inheritance.

A `Shape` has a `Polygon`; it is not a subclass of `Polygon`.

The initial API exposes:

- Shape ID
- Layer
- Read-only access to the owned polygon

No setters are currently required.

### `Violation`

`Violation` represents a detected DRC error.

The initial representation stores:

- `ViolationType`
- Involved shape IDs
- Human-readable message

Current violation types:

```cpp
ViolationType::MinWidth
ViolationType::MinSpacing
ViolationType::Enclosure
```

Shape IDs are stored instead of references or pointers, avoiding lifetime problems and simplifying future serialization.

### Dependency Direction

```text
Rules / Engine
      ↓
    Domain
      ↓
   Geometry
```

The geometry layer remains independent of domain concepts.

---

## Testing

GoogleTest is used for unit testing.

### Geometry Tests

Coverage includes:

#### Point

- Coordinate storage
- Proximity comparison
- Proximity symmetry
- Vector creation
- Clockwise / counterclockwise / collinear orientation
- Small-geometry orientation behavior

#### Vector

- Length
- Dot product
- Cross product

#### BoundingBox

- Overlap and separation
- Boundary touching
- Explicit tolerance
- Invalid coordinate ranges
- Negative-tolerance rejection

#### Segment

- Point containment
- Proper intersection
- Shared endpoints
- Collinear overlap / separation
- Degenerate and near-degenerate constructor rejection
- Intersection symmetry
- Point-to-segment distance
- Segment-to-segment distance
- Distance symmetry
- Horizontal / vertical classification

#### Polygon

- Area and signed area
- Orientation
- Bounding boxes
- Point containment
- Near-boundary containment
- Polygon intersection
- Intersection symmetry
- Polygon containment
- Small valid polygons
- Self-intersection rejection
- Polygon distance
- Distance symmetry
- Rectangle minimum-width regression
- L-shape minimum width
- U-shape minimum width
- Notch / concavity cases
- Clockwise orthogonal shapes
- Non-orthogonal `minWidth()` rejection

### Domain Tests

Coverage includes:

- `Shape` stores its ID, layer, and polygon correctly
- `Violation` stores type, shape IDs, and message correctly

---

## Building

Requirements:

- CMake 3.20+
- C++20 compiler
- Git, if GoogleTest is fetched through CMake

Configure:

```bash
cmake -S . -B build
```

Configure without tests:

```bash
cmake -S . -B build -DDRCHECK_BUILD_TESTS=OFF
```

Build:

```bash
cmake --build build
```

---

## Running Tests

Run all tests:

```bash
ctest --test-dir build --output-on-failure
```

Run a specific suite:

```bash
drchecker_tests --gtest_filter=PolygonTest.*
```

Run one test:

```bash
drchecker_tests --gtest_filter=SegmentTest.DetectsProperIntersection
```

---

## Current API Overview

```text
Geometry

Point
├── getX()
├── getY()
├── isNear()
├── static vectorBetween()
├── static orientationValue()
└── static getOrientation()

Vector
├── length()
├── dot()
└── cross()

BoundingBox
└── overlaps(other, tolerance = 0.0)

Segment
├── length()
├── getBoundingBox()
├── contains()
├── intersects()
├── distanceTo(Point)
├── distanceTo(Segment)
├── isHorizontal()
└── isVertical()

Polygon
├── getVertices()
├── getVertexCount()
├── getEdges()
├── signedArea()
├── getOrientation()
├── getBoundingBox()
├── contains()
├── intersects()
├── distanceTo(Polygon)
└── minWidth()

Domain

Layer
├── Metal1
├── Metal2
├── Poly
├── Diffusion
└── Via12

Shape
├── getId()
├── getLayer()
└── getPolygon()

Violation
├── getType()
├── getShapeIds()
└── getMessage()
```

---

## Completed Foundations

### Geometry Core

- Point / vector mathematics
- Segment intersection and distance
- Polygon area and containment
- Polygon intersection and distance
- Constructor invariants
- Scale-aware tolerance handling
- Orthogonal polygon minimum width
- GoogleTest regression coverage

### Domain Model

- `Layer`
- `Shape`
- `Violation`

---

## Next Development

### Rule Layer

Next planned work:

- Define the abstract `Rule` interface
- Implement `MinWidthRule`
- Apply rules to domain `Shape` objects
- Produce `Violation` objects
- Add rule-focused GoogleTests

### Later Rule Development

- `MinSpacingRule`
- `EnclosureRule`
- Rule creation / configuration
- `RuleFactory`

### Layout Loading

- JSON layout format
- `LayoutParser`
- Hand-crafted example layouts
- Known violation cases

### Rule Engine

- `DRCEngine`
- Rule orchestration
- Violation collection
- JSON reporting

### Performance

- Quadtree spatial index
- Nearby / range queries
- Replace brute-force pairwise checks
- Sweep-line optimization
- Benchmarks against brute force

### Future Geometry Extension

- Minimum-width support for non-Manhattan polygons
- 45° and arbitrary-angle edge support where required
- Keep the public `Polygon::minWidth()` API stable

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
