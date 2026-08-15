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

The project has completed the **geometry core**, **domain model**, initial **DRC rule layer**, **DRC engine orchestration**, **JSON input/output flow**, richer **violation metadata**, the first **spatial-indexing optimization** using a QuadTree, and **performance validation and parameter tuning** of the QuadTree-backed spacing check against a brute-force baseline.

The current implementation can load layout shapes and rule definitions from JSON, run width, spacing, and enclosure checks through `DRCEngine`, record actual and required rule values in violations, write machine-readable JSON reports, and use a QuadTree to reduce candidate comparisons for minimum-spacing checks. Release-mode dense and sparse benchmarks have been completed, followed by a 16-configuration QuadTree parameter sweep. The selected balanced defaults are `capacity = 16` and `maxDepth = 8`.

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
├── benchmarks/
│   ├── MinSpacingBenchmark.cpp
│   └── results/
│       ├── min_spacing_dense_benchmark.txt
│       ├── min_spacing_sparse_benchmark.txt
│       └── quadtree_parameter_tuning.txt
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

After benchmark-based parameter tuning, the current default configuration used by `MinSpacingRule` is:

```text
QuadTree Capacity: 16
QuadTree Max Depth: 8
```

These values are implementation/performance parameters, not design-rule parameters.

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

## Performance Benchmarking

A dedicated benchmark executable compares the original brute-force minimum-spacing candidate enumeration with the current QuadTree-backed implementation.

The exact geometry operation remains unchanged in both paths:

```text
Brute force:
all unique shape pairs
        ↓
Polygon::distanceTo()

QuadTree:
expanded bounding-box query
        ↓
nearby candidate pairs
        ↓
Polygon::distanceTo()
```

This keeps the comparison focused on candidate selection rather than changing DRC semantics.

### Measurements

Each benchmark run records:

- Number of shapes
- Exact polygon-pair checks
- Number of detected violations
- Total runtime
- Pair-check reduction
- Speedup relative to brute force

The QuadTree timing includes root-boundary calculation, tree construction, shape insertion, range queries, and exact polygon-distance checks.

Correctness is checked by requiring the brute-force and QuadTree implementations to report the same violation count.

### Scenario 1 — Dense Grid

Configuration:

```text
Build Type: Release
Shape Size: 10 × 10
Gap: 2
Minimum Spacing: 3
QuadTree Capacity: 4
QuadTree Max Depth: 8
```

Measured results:

| Shapes | Brute Pair Checks | QuadTree Pair Checks | Pair Reduction | Brute Time (ms) | QuadTree Time (ms) | Speedup |
|---:|---:|---:|---:|---:|---:|---:|
| 100 | 4,950 | 342 | 93.09% | 51.37 | 4.10 | 12.52× |
| 500 | 124,750 | 1,868 | 98.50% | 1,452.57 | 23.63 | 61.47× |
| 1,000 | 499,500 | 3,811 | 99.24% | 4,704.80 | 43.76 | 107.52× |
| 2,000 | 1,999,000 | 7,733 | 99.61% | 18,048.81 | 121.71 | 148.30× |

For every tested size, the brute-force and QuadTree implementations reported the same violation count, confirming that the spatial filtering preserved the benchmarked spacing-check results.

For this dense regular grid, the QuadTree greatly reduces exact `Polygon::distanceTo()` calls as layout size increases. The measured speedup also grows with shape count because brute-force candidate enumeration grows quadratically while the spatial query limits exact checks to nearby candidates.

### Scenario 2 — Sparse Grid

The sparse scenario uses the same benchmark framework, QuadTree settings, and minimum spacing, but spaces shapes far enough apart that no spacing violations occur.

Measured results:

| Shapes | Brute Pair Checks | QuadTree Pair Checks | Pair Reduction | Brute Time (ms) | QuadTree Time (ms) | Speedup |
|---:|---:|---:|---:|---:|---:|---:|
| 100 | 4,950 | 0 | 100.00% | 51.62 | 0.44 | 116.87× |
| 500 | 124,750 | 0 | 100.00% | 1,233.44 | 4.03 | 305.99× |
| 1,000 | 499,500 | 0 | 100.00% | 5,218.15 | 3.95 | 1,321.29× |
| 2,000 | 1,999,000 | 0 | 100.00% | 18,352.65 | 39.05 | 470.00× |

The brute-force path still evaluates every unique pair even though no shapes are close enough to violate the rule. The QuadTree rejects all non-nearby pairs during the bounding-box query stage, so no exact `Polygon::distanceTo()` calls are required in this scenario.

Across both completed scenarios, the brute-force and QuadTree implementations produced matching violation counts for every tested shape count. This confirms that the spatial optimization preserved the benchmarked spacing-check results while substantially reducing exact geometry work.

The two benchmark scenarios are sufficient for the current layout-density comparison: the dense grid demonstrates performance when nearby shapes produce real spacing violations, while the sparse grid demonstrates the benefit when shapes are spatially separated. No additional layout scenarios are required for the current project plan.

### QuadTree Parameter Tuning

After validating the dense and sparse scenarios, the QuadTree configuration was tuned using:

```text
Shape Count: 2000
Minimum Spacing: 3
Repetitions Per Configuration: 5

Capacity values: 2, 4, 8, 16
Max-depth values: 4, 6, 8, 10
Total configurations: 16
```

The existing `capacity = 4`, `maxDepth = 8` configuration was used as the correctness reference. Every tested configuration preserved:

```text
Dense Pair Checks: 7733
Dense Violations: 7733

Sparse Pair Checks: 0
Sparse Violations: 0
```

Therefore, the parameter sweep changed performance only and did not change the benchmarked DRC results.

Measured average QuadTree runtimes:

| Capacity | Max Depth | Dense Avg. Time (ms) | Sparse Avg. Time (ms) |
|---:|---:|---:|---:|
| 2 | 4 | 144.90 | 39.57 |
| 2 | 6 | 224.86 | 40.99 |
| 2 | 8 | 126.17 | 40.90 |
| 2 | 10 | 129.53 | 40.62 |
| 4 | 4 | 124.47 | **37.89** |
| 4 | 6 | 122.89 | 40.52 |
| 4 | 8 | 124.14 | 38.72 |
| 4 | 10 | 127.00 | 38.61 |
| 8 | 4 | 125.65 | 38.63 |
| 8 | 6 | **121.16** | 39.34 |
| 8 | 8 | 123.75 | 40.49 |
| 8 | 10 | 123.99 | 38.74 |
| 16 | 4 | 125.86 | 42.10 |
| 16 | 6 | 121.52 | 38.86 |
| **16** | **8** | **122.02** | **38.43** |
| 16 | 10 | 122.42 | 38.90 |

The absolute fastest dense result was `capacity = 8`, `maxDepth = 6`, while the absolute fastest sparse result was `capacity = 4`, `maxDepth = 4`.

The selected default is:

```text
capacity = 16
maxDepth = 8
```

This configuration was selected as a balanced choice across both workloads: it was within about 0.71% of the fastest dense result and about 1.42% of the fastest sparse result, while improving both measured workloads relative to the original `4 / 8` baseline.

The tuning also demonstrates that more aggressive subdivision is not automatically faster. For example, `capacity = 2`, `maxDepth = 6` was substantially slower on the dense layout than the better-balanced configurations.

### Benchmark Driver Workflow

`benchmarks/MinSpacingBenchmark.cpp` is a dedicated experimental benchmark driver, not a fixed production entry point.

Its helper functions such as grid generation and QuadTree/brute-force measurement are reused, while its `main()` function is intentionally changed according to the benchmarking task being performed.

Examples:

```text
Dense / sparse scenario benchmarking
→ main() generates layouts of several sizes
→ compares brute force against QuadTree
→ records pair reduction and speedup

QuadTree parameter tuning
→ main() generates the selected benchmark layouts
→ runs capacity / maxDepth combinations
→ averages repeated QuadTree timings
→ validates unchanged pair checks and violations
```

This keeps performance experiments isolated from the production `drcheck` application while allowing the benchmark executable to evolve with the current measurement task.

These measurements are specific to the benchmark geometries, machine, Release build, and current implementation. They are not a universal QuadTree speedup guarantee.

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

## Running Benchmarks

Build the benchmark in Release mode:

```bash
cmake --build build --config Release
```

Run:

```bash
drcheck_benchmark
```

Recorded benchmark results are stored under:

```text
benchmarks/results/
├── min_spacing_dense_benchmark.txt
├── min_spacing_sparse_benchmark.txt
└── quadtree_parameter_tuning.txt
```

`MinSpacingBenchmark.cpp` is intentionally task-oriented: its `main()` is modified depending on whether the executable is being used for scenario benchmarking or QuadTree parameter tuning. The reusable benchmark helpers remain the same while `main()` defines the current experiment.

Release builds should be used for performance comparisons because Debug-mode timings are not representative.

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

Benchmark

drcheck_benchmark
├── dense / sparse brute-force vs QuadTree comparison
├── QuadTree parameter tuning
└── task-specific MinSpacingBenchmark.cpp main()
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

### Performance Benchmarking

- Dedicated `drcheck_benchmark` executable
- Synthetic grid-layout generation
- Brute-force spacing baseline
- QuadTree spacing benchmark
- Exact pair-check counting
- Runtime measurement with `std::chrono::steady_clock`
- Correctness comparison through matching violation counts
- File-based benchmark result output
- Dense and sparse Release-mode benchmark scenarios completed
- Dense scenario: up to 99.61% reduction in exact pair checks and 148.30× measured speedup at 2,000 shapes
- Sparse scenario: 100% reduction in exact pair checks for all tested sizes and up to 1,321.29× measured speedup
- Matching violation counts between brute-force and QuadTree implementations in every recorded benchmark
- 16-configuration QuadTree parameter sweep completed
- Five timing repetitions per tuning configuration
- Selected balanced defaults: `capacity = 16`, `maxDepth = 8`
- Task-specific benchmark `main()` used for scenario comparison and parameter tuning

---

## Known Limitations

The current implementation intentionally prioritizes correctness, testability, and clear architecture over complete production-DRC coverage.

### Minimum Width

- `Polygon::minWidth()` and therefore `MinWidthRule` currently support **orthogonal (Manhattan) polygons only**.
- Non-orthogonal polygons, including 45° geometry, are not yet supported by the minimum-width algorithm.
- Unsupported non-orthogonal width checks throw `std::logic_error` rather than silently returning an incorrect result.

### Minimum Spacing

- `MinSpacingRule` uses a QuadTree to reduce candidate comparisons before exact polygon-distance checking.
- Dense and sparse synthetic-grid benchmarks both show substantial improvement over the brute-force baseline while preserving violation counts.
- Benchmark results remain workload-specific; different geometry distributions may produce different speedups.
- QuadTree performance depends on spatial distribution, and pathological layouts can still degrade toward brute-force behavior.
- The current implementation builds a QuadTree inside each `MinSpacingRule::check()` call rather than reusing a shared spatial index across rules.
- The current QuadTree defaults (`capacity = 16`, `maxDepth = 8`) were selected from the completed dense/sparse parameter sweep; they are benchmark-informed defaults rather than universal optima.

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

### Future Performance Work

- Reusable/shared spatial indices across rules
- Sweep-line optimization where appropriate
- Additional spatial-query strategies if future project requirements justify them

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
