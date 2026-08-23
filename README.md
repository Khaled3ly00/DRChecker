# DRCheck

[![CI](https://github.com/Khaled3ly00/DRChecker/actions/workflows/ci.yml/badge.svg)](https://github.com/Khaled3ly00/DRChecker/actions/workflows/ci.yml)

DRCheck is a C++ Design Rule Checker for simplified IC layouts. The project focuses on modern C++, computational geometry, IC verification concepts, unit testing, spatial indexing, and evidence-based performance optimization.

The current end-to-end application can:

- load layout geometry from JSON and rule definitions from JSON or Tcl;
- translate parsed rule parameters into validated polymorphic rules through a central factory;
- run minimum-width, minimum-spacing, minimum-enclosure, and per-layer minimum/maximum density rules;
- build one shared, layer-aware spatial index for a rule run;
- report actual and required rule values;
- attach optional point/edge or region-based witness geometry to violations;
- write machine-readable JSON reports; and
- render SVG layouts with highlighted violation geometry, including density regions.

## Current Status

The geometry core, domain model, rule framework, centralized `RuleFactory`, JSON and Tcl rule-deck input paths, CLI, SVG visualization, violation metadata, QuadTree optimization, reusable spatial-index architecture, density-rule pipeline, and cross-platform CI workflow are implemented and covered by tests.

`DRCEngine` builds one `LayerSpatialIndex` from the current shape collection and passes it to every rule. `MinSpacingRule`, `MinEnclosureRule`, and `DensityRule` reuse that index for layer-specific candidate discovery instead of building rule-local trees or scanning every possible shape.

Rule construction is separated from rule-deck decoding. `JSONRuleParser` and `TclRuleParser` validate and convert their input fields into `RuleParameters`, then delegate concrete rule creation and required-parameter validation to `RuleFactory`.

Detailed benchmark methodology, results, and QuadTree tuning data have moved to [BENCHMARKS.md](BENCHMARKS.md).

## Architecture

```text
layout.json                    rules.json / rules.tcl
     |                                  |
     v                                  v
JSONLayoutParser         JSONRuleParser / TclRuleParser
     |                                  |
     |                           RuleParameters
     |                                  |
     |                                  v
     |                            RuleFactory
     |                                  |
     +-----------> DRCEngine <-----------+
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
| `rules/` | Rule-specific DRC semantics and centralized rule construction |
| `engine/` | Shared-index construction and rule orchestration |
| `io/` | JSON layout parsing, JSON/Tcl rule-deck parsing, parameter conversion, and report serialization |

## Rule Construction and JSON Parsing

`RuleFactory` centralizes construction of every supported concrete rule behind one interface:

```cpp
static std::unique_ptr<Rule> create(
    const std::string& type,
    const RuleParameters& params
);
```

`RuleParameters` is a shared parameter object containing optional fields for single-layer rules, inner/outer-layer rules, numeric thresholds, density configuration, and an optional `BoundingBox` analysis window. The factory validates the fields required by the requested rule before returning a `std::unique_ptr<Rule>`.

| JSON rule type | Factory key | Required parameters | Optional parameters |
|---|---|---|---|
| `MinWidth` | `min_width` | `layer`, `value` | none |
| `MinSpacing` | `min_spacing` | `layer`, `value` | none |
| `MinEnclosure` | `min_enclosure` | `innerLayer`, `outerLayer`, `value` | none |
| `Density` | `density` | `layer`, `limit`, `value`, `windowSize`, `windowStep` | `analysisWindow` |

`JSONRuleParser` remains responsible for the JSON-facing concerns:

1. opening and decoding the rule file;
2. requiring a top-level `rules` array and an object for each rule entry;
3. converting layer names through `layerFromString()`;
4. converting density limits from `Minimum` or `Maximum`;
5. parsing optional analysis-window bounds from `minX`, `minY`, `maxX`, and `maxY`;
6. populating `RuleParameters`; and
7. mapping the external JSON type to the internal factory key and delegating construction.

`JSONRuleParser::load()` is now a static entry point. JSON-specific density-limit and bounding-box helpers remain inside the implementation file, keeping `nlohmann/json` and density parsing details out of the public parser header.

```text
JSON rule object
       |
       v
JSONRuleParser
field validation and conversion
       |
       v
RuleParameters
       |
       v
RuleFactory::create()
required-parameter validation
       |
       v
unique_ptr<Rule>
```

The factory rejects missing required parameters and unknown factory keys with `std::invalid_argument`. The parser separately rejects malformed rule-file structure, unknown JSON rule types, and unsupported density-limit strings. This keeps JSON representation concerns out of concrete rule constructors while giving programmatic callers the same validated construction path.

## Tcl Rule-Deck Parsing

`TclRuleParser` adds a second rule-deck front end without changing the rule or engine interfaces. Its static `load()` function creates a Tcl interpreter, registers one DRCheck-specific object command named `rule`, evaluates the requested file, and returns the resulting polymorphic rules.

The supported Tcl declarations mirror the four JSON rule families:

```tcl
rule min_width -layer Metal1 -value 0.20
rule min_spacing -layer Metal1 -value 0.25
rule min_enclosure -inner Via12 -outer Metal1 -value 0.10
rule density -layer Metal1 -limit minimum -value 0.30 -window_size 10 -window_step 5
rule density -layer Metal2 -limit maximum -value 0.70 -window_size 20 -window_step 10 -region {0 0 100 100}
```

Rule options are parsed as `-option value` pairs and stored by name, so their order does not affect the result. The parser rejects malformed pairs, duplicate options, missing required options, unexpected rule-specific options, unsupported rule types, invalid numeric values, and density limits other than `minimum` or `maximum`.

For density rules, `-region` is optional. When present, it must be a Tcl list containing exactly four numeric values in `minX minY maxX maxY` order. The parser uses Tcl's list API rather than splitting the string manually and uses the count type required by either Tcl 8 or Tcl 9 at compile time.

```text
rules.tcl
    |
    v
Tcl_EvalFile()
    |
    v
registered rule command
option validation and conversion
    |
    v
RuleParameters
    |
    v
RuleFactory::create()
    |
    v
unique_ptr<Rule>
```

The callback converts C++ exceptions into Tcl command errors. If deck evaluation fails, `TclRuleParser` captures the interpreter message, destroys the interpreter, and reports the failure as `std::invalid_argument`. No Tcl-specific object crosses the `RuleFactory` boundary.

CMake adds `TclRuleParser.cpp` to the main library, discovers Tcl through `find_package(TCL REQUIRED)`, and keeps the Tcl include path and library linkage private to the `drchecker` target.

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

`JSONRuleParser` parses density rules through the same rule-definition input path as the existing rule types, including the target layer, limit direction, threshold, window size, window step, and optional explicit analysis bounds. It stores those values in `RuleParameters` and delegates construction to `RuleFactory`.

`TclRuleParser` supplies the same density configuration through `-layer`, `-limit`, `-value`, `-window_size`, `-window_step`, and the optional `-region` list before delegating to the same factory.

## Rule and Engine Interface

Rules receive the shape collection and the shared index as read-only inputs:

```cpp
virtual std::vector<domain::Violation> check(
    const std::vector<domain::Shape>& shapes,
    const spatial::LayerSpatialIndex& spatialIndex
) const = 0;
```

`DRCEngine::run()` constructs one `LayerSpatialIndex`, passes it to each rule, and aggregates their violations. `DensityRule` participates through this same polymorphic pipeline. Rules that do not need spatial queries, such as `MinWidthRule`, accept the same interface but operate directly on the shapes.

The CLI integration uses stateless static entry points for `JSONLayoutParser::load()`, `JSONRuleParser::load()`, `TclRuleParser::load()`, `DRCEngine::run()`, and `JSONReportWriter::write()`. JSON and Tcl decks therefore converge on the same rule vector before engine execution.

## Project Structure

```text
drcheck/
├── .github/
│   └── workflows/
│       └── ci.yml
├── benchmarks/
│   ├── results/
│   │   ├── min_enclosure_benchmark.txt
│   │   ├── min_spacing_dense_benchmark.txt
│   │   ├── min_spacing_sparse_benchmark.txt
│   │   └── quadtree_parameter_tuning.txt
│   ├── MinEnclosureBenchmark.cpp
│   └── MinSpacingBenchmark.cpp
├── examples/
├── include/drcheck/
│   ├── domain/
│   │   ├── Layer.h
│   │   ├── Shape.h
│   │   └── Violation.h
│   ├── engine/
│   │   └── DRCEngine.h
│   ├── geometry/
│   │   ├── BoundingBox.h
│   │   ├── Constants.h
│   │   ├── Point.h
│   │   ├── Polygon.h
│   │   ├── Segment.h
│   │   └── Vector.h
│   ├── io/
│   │   ├── JSONLayoutParser.h
│   │   ├── JSONReportWriter.h
│   │   ├── JSONRuleParser.h
│   │   ├── SVGReportWriter.h
│   │   └── TclRuleParser.h
│   ├── rules/
│   │   ├── DensityRule.h
│   │   ├── MinEnclosureRule.h
│   │   ├── MinSpacingRule.h
│   │   ├── MinWidthRule.h
│   │   ├── Rule.h
│   │   └── RuleFactory.h
│   └── spatial/
│       ├── LayerSpatialIndex.h
│       └── QuadTree.h
├── src/
│   ├── domain/
│   │   ├── Layer.cpp
│   │   ├── Shape.cpp
│   │   └── Violation.cpp
│   ├── engine/
│   │   └── DRCEngine.cpp
│   ├── geometry/
│   │   ├── BoundingBox.cpp
│   │   ├── Point.cpp
│   │   ├── Polygon.cpp
│   │   ├── Segment.cpp
│   │   └── Vector.cpp
│   ├── io/
│   │   ├── JSONLayoutParser.cpp
│   │   ├── JSONReportWriter.cpp
│   │   ├── JSONRuleParser.cpp
│   │   ├── SVGReportWriter.cpp
│   │   └── TclRuleParser.cpp
│   ├── rules/
│   │   ├── DensityRule.cpp
│   │   ├── MinEnclosureRule.cpp
│   │   ├── MinSpacingRule.cpp
│   │   ├── MinWidthRule.cpp
│   │   └── RuleFactory.cpp
│   ├── spatial/
│   │   ├── LayerSpatialIndex.cpp
│   │   └── QuadTree.cpp
│   └── main.cpp
├── tests/
│   ├── domain/
│   │   ├── LayerTest.cpp
│   │   ├── ShapeTest.cpp
│   │   └── ViolationTest.cpp
│   ├── engine/
│   │   └── DRCEngineTest.cpp
│   ├── geometry/
│   │   ├── BoundingBoxTest.cpp
│   │   ├── PointTest.cpp
│   │   ├── PolygonTest.cpp
│   │   ├── SegmentTest.cpp
│   │   └── VectorTest.cpp
│   ├── integration/
│   │   └── EndtoEndTest.cpp
│   ├── io/
│   │   ├── JSONLayoutParserTest.cpp
│   │   ├── JSONReportWriterTest.cpp
│   │   ├── JSONRuleParserTest.cpp
│   │   ├── SVGReportWriterTest.cpp
│   │   └── TclRuleParserTest.cpp
│   ├── rules/
│   │   ├── DensityRuleTest.cpp
│   │   ├── MinEnclosureRuleTest.cpp
│   │   ├── MinSpacingRuleTest.cpp
│   │   ├── MinWidthRuleTest.cpp
│   │   └── RuleFactoryTest.cpp
│   └── spatial/
│       ├── LayerSpatialIndexTest.cpp
│       └── QuadTreeTest.cpp
├── BENCHMARKS.md
├── CMakeLists.txt
└── README.md
```

## Building

Requirements:

- CMake 3.20 or newer;
- a C++20 compiler;
- Tcl development headers and library discoverable by CMake's `find_package(TCL REQUIRED)`;
- Git when dependencies are fetched by CMake.

On Ubuntu, install the Tcl development package before configuring:

```bash
sudo apt-get update
sudo apt-get install -y tcl-dev
```

On Windows, the CI workflow uses vcpkg and its CMake toolchain:

```powershell
vcpkg install tcl:x64-windows
cmake -S . -B build "-DCMAKE_TOOLCHAIN_FILE=$env:VCPKG_INSTALLATION_ROOT/scripts/buildsystems/vcpkg.cmake"
```

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

The test suite covers geometry invariants and algorithms, JSON and Tcl parser validation, factory construction and validation, rule behavior, report serialization, SVG output, QuadTree operations, layer isolation in `LayerSpatialIndex`, engine orchestration, violation-marker consistency, and end-to-end rule-deck processing. `RuleFactoryTest` verifies construction of every supported rule type, representative missing-parameter rejection, and unknown-type rejection. `JSONRuleParserTest` verifies the JSON parser-to-factory path for width, spacing, enclosure, minimum/maximum density, and density with an explicit analysis window. Density coverage also includes target-layer filtering, inferred analysis bounds, empty-layout behavior, partial edge windows, region markers, report integration, SVG highlighting, and execution through `DRCEngine` alongside existing rules.

`TclRuleParserTest` covers all supported Tcl rule families, option-order independence, minimum and maximum density, optional density regions, and invalid-deck rejection. The end-to-end suite loads equivalent JSON and Tcl decks, runs each through `DRCEngine`, and compares rule counts, violation counts, violation types, participating shape IDs, and actual/required values.

## Continuous Integration

GitHub Actions provides cross-platform CI through `.github/workflows/ci.yml`. The workflow runs automatically on pushes and pull requests and validates the project on both Ubuntu and Windows.

For each platform, CI:

1. checks out the repository;
2. installs the Tcl development dependency using `tcl-dev` on Ubuntu or vcpkg on Windows;
3. configures the project with CMake, using the vcpkg toolchain on Windows;
4. builds the project in Release configuration; and
5. runs the test suite through CTest with failure output enabled.

```text
push / pull request
        |
        v
GitHub Actions
      /               \
     v                 v
  Ubuntu             Windows
     |                 |
  tcl-dev          vcpkg Tcl
     |                 |
CMake configure   CMake + vcpkg toolchain
      \               /
       v             v
        Build + CTest
             |
             v
          pass/fail
```

The Windows job also adds the installed Tcl DLL directory to `PATH` for test execution. The cross-platform workflow helps catch portability issues that may not appear during local Windows development, such as case-sensitive include-path mismatches on Linux, while compiling the Tcl list-handling path against the platform-provided Tcl API.

## Running DRCheck

The CLI uses named argument/value pairs, which can appear in any order. `--layout`, `--rules`, and `--report` are required, while `--svg` is optional. The rule-deck parser is selected from the `.json` or `.tcl` file extension.

Run with a JSON rule deck:

```bash
drcheck --layout layout.json --rules rules.json --report report.json
```

Run with a Tcl rule deck and optional SVG output:

```bash
drcheck --layout layout.json --rules rules.tcl --report report.json --svg report.svg
```

The high-level flow is:

```text
layout JSON + JSON/Tcl rule deck
                 |
                 v
       extension-based parser
                 |
                 v
             DRCEngine
                 |
          +------+------+
          |             |
          v             v
   JSON report     optional SVG
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
- The CLI selects rule-deck parsers by the exact `.json` or `.tcl` extension; other extensions are rejected.
- The only DRCheck-specific Tcl command is `rule`;

## Next Development Areas

- GDSII layout import;
- AI-assisted translation from natural-language requirements into executable design rules;
- minimum-width support for non-Manhattan polygons;
- sweep-line optimization for geometry-intensive rule checks;
- Tcl-based DRC execution and automation commands beyond rule declaration;

## Project Goal

DRCheck is intended to demonstrate strong C++ design and EDA-oriented problem solving through:

- isolated, testable computational geometry;
- explicit ownership and lifetime rules;
- polymorphic rule execution;
- centralized, validated rule construction;
- JSON and Tcl rule-deck front ends sharing the same factory and engine;
- reusable spatial acceleration;
- structured violation reporting; and
- performance claims backed by correctness-checked benchmarks.
