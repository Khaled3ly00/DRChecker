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

The project has completed the **geometry core**, **domain model**, initial **DRC rule layer**, **DRC engine orchestration**, and the first **JSON input parsers**.

The current implementation can load layout shapes and rule definitions from JSON, build the corresponding domain and rule objects, run width, spacing, and enclosure checks through `DRCEngine`, and return a unified collection of violations.

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
│       │   └── JSONRuleParser.h
│       │
│       └── engine/
│           └── DRCEngine.h
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
│   │   └── JSONRuleParser.cpp
│   │
│   └── engine/
│       └── DRCEngine.cpp
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
│   │   └── JSONRuleParserTest.cpp
│   │
│   └── engine/
│       └── DRCEngineTest.cpp
│
├── examples/
│   ├── basic_layout.json
│   ├── empty_layout.json
│   ├── invalid_polygon.json
│   ├── invalid_rules.json
│   ├── layer_parser.json
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

- Applies to unique pairs of shapes on the selected layer
- Uses `Polygon::distanceTo()`
- Produces violations containing both involved shape IDs
- Currently uses brute-force pairwise checking as the correctness baseline

### `MinEnclosureRule`

- Applies between an inner layer and an outer layer
- Uses `Polygon::contains(const Polygon&)` to verify containment
- Uses `Polygon::distanceTo(other, false)` to measure edge-to-edge enclosure distance
- Produces an enclosure violation when no valid outer polygon provides the required enclosure


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

The current in-memory flow is:

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
```


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
- Multiple outer candidates
- Multiple inner shapes
- Invalid rule parameters
- Polymorphic use through `Rule`

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
├── getShapeIds()
└── getMessage()

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

Engine

DRCEngine
└── run(shapes, rules) → vector<Violation>
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

---

## Known Limitations

The current implementation intentionally prioritizes correctness, testability, and clear architecture over complete production-DRC coverage.

### Minimum Width

- `Polygon::minWidth()` and therefore `MinWidthRule` currently support **orthogonal (Manhattan) polygons only**.
- Non-orthogonal polygons, including 45° geometry, are not yet supported by the minimum-width algorithm.
- Unsupported non-orthogonal width checks throw `std::logic_error` rather than silently returning an incorrect result.

### Minimum Spacing

- `MinSpacingRule` currently uses brute-force pairwise shape checking.
- Spatial indexing and optimized candidate filtering are planned later.

### Minimum Enclosure

- `MinEnclosureRule` evaluates enclosure against **individual outer polygons**.
- If two separate outer polygons both contain the same inner polygon, the current rule does **not** combine or jointly evaluate them as a single enclosing structure.
- The rule accepts an inner shape once one individual outer polygon satisfies the required enclosure.
- Richer violation metadata identifying the best or nearest failed outer candidate is not yet implemented.

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

---

## Next Development

### Executable / End-to-End Integration

The next milestone is to connect the parsers and engine through a real executable.

Planned flow:

```text
layout.json
    ↓
JSONLayoutParser

rules.json
    ↓
JSONRuleParser

        ↓
    DRCEngine
        ↓
violations
```

Planned work:

- Add `main.cpp`
- Accept layout and rule file paths
- Run the full file-to-DRC pipeline
- Print or serialize violations
- Add end-to-end example inputs

### Reporting

- JSON violation report writer
- More structured violation metadata where useful
- Optional SVG visualization

### Performance

- Quadtree spatial index
- Nearby / range queries
- Replace brute-force pairwise spacing checks
- Sweep-line optimization
- Benchmarks against brute force

### Future Geometry Extension

- Minimum-width support for non-Manhattan polygons
- 45° and arbitrary-angle support where required
- Preserve the public `Polygon::minWidth()` API

### Future Input / Integration

- Direct or intermediate GDSII import
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
