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

The project has completed the **geometry core**, **domain model**, initial **DRC rule layer**, **DRC engine orchestration**, **JSON input/output flow**, richer **violation metadata**, and the first **spatial-indexing optimization** using a QuadTree.

The current implementation can load layout shapes and rule definitions from JSON, run width, spacing, and enclosure checks through `DRCEngine`, record actual and required rule values in violations, write machine-readable JSON reports, and use a QuadTree to reduce candidate comparisons for minimum-spacing checks.

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
- Bounding-box containment
- Bounding-box merging
- Uniform bounding-box expansion

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
- Polygon containment, including concave-boundary crossing checks
- Polygon-to-polygon distance with configurable intersection handling
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
│       ├── domain/
│       │   ├── Layer.h
│       │   ├── Shape.h
│       │   └── Violation.h
│       │
│       ├── rules/
│       │   ├── Rule.h
│       │   ├── MinWidthRule.h
│       │   ├── MinSpacingRule.h
│       │   └── MinEnclosureRule.h
│       │
│       ├── io/
│       │   ├── JSONLayoutParser.h
│       │   ├── JSONRuleParser.h
│       │   └── JSONReportWriter.h
│       │
│       ├── engine/
│       │   └── DRCEngine.h
│       │
│       └── spatial/
│           └── QuadTree.h
│
├── src/
│   ├── geometry/
│   │   ├── Point.cpp
│   │   ├── Vector.cpp
│   │   ├── BoundingBox.cpp
│   │   ├── Segment.cpp
│   │   └── Polygon.cpp
│   │
│   ├── domain/
│   │   ├── Layer.cpp
│   │   ├── Shape.cpp
│   │   └── Violation.cpp
│   │
│   ├── rules/
│   │   ├── MinWidthRule.cpp
│   │   ├── MinSpacingRule.cpp
│   │   └── MinEnclosureRule.cpp
│   │
│   ├── io/
│   │   ├── JSONLayoutParser.cpp
│   │   ├── JSONRuleParser.cpp
│   │   └── JSONReportWriter.cpp
│   │
│   ├── engine/
│   │   └── DRCEngine.cpp
│   │
│   ├── spatial/
│   │   └── QuadTree.cpp
│   │
│   └── main.cpp
│
├── tests/
│   ├── geometry/
│   │   ├── PointTest.cpp
│   │   ├── VectorTest.cpp
│   │   ├── BoundingBoxTest.cpp
│   │   ├── SegmentTest.cpp
│   │   └── PolygonTest.cpp
│   │
│   ├── domain/
│   │   ├── ShapeTest.cpp
│   │   └── ViolationTest.cpp
│   │
│   ├── rules/
│   │   ├── MinWidthRuleTest.cpp
│   │   ├── MinSpacingRuleTest.cpp
│   │   └── MinEnclosureRuleTest.cpp
│   │
│   ├── io/
│   │   ├── JSONLayoutParserTest.cpp
│   │   ├── JSONRuleParserTest.cpp
│   │   └── JSONReportWriterTest.cpp
│   │
│   ├── engine/
│   │   └── DRCEngineTest.cpp
│   │
│   ├── spatial/
│   │   └── QuadTreeTest.cpp
│   │
│   └── integration/
│       └── EndToEndTest.cpp
│
├── examples/
│   ├── basic_layout.json
│   ├── cli_layout.json
│   ├── cli_multiple_shapes_layout.json
│   ├── cli_rules.json
│   ├── empty_layout.json
│   ├── invalid_polygon.json
│   ├── invalid_rules.json
│   ├── layer_parser.json
│   ├── report.json
│   └── rules.json
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
polygon.contains(otherPolygon);
polygon.intersects(otherPolygon);
polygon.distanceTo(otherPolygon);
polygon.distanceTo(otherPolygon, false);
polygon.minWidth();

boundingBox.overlaps(otherBoundingBox);
boundingBox.overlaps(otherBoundingBox, EPSILON);
boundingBox.contains(otherBoundingBox);
boundingBox.mergedWith(otherBoundingBox);
boundingBox.expanded(amount);
```

Internal implementation details are kept private where appropriate.

The public `Polygon::minWidth()` interface is intentionally kept general. The current implementation selects the orthogonal algorithm, while future non-Manhattan width support can be added internally without changing callers.

`Segment::intersects()` preserves its original behavior by default, including touching and collinear overlap.

`Polygon::distanceTo()` also preserves its original region-distance behavior by default. Passing `false` skips the polygon-level intersection shortcut and computes minimum edge-to-edge distance, which is used by minimum-enclosure checking.

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

`Polygon::contains(const Polygon&)` verifies that every vertex of the inner polygon lies inside the outer polygon and that no inner edge properly crosses the outer boundary. This is important for concave outer polygons, where vertex containment alone is not sufficient.

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

By default, if polygon regions intersect or one contains the other, distance is zero.

Otherwise, the minimum is taken over all edge-pair segment distances.

The method can also skip the polygon-level intersection shortcut. In that mode, it returns the minimum edge-to-edge distance even when one polygon contains the other. This is used to measure minimum enclosure.

This operation supports both the **minimum spacing** and **minimum enclosure** rules.

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

Layer-name conversion is centralized in the domain layer through `layerFromString()`, avoiding duplicated string-to-layer conversion logic across parsers.

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

The current representation stores:

- `ViolationType`
- Involved shape IDs
- Human-readable message
- Actual measured value
- Required rule value

`Violation::getTypeAsString()` provides the string representation used by CLI and JSON reporting without requiring output code to duplicate enum-conversion logic.

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

## Rule Layer

The rule layer is contained inside:

```cpp
namespace drcheck::rules
```

### `Rule`

`Rule` is an abstract base class with a common polymorphic interface:

```cpp
virtual std::vector<domain::Violation>
check(const std::vector<domain::Shape>& shapes) const = 0;
```

### `MinWidthRule`

- Applies to a selected layer
- Calls `Polygon::minWidth()`
- Compares actual width against the required minimum
- Produces a `MinWidth` violation for failing shapes

### `MinSpacingRule`

- Applies to shapes on the selected layer
- Builds a target-layer QuadTree from polygon bounding boxes
- Expands each shape's bounding box by the minimum-spacing requirement
- Queries the QuadTree for nearby candidate shapes
- Uses `Polygon::distanceTo()` for the exact spacing calculation
- Avoids self-pairs and duplicate A-B / B-A checks
- Produces violations containing both involved shape IDs, actual spacing, and required spacing

The QuadTree is used only as a broad-phase accelerator. Exact DRC correctness still comes from the polygon-distance calculation.

### `MinEnclosureRule`

- Applies between an inner layer and an outer layer
- Uses `Polygon::contains(const Polygon&)` to verify containment
- Uses `Polygon::distanceTo(other, false)` to measure edge-to-edge enclosure distance
- Stores actual and required enclosure values in violations
- Reports both inner and outer shape IDs when a containing outer shape exists but enclosure is insufficient
- Reports only the inner shape ID with actual enclosure `0.0` when no containing outer shape exists

The current implementation intentionally assumes that each inner shape is associated with at most one containing outer polygon.


---

## Spatial Indexing

### QuadTree

The project now includes a QuadTree under:

```cpp
namespace drcheck::spatial
```

The QuadTree partitions a 2D layout region into four recursive child regions and stores pointers to existing `Shape` objects instead of copying polygon geometry.

A shape is moved into a child only when its complete bounding box fits inside that child. Shapes that cross child boundaries remain in the parent node, ensuring each shape is stored exactly once.

Implemented behavior includes:

- Shape insertion
- Capacity-based subdivision
- Maximum-depth control
- Redistribution of stored shapes after subdivision
- Bounding-box range queries
- Rejection of shapes outside the root boundary

The root boundary is calculated dynamically from the actual target-layer geometry, so no fixed layout dimensions are required.

### QuadTree-Accelerated Spacing

```text
shape bounding box
      ↓
expand by minimum spacing
      ↓
QuadTree range query
      ↓
nearby candidates
      ↓
Polygon::distanceTo()
      ↓
exact spacing result
```

The QuadTree may return false-positive candidates, but the exact polygon-distance calculation still makes the final DRC decision. This reduces unnecessary comparisons for spatially distributed layouts, while worst-case layouts can still approach brute-force behavior.

---

## DRC Engine

`DRCEngine` orchestrates rule execution without implementing rule-specific geometry itself.

```text
vector<Shape>
      +
vector<unique_ptr<Rule>>
      ↓
   DRCEngine
      ↓
vector<Violation>
```

Each rule is executed polymorphically through the abstract `Rule` interface, and the engine appends all returned violations into one result vector.

Unsupported geometry errors remain exceptions rather than being silently converted into DRC violations.

---

## JSON Input

The project uses `nlohmann/json` for JSON parsing.

### `JSONLayoutParser`

`JSONLayoutParser` loads a flat list of polygon shapes and returns:

```cpp
std::vector<domain::Shape>
```

Example:

```json
{
  "shapes": [
    {
      "layer": "Metal1",
      "vertices": [
        [0, 0],
        [10, 0],
        [10, 5],
        [0, 5]
      ]
    }
  ]
}
```

The parser validates JSON structure while reusing existing geometry invariants from `Polygon`.

Shape IDs are currently generated automatically in import order (`0, 1, 2, ...`) rather than being required from the input file. This keeps the simplified JSON format compatible with future raw-layout import flows where source polygon IDs may not exist.

### `JSONRuleParser`

`JSONRuleParser` loads rule definitions and returns:

```cpp
std::vector<std::unique_ptr<rules::Rule>>
```

Supported rule types:

```text
MinWidth
MinSpacing
MinEnclosure
```

Example:

```json
{
  "rules": [
    {
      "type": "MinWidth",
      "layer": "Metal1",
      "value": 3.0
    },
    {
      "type": "MinSpacing",
      "layer": "Metal1",
      "value": 2.0
    },
    {
      "type": "MinEnclosure",
      "innerLayer": "Via12",
      "outerLayer": "Metal1",
      "value": 1.0
    }
  ]
}
```

Rule constructors remain responsible for validating rule-specific invariants.

The current file-based flow is:

```text
layout.json ──→ JSONLayoutParser ──→ vector<Shape>
                                         │
rules.json  ──→ JSONRuleParser ────→ vector<unique_ptr<Rule>>
                                         │
                                         ▼
                                     DRCEngine
                                         │
                                         ▼
                                 vector<Violation>
                                         │
                                         ▼
                                 JSONReportWriter
                                         │
                                         ▼
                                    report.json
```


---

## Command-Line Application

The `drcheck` executable connects the parsers, rule engine, and report writer into a complete application pipeline.

Typical usage:

```bash
drcheck <layout.json> <rules.json> <report.json>
```

The application loads the layout and rules, runs `DRCEngine`, writes the report, and prints a concise completion summary. Exceptions from parsing, invalid geometry, unsupported operations, or file I/O are caught at the `main()` application boundary.

A DRC violation is treated as a verification result, not as a program execution failure.

---

## JSON Violation Reporting

`JSONReportWriter` converts collected violations into a machine-readable JSON report.

Example:

```json
{
  "violationCount": 2,
  "violations": [
    {
      "type": "MinWidth",
      "shapeIds": [0],
      "actual": 2.0,
      "required": 3.0,
      "message": "Minimum width violation"
    },
    {
      "type": "MinSpacing",
      "shapeIds": [0, 1],
      "actual": 1.0,
      "required": 2.0,
      "message": "Minimum spacing violation"
    }
  ]
}
```

Clean layouts are also represented explicitly:

```json
{
  "violationCount": 0,
  "violations": []
}
```

The report now includes actual measured values and required rule values. Exact violation locations, offending edge pairs, and marker geometry are not yet stored.


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
- Bounding-box containment
- Boundary-touching containment
- Partial-containment rejection
- Bounding-box merging
- Bounding-box expansion
- Negative-expansion rejection

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
- `Violation` stores type, shape IDs, message, actual value, and required value correctly
- `Violation::getTypeAsString()` for all current violation types
- Layer string conversion through `layerFromString()`

### Rule Tests

#### `MinWidthRule`

- Width below, exactly at, and above the minimum
- Layer filtering
- Multiple shapes
- Orthogonal concave shapes
- Invalid rule parameters
- Polymorphic use through `Rule`

#### `MinSpacingRule`

- Spacing below, exactly at, and above the minimum
- Intersecting polygons
- Layer filtering
- Multiple shapes
- Duplicate-pair prevention
- Concave polygons
- Cross-quadrant candidate detection
- Parent-node crossing-shape detection
- Far-shape rejection
- Actual and required spacing metadata
- Invalid rule parameters
- Polymorphic use through `Rule`

#### `MinEnclosureRule`

- Exact minimum enclosure
- More than minimum enclosure
- Less than minimum enclosure
- Intersecting and internally touching polygons
- Inner polygon completely outside
- Concave outer polygon with valid enclosure
- Concave outer polygon with crossing geometry
- Wrong outer layer
- Multiple inner shapes
- Missing containing outer polygon
- Actual and required enclosure metadata
- Invalid rule parameters
- Polymorphic use through `Rule`

### QuadTree Tests

Coverage includes:

- Zero-capacity rejection
- Single-shape insertion and query
- Non-overlapping query regions
- Node subdivision
- Shape redistribution after subdivision
- Queries across different quadrants
- Boundary-crossing shapes retained at parent nodes
- Outside-root insertion rejection
- Maximum-depth behavior

### Engine Tests

Coverage includes:

- One rule / one violation
- Multiple rule types
- Multiple violations from one rule
- Empty rules
- Empty shapes
- Unified violation collection

### JSON Layout Parser Tests

Coverage includes:

- Valid layouts
- Multiple shapes
- Generated sequential shape IDs
- Supported layers
- Empty layout
- Missing or invalid `shapes`
- Missing required fields
- Malformed vertices
- Unknown layer rejection
- Invalid polygon geometry rejection
- Missing file handling
- Malformed JSON handling

### JSON Rule Parser Tests

Coverage includes:

- Valid `MinWidthRule`
- Valid `MinSpacingRule`
- Valid `MinEnclosureRule`
- Multiple rules
- Empty rule deck
- Unknown rule type rejection
- Unknown layer rejection
- Missing required fields
- Invalid rule values
- Missing file handling
- Malformed JSON handling
- Polymorphic behavior of parsed rules

### JSON Report Writer Tests

Coverage includes:

- Empty report generation
- Single and multiple violation serialization
- Multiple shape IDs
- Violation type string output
- Actual measured values
- Required rule values
- Message serialization
- Invalid output path handling
- Re-parsing generated JSON to validate semantic contents

### Integration Tests

Coverage includes the complete flow from JSON layout and rule files through parsing, `DRCEngine`, and JSON report generation.


---

## Building

Requirements:

- CMake 3.20+
- C++20 compiler
- Git / network access when dependencies are fetched through CMake
- GoogleTest for unit testing
- `nlohmann/json` for JSON parsing

GoogleTest and `nlohmann/json` are integrated through CMake `FetchContent`.

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

## Running DRCheck

```bash
drcheck <layout.json> <rules.json> <report.json>
```

Example:

```bash
drcheck ../../examples/basic_layout.json ../../examples/rules.json report.json
```

The executable prints a concise summary and writes detailed violations to the requested JSON report file.

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
├── getMinX()
├── getMinY()
├── getMaxX()
├── getMaxY()
├── overlaps(other, tolerance = 0.0)
├── contains(other)
├── mergedWith(other)
└── expanded(amount)

Segment
├── length()
├── getBoundingBox()
├── contains()
├── intersects(other)
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
├── contains(Point)
├── contains(Polygon)
├── intersects()
├── distanceTo(other, treatIntersectionAsZero = true)
└── minWidth()

Domain

Layer
├── Metal1
├── Metal2
├── Poly
├── Diffusion
├── Via12
└── layerFromString()

Shape
├── getId()
├── getLayer()
└── getPolygon()

Violation
├── getType()
├── getTypeAsString()
├── getShapeIds()
├── getMessage()
├── getActualValue()
└── getRequiredValue()

Rules

Rule
└── check(shapes)

MinWidthRule
├── check(shapes)
├── getMinimumWidth()
└── getLayer()

MinSpacingRule
├── check(shapes)
├── getLayer()
└── getMinimumSpacing()

MinEnclosureRule
├── check(shapes)
├── getInnerLayer()
├── getOuterLayer()
└── getMinimumEnclosure()

I/O

JSONLayoutParser
└── load(filePath) → vector<Shape>

JSONRuleParser
└── load(filePath) → vector<unique_ptr<Rule>>

JSONReportWriter
└── write(violations, filePath)

Engine

DRCEngine
└── run(shapes, rules) → vector<Violation>

Spatial

QuadTree
├── insert(shape)
└── query(region) → vector<const Shape*>

Application

drcheck
└── layout.json + rules.json → report.json
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

### Rule Layer

- Abstract `Rule` interface
- `MinWidthRule`
- `MinSpacingRule`
- `MinEnclosureRule`
- Rule-focused GoogleTest coverage
- Polymorphic execution through `Rule`

### Engine

- `DRCEngine`
- Multi-rule orchestration
- Unified violation collection
- Engine-focused GoogleTest coverage

### JSON Input

- `nlohmann/json` integration
- `JSONLayoutParser`
- Generated shape IDs
- Shared `layerFromString()` conversion
- `JSONRuleParser`
- Parser validation and tests

### Executable Integration

- `main.cpp`
- `drcheck` executable
- Command-line layout/rule/report paths
- Application-boundary exception handling
- Full parser-to-engine execution flow

### JSON Reporting

- `Violation::getTypeAsString()`
- Actual and required rule values
- `JSONReportWriter`
- Empty and non-empty reports
- Machine-readable violation output
- Reporting and end-to-end tests

### Spatial Indexing

- `BoundingBox::contains()`
- `BoundingBox::mergedWith()`
- `BoundingBox::expanded()`
- QuadTree insertion and subdivision
- Parent-node handling for boundary-crossing shapes
- Bounding-box range queries
- Dynamic root-boundary calculation
- QuadTree-backed `MinSpacingRule`
- Spatial-index regression tests

---

## Known Limitations

The current implementation intentionally prioritizes correctness, testability, and clear architecture over complete production-DRC coverage.

### Minimum Width

- `Polygon::minWidth()` and therefore `MinWidthRule` currently support **orthogonal (Manhattan) polygons only**.
- Non-orthogonal polygons, including 45° geometry, are not yet supported by the minimum-width algorithm.
- Unsupported non-orthogonal width checks throw `std::logic_error` rather than silently returning an incorrect result.

### Minimum Spacing

- `MinSpacingRule` now uses a QuadTree to reduce candidate comparisons before exact polygon-distance checking.
- QuadTree performance depends on spatial distribution; worst-case layouts can still approach brute-force behavior.
- The current implementation builds a QuadTree inside each `MinSpacingRule::check()` call rather than reusing a shared spatial index across rules.
- Current QuadTree parameters are initial fixed values rather than benchmark-tuned settings.

### Minimum Enclosure

- `MinEnclosureRule` currently assumes that an inner shape is associated with at most one containing outer polygon.
- Multiple containing outer candidates are not compared to select a best enclosure result.
- Two or more outer polygons are not combined or evaluated as a union for enclosure.
- If no containing outer polygon exists, the reported actual enclosure is currently `0.0`.

### Geometry Representation

- Coordinates currently use floating-point values with the project `EPSILON` tolerance policy.
- Integer database units may be adopted later for stronger IC-layout-style numerical robustness.

### Layout Input

- The current JSON layout format is flat and polygon-based.
- Hierarchical cells, paths, text, nets, and other GDSII concepts are not yet represented.
- Shape IDs are generated from import order, so IDs change if shape ordering changes.
- Direct GDSII parsing is not implemented yet.

### Rule Input

- JSON loading currently supports only `MinWidth`, `MinSpacing`, and `MinEnclosure`.
- Unknown rule types are rejected.
- Tcl-style rule decks are planned as a later extension.

### Violation Reporting

- Reports contain violation type, involved shape IDs, actual measured value, required value, and a human-readable message.
- Exact violation coordinates, offending edges, and marker geometry are not yet stored.
- Because shape IDs are generated from import order, report IDs are tied to the ordering of the imported layout.

---

## Next Development

### Spatial Performance Validation

The next performance milestone is to measure the value of the QuadTree rather than relying only on functional correctness.

Planned work:

- Create synthetic layouts with increasing shape counts
- Compare QuadTree-backed spacing checks against a brute-force baseline
- Measure candidate-count reduction and runtime
- Experiment with QuadTree capacity and maximum depth
- Document cases where spatial indexing helps and cases where it degrades toward worst-case behavior

### Future Performance Work

- Reusable/shared spatial indices across rules
- Sweep-line optimization where appropriate
- Additional spatial-query strategies
- Larger-layout benchmarks

### Future Geometry Extension

- Minimum-width support for non-Manhattan polygons
- 45° and arbitrary-angle support where required
- Preserve the public `Polygon::minWidth()` API

### Future Input / Integration

- Direct or intermediate GDSII import
- Optional Tcl rule decks
- More stable source-geometry identifiers where available
- Cross-platform build verification
- CI

### Future Reporting / Visualization

- Exact violation locations
- Offending edge-pair information
- Marker geometry
- Optional SVG visualization

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
