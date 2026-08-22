# DRCheck

DRCheck is a C++ Design Rule Checker for simplified IC layouts. The project focuses on modern C++, computational geometry, IC verification concepts, unit testing, spatial indexing, and evidence-based performance optimization.

The current end-to-end application can:

- load layout geometry and rule definitions from JSON;
- run minimum-width, minimum-spacing, minimum-enclosure, and per-layer minimum/maximum density rules;
- build one shared, layer-aware spatial index for a rule run;
- report actual and required rule values;
- attach optional point/edge or region-based witness geometry to violations;
- write machine-readable JSON reports; and
- render SVG layouts with highlighted violation geometry, including density regions.

## Current Status

The geometry core, domain model, rule framework, JSON input/output path, CLI, SVG visualization, violation metadata, QuadTree optimization, reusable spatial-index architecture, and density-rule pipeline are implemented and covered by tests.

`DRCEngine` builds one `LayerSpatialIndex` from the current shape collection and passes it to every rule. `MinSpacingRule`, `MinEnclosureRule`, and `DensityRule` reuse that index for layer-specific candidate discovery instead of building rule-local trees or scanning every possible shape.

Detailed benchmark methodology, results, and QuadTree tuning data have moved to [BENCHMARKS.md](BENCHMARKS.md).

## Architecture

```text
layout.json                 rules.json
     |                          |
     v                          v
JSONLayoutParser          JSONRuleParser
     |                          |
     +---------> DRCEngine <----+
                    |
                    | builds once per run
                    v
           LayerSpatialIndex
         one QuadTree per populated layer
                    |
                    | shared read-only access
          +---------+----------+---------+
          |         |          |         |
          v         v          v         v
 MinWidthRule MinSpacingRule MinEnclosureRule DensityRule
          |         |          |         |
          +---------+----------+---------+
                    |
          exact geometry and density checks
                    |
                    v
          vector<Violation>
                    |
          +---------+---------+
          |                   |
          v                   v
  JSONReportWriter      SVG visualization
```

The design keeps responsibilities separated:

| Module | Responsibility |
|---|---|
| `geometry/` | Geometry primitives, predicates, measurements, and witness results |
| `domain/` | Layers, shapes, violations, and violation markers |
| `spatial/` | QuadTree storage and layer-aware candidate queries |
| `rules/` | Rule-specific DRC semantics |
| `engine/` | Shared-index construction and rule orchestration |
| `io/` | JSON layout/rule parsing and report serialization |

## Shared `LayerSpatialIndex`

`LayerSpatialIndex` owns one `QuadTree` for each populated layout layer:

```text
LayerSpatialIndex
├── Metal1 -> QuadTree
├── Metal2 -> QuadTree
└── Via12  -> QuadTree
```

Construction follows two stages:

1. Group pointers to the input shapes by `Layer`.
2. For each populated layer, merge its shape bounding boxes into a root boundary, build a QuadTree, and insert the layer's shapes.

The index owns the QuadTrees but does not copy or own the shapes. Its trees store `const Shape*` values that refer to the caller-owned `std::vector<Shape>`. The indexed shape collection must therefore remain alive and stable while the index is in use.

The public query stays independent of the underlying tree implementation:

```cpp
std::vector<const domain::Shape*> query(
    domain::Layer layer,
    const geometry::BoundingBox& region
) const;
```

Queries against absent layers return an empty result, and an empty layout is valid.

### Broad phase and narrow phase

The spatial index is a broad-phase filter. It returns shapes whose bounding boxes make them plausible candidates; it does not replace exact polygon geometry.

```text
QuadTree query
    -> inexpensive candidate filtering

Polygon::distanceTo() / Polygon::contains()
    -> exact DRC decision
```

`DensityRule` uses the same broad-phase contract: it queries only its target layer for each density window, then calls `Polygon::areaInside()` for exact covered-area calculation. This distinction is important for concave polygons and other cases where bounding-box overlap alone does not prove the corresponding polygon relationship or coverage.

## Geometry Core

The geometry layer is isolated under `drcheck::geometry` and contains no rule or layer knowledge.

### `Point` and `Vector`

- coordinate and vector storage;
- vector construction between points;
- orientation tests;
- length, dot product, and cross product.

### `BoundingBox`

- overlap and containment checks;
- merging;
- expansion by a search distance;
- access to minimum and maximum coordinates.

### `Segment`

- constructor rejection of degenerate segments;
- point containment and segment intersection;
- point-to-segment and segment-to-segment distance;
- closest-point witness calculation.

### `Polygon`

- constructor-enforced validity;
- edge generation, signed area, area, and orientation;
- bounding-box calculation;
- point and polygon containment;
- polygon intersection and polygon-to-polygon distance;
- minimum width for orthogonal polygons;
- polygon area clipped to an axis-aligned bounding box through `areaInside()`; and
- detailed edge-pair and closest-point results.

Detailed measurements preserve the geometry that produced them. `DistanceResult` stores the distance and closest points, while `PolygonEdgePairResult` also identifies the corresponding polygon edges. Rules use those results to create optional `ViolationMarker` data.

## Domain Model and Violation Reporting

`Layer` identifies the IC layer associated with a shape. `Shape` combines a unique ID, a layer, and a polygon.

A `Violation` records:

- violation type;
- participating shape IDs;
- explanatory message;
- actual measured value;
- required rule value; and
- an optional `ViolationMarker`.

`ViolationMarker` supports two forms of violation geometry. Distance- and width-based rules can preserve two witness points and their polygon-edge indices, while window-based rules can preserve a region. `DensityRule` uses the region form to identify the exact window that violated its threshold.

`JSONReportWriter` serializes marker data only when it is present. Point/edge markers retain their witness fields, and region markers emit the region bounds needed by downstream consumers. The SVG output highlights these regions directly on the rendered layout.

## Implemented Rules

### `MinWidthRule`

Checks shapes on one target layer using the orthogonal-polygon minimum-width calculation. When a violation is found, the detailed width result supplies the two boundary witnesses used by its marker.

### `MinSpacingRule`

For every shape on the target layer, the rule:

1. expands the shape bounding box by the required minimum spacing;
2. queries that layer through the shared `LayerSpatialIndex`;
3. removes self-pairs and duplicate A-B/B-A pairs; and
4. calls `Polygon::distanceTo()` for exact spacing and witness geometry.

```text
target-layer shape
        |
expanded bounding box
        |
LayerSpatialIndex::query(target layer, region)
        |
nearby candidates only
        |
exact Polygon::distanceTo()
```

### `MinEnclosureRule`

For every inner-layer shape, the rule:

1. queries the shared index for outer-layer candidates using the inner bounding box;
2. uses `Polygon::contains()` to prove exact containment;
3. measures boundary-to-boundary enclosure with `distanceTo(..., false)`; and
4. reports insufficient enclosure or a missing containing outer shape.

The current rule setup checks `Via12` enclosure against `Metal1` and `Metal2` independently through separate rule instances. Spatial filtering reduces candidate discovery work, while `contains()` remains necessary because a bounding-box match alone cannot prove polygon containment.

### `DensityRule`

Checks either a minimum or maximum density threshold on one target layer. Each rule accepts a user-provided `windowSize` and `windowStep`, then evaluates a fixed grid of windows over its analysis bounds.

The analysis bounds can be supplied explicitly. When they are omitted, the rule uses the merged bounding box of the complete layout. An explicit analysis region therefore also allows density checking on an empty layout; without one, an empty layout has no bounds to infer and is rejected.

For every window, the rule:

1. clips the window at the analysis-region boundary, retaining partial edge windows;
2. queries the shared `LayerSpatialIndex` for candidates on the target layer only;
3. uses exact clipped polygon area to calculate covered area;
4. divides by the actual clipped-window area rather than the nominal full-window area; and
5. emits `MinDensity` or `MaxDensity` when the measured ratio violates the configured threshold.

Each density violation carries the evaluated window as a region-based `ViolationMarker`, so JSON and SVG outputs identify the failing area even when no individual shape is the sole cause.

`JSONRuleParser` constructs density rules from the same rule-definition input path as the existing rule types, including the target layer, limit direction, threshold, window size, window step, and optional explicit analysis bounds.

## Rule and Engine Interface

Rules receive the shape collection and the shared index as read-only inputs:

```cpp
virtual std::vector<domain::Violation> check(
    const std::vector<domain::Shape>& shapes,
    const spatial::LayerSpatialIndex& spatialIndex
) const = 0;
```

`DRCEngine::run()` constructs one `LayerSpatialIndex`, passes it to each rule, and aggregates their violations. `DensityRule` participates through this same polymorphic pipeline. Rules that do not need spatial queries, such as `MinWidthRule`, accept the same interface but operate directly on the shapes.

## Project Structure

```text
drcheck/
├── CMakeLists.txt
├── README.md
├── BENCHMARKS.md
├── include/drcheck/
│   ├── geometry/
│   ├── domain/
│   ├── spatial/
│   │   ├── QuadTree.h
│   │   └── LayerSpatialIndex.h
│   ├── rules/
│   ├── engine/
│   └── io/
├── src/
│   ├── geometry/
│   ├── domain/
│   ├── spatial/
│   │   ├── QuadTree.cpp
│   │   └── LayerSpatialIndex.cpp
│   ├── rules/
│   ├── engine/
│   ├── io/
│   └── main.cpp
├── tests/
│   ├── geometry/
│   ├── domain/
│   ├── spatial/
│   ├── rules/
│   ├── engine/
│   ├── io/
│   └── integration/
├── benchmarks/
│   ├── MinSpacingBenchmark.cpp
│   ├── MinEnclosureBenchmark.cpp
│   └── results/
└── examples/
```

## Building

Requirements:

- CMake 3.20 or newer;
- a C++20 compiler;
- Git when dependencies are fetched by CMake.

Configure and build:

```bash
cmake -S . -B build
cmake --build build --config Release
```

Configure without tests when required:

```bash
cmake -S . -B build -DDRCHECK_BUILD_TESTS=OFF
```

## Running Tests

Run the full suite through CTest:

```bash
ctest --test-dir build --output-on-failure
```

The test suite covers geometry invariants and algorithms, parser validation, rule behavior, report serialization, SVG output, QuadTree operations, layer isolation in `LayerSpatialIndex`, engine orchestration, violation-marker consistency, and end-to-end JSON processing. Density coverage includes minimum and maximum limits, target-layer filtering, explicit and inferred analysis bounds, empty-layout behavior, partial edge windows, region markers, parser/report integration, SVG highlighting, and execution through `DRCEngine` alongside existing rules.

## Running DRCheck

The CLI accepts a layout file, a rule file, and an output report path:

```bash
drcheck layout.json rules.json report.json
```

The high-level flow is:

```text
layout JSON + rule JSON
          |
          v
       DRCEngine
          |
          v
    violation report JSON
```

See `examples/` for sample inputs.

## Benchmark Summary

The benchmark suite validates both correctness preservation and reduced exact geometry work:

- dense `MinSpacingRule` layout: up to 99.61% fewer exact pair checks and a measured 148.30x speedup at 2,000 shapes;
- sparse `MinSpacingRule` layout: 100% fewer exact pair checks at every tested size and a peak measured 1,321.29x speedup;
- QuadTree tuning: `capacity = 16` and `maxDepth = 8` selected as balanced defaults from 16 configurations; and
- `MinEnclosureRule`: up to 99.90% fewer containment checks and a measured 4.86x rule-execution speedup at 2,000 vias.

These are workload- and machine-specific measurements, not universal performance guarantees. See [BENCHMARKS.md](BENCHMARKS.md) for methodology, complete results, index-build timing, and interpretation.

## Known Limitations

The current implementation intentionally favors a clear, well-tested architecture over full production-rule coverage.

- `MinWidthRule` currently supports orthogonal polygons only.
- `MinEnclosureRule` evaluates individual outer polygons; it does not combine multiple outer polygons into a union for enclosure.
- The current enclosure assumption is at most one relevant containing outer polygon per rule evaluation.
- QuadTree queries can return false-positive candidates; exact geometry remains required.
- Spatial-index performance depends on geometry distribution and can degrade toward brute-force behavior in unfavorable layouts.
- `DensityRule` uses a fixed, axis-aligned grid with one `windowSize` and `windowStep`; adaptive or multiscale density analysis is not implemented.
- Automatic density analysis bounds require at least one layout shape; an explicit analysis region is required for an empty layout.
- The current JSON format and rule set are intentionally simplified.

## Next Development Areas

- GDSII layout import;
- AI-assisted translation from natural-language requirements into executable design rules;
- minimum-width support for non-Manhattan polygons;
- sweep-line optimization for geometry-intensive rule checks;
- Tcl-based rule decks;

## Project Goal

DRCheck is intended to demonstrate strong C++ design and EDA-oriented problem solving through:

- isolated, testable computational geometry;
- explicit ownership and lifetime rules;
- polymorphic rule execution;
- reusable spatial acceleration;
- structured violation reporting; and
- performance claims backed by correctness-checked benchmarks.
