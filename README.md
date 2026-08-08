# DRCheck

A C++ Design Rule Checker (DRC) project for simplified IC layouts.

The project is designed as a portfolio project focused on:
- Modern C++
- Object-oriented design
- Computational geometry
- Unit testing with GoogleTest
- Spatial indexing and performance optimization
- EDA / IC verification concepts

## Current Status

The project is currently in the geometry foundation phase.

Implemented geometry concepts:

- `Point`
  - Coordinate storage
  - `vectorBetween()`
  - `orientationValue()`
  - `getOrientation()`

- `Vector`
  - Length
  - Dot product
  - Cross product

- `BoundingBox`
  - Axis-aligned bounding box representation
  - Bounding-box overlap detection

- `Segment`
  - Endpoint storage
  - Bounding-box generation
  - Point containment
  - Segment intersection
  - Proper-intersection helper hidden through encapsulation

- `Polygon`
  - Vertex storage
  - Constructor validation
  - Edge generation
  - Signed area using the shoelace formula
  - Absolute area
  - Polygon orientation
  - Axis-aligned bounding box
  - Point containment using ray casting

## Project Structure

```text
DRChecker/
├── CMakeLists.txt
├── include/
│   └── drcheck/
│       └── geometry/
│           ├── Point.h
│           ├── Vector.h
│           ├── BoundingBox.h
│           ├── Segment.h
│           └── Polygon.h
│           └── Constants.h
├── src/
│   └── geometry/
│       ├── Point.cpp
│       ├── Vector.cpp
│       ├── BoundingBox.cpp
│       ├── Segment.cpp
│       └── Polygon.cpp
├── tests/
│   └── geometry/
│       ├── PointTest.cpp
│       ├── VectorTest.cpp
│       ├── BoundingBoxTest.cpp
│       ├── SegmentTest.cpp
│       └── PolygonTest.cpp
└── README.md
```

## Geometry Design

The geometry layer is contained inside:

```cpp
namespace drcheck::geometry
```

The project uses classes to keep behavior close to the object it belongs to.

Examples:

```cpp
segment.intersects(otherSegment);
polygon.contains(point);
polygon.area();
boundingBox.overlaps(otherBoundingBox);
```

Internal implementation details are kept private where appropriate.

For example, `Segment::properIntersection()` is private because users of the class only need to ask whether two segments intersect.

## Polygon Point Containment

`Polygon::contains()` uses the ray-casting algorithm.

A horizontal ray is conceptually projected from the query point to the right.

Each time the ray crosses a polygon edge, the crossing count is toggled.

- Odd number of crossings: point is inside
- Even number of crossings: point is outside

Points lying directly on polygon edges are treated as inside.

## Building

Requirements:

- CMake 3.20+
- C++17 compiler
- Git, if GoogleTest is fetched through CMake

Configure:

```bash
cmake -S . -B build
```

Build:

```bash
cmake --build build
```

## Testing

GoogleTest is used for unit testing.

Run all tests through CTest:

```bash
ctest --test-dir build --output-on-failure
```

You can also run only selected GoogleTest suites, for example:

```bash
drcheck_geometry_tests --gtest_filter=PolygonTest.*
```

Current geometry tests should cover normal and boundary cases for:

- Point orientation
- Vector operations
- Bounding-box overlap
- Segment containment and intersection
- Polygon area
- Polygon orientation
- Polygon bounding box
- Polygon point containment

## Planned Development

### Geometry
- Polygon-to-polygon intersection
- Point-to-segment distance
- Segment-to-segment distance
- Polygon-to-polygon distance
- Minimum width calculations

### DRC Domain Layer
- Layers
- Shapes
- Violations

### Rule Engine
- Minimum width rule
- Minimum spacing rule
- Enclosure rule
- Rule abstraction and factory

### Performance
- Quadtree spatial index
- Sweep-line optimization
- Benchmarking against brute-force checking

### I/O and Integration
- JSON layout loading
- JSON violation reports
- Optional SVG visualization
- Optional Tcl rule decks

## Goal

The final project will load simplified IC layout geometry, apply design rules, report violations, and compare brute-force checking against spatially optimized implementations.
