# DRCheck Benchmarks

This document contains the benchmark methodology and recorded results for the spatially optimized `MinSpacingRule` and `MinEnclosureRule` paths. The main [README.md](README.md) keeps only a concise performance summary.

## Benchmark Goals

The benchmarks answer two separate questions:

1. Does spatial candidate filtering preserve the benchmarked DRC result?
2. How much exact geometry work and runtime does it remove for the selected synthetic layouts?

Runtime alone is not sufficient because it varies with the machine, compiler, build configuration, and background load. Each benchmark therefore records a direct work metric:

- `MinSpacingRule`: exact polygon-pair distance checks;
- `MinEnclosureRule`: exact polygon containment checks.

For every scenario, the brute-force and spatial paths must report the same violation count. A mismatch is treated as a benchmark failure rather than a performance result.

All recorded timings were produced with a Release build. They are specific to the tested geometries, machine, and implementation and should not be interpreted as universal speedup guarantees.

## Candidate Filtering Model

Both optimized rules follow a broad-phase/narrow-phase design:

```text
spatial query
    -> bounding-box candidates
    -> inexpensive broad-phase reduction

exact polygon operation
    -> distance or containment
    -> final narrow-phase DRC decision
```

The optimized and brute-force paths use the same exact polygon operation. Only candidate discovery changes.

## MinSpacing Benchmark

### Compared paths

```text
Brute force
all unique target-layer shape pairs
        |
        v
Polygon::distanceTo()

Spatial
expanded shape bounding box
        |
        v
QuadTree range query
        |
        v
unique nearby candidates
        |
        v
Polygon::distanceTo()
```

The original benchmark's QuadTree timing includes:

- root-boundary calculation;
- tree construction;
- shape insertion;
- range queries; and
- exact polygon-distance checks.

For each layout size, the benchmark records pair checks, violations, runtime, pair-check reduction, and brute-force-to-QuadTree speedup.

### Dense-grid methodology

```text
Build Type: Release
Shape Size: 10 x 10
Gap: 2
Minimum Spacing: 3
QuadTree Capacity: 4
QuadTree Max Depth: 8
Shape Counts: 100, 500, 1000, 2000
```

Because the gap is smaller than the minimum spacing, nearby shapes violate the rule. The brute-force and QuadTree paths produced the same violation count at every size.

### Dense-grid results

| Shapes | Brute Pair Checks | QuadTree Pair Checks | Violations | Pair Reduction | Brute Time (ms) | QuadTree Time (ms) | Speedup |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 100 | 4,950 | 342 | 342 | 93.090909% | 51.3702 | 4.1022 | 12.522598x |
| 500 | 124,750 | 1,868 | 1,868 | 98.502605% | 1,452.5674 | 23.6293 | 61.473146x |
| 1,000 | 499,500 | 3,811 | 3,811 | 99.237037% | 4,704.7999 | 43.7586 | 107.517149x |
| 2,000 | 1,999,000 | 7,733 | 7,733 | 99.613157% | 18,048.8126 | 121.7085 | 148.295416x |

The optimized path still calls `Polygon::distanceTo()` for every returned candidate. Its benefit comes from preventing distant shapes from reaching that exact calculation.

### Sparse-grid methodology

The sparse scenario uses the same framework, shape size, minimum spacing, and initial QuadTree parameters, but increases the gap to 10. No pair violates the minimum spacing.

### Sparse-grid results

| Shapes | Brute Pair Checks | QuadTree Pair Checks | Violations | Pair Reduction | Brute Time (ms) | QuadTree Time (ms) | Speedup |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 100 | 4,950 | 0 | 0 | 100.000000% | 51.6193 | 0.4417 | 116.865067x |
| 500 | 124,750 | 0 | 0 | 100.000000% | 1,233.4378 | 4.0310 | 305.988043x |
| 1,000 | 499,500 | 0 | 0 | 100.000000% | 5,218.1540 | 3.9493 | 1,321.285797x |
| 2,000 | 1,999,000 | 0 | 0 | 100.000000% | 18,352.6528 | 39.0482 | 469.999969x |

The brute-force path evaluates every unique pair even when all shapes are far apart. In this synthetic sparse layout, the QuadTree eliminates every exact distance check.

## QuadTree Parameter Tuning

After the dense and sparse comparisons, `capacity` and `maxDepth` were tuned on the 2,000-shape layouts.

```text
Shape Count: 2000
Minimum Spacing: 3
Repetitions Per Configuration: 5

Capacity Values: 2, 4, 8, 16
Max-Depth Values: 4, 6, 8, 10
Total Configurations: 16
```

Each timing is the average of five measured runs after a warm-up run. The previously validated `capacity = 4`, `maxDepth = 8` configuration served as the correctness reference.

Every configuration preserved:

```text
Dense Pair Checks: 7733
Dense Violations: 7733

Sparse Pair Checks: 0
Sparse Violations: 0
```

### Tuning results

| Capacity | Max Depth | Dense Average (ms) | Sparse Average (ms) |
|---:|---:|---:|---:|
| 2 | 4 | 144.904740 | 39.571920 |
| 2 | 6 | 224.855460 | 40.989100 |
| 2 | 8 | 126.166980 | 40.901360 |
| 2 | 10 | 129.532500 | 40.623960 |
| 4 | 4 | 124.468900 | **37.893120** |
| 4 | 6 | 122.885340 | 40.515640 |
| 4 | 8 | 124.138340 | 38.722680 |
| 4 | 10 | 127.001320 | 38.606560 |
| 8 | 4 | 125.645100 | 38.631500 |
| 8 | 6 | **121.160320** | 39.335440 |
| 8 | 8 | 123.754980 | 40.490280 |
| 8 | 10 | 123.988160 | 38.737600 |
| 16 | 4 | 125.864360 | 42.097560 |
| 16 | 6 | 121.517380 | 38.862880 |
| **16** | **8** | **122.019400** | **38.430120** |
| 16 | 10 | 122.424560 | 38.903620 |

### Selected defaults

```text
capacity = 16
maxDepth = 8
```

The absolute fastest dense configuration was `8 / 6`; the absolute fastest sparse configuration was `4 / 4`. The selected `16 / 8` configuration was chosen as a balanced default:

- about 0.71% slower than the fastest dense result;
- about 1.42% slower than the fastest sparse result; and
- faster in both recorded workloads than the original `4 / 8` reference.

The sweep also shows that more aggressive subdivision is not automatically better. For example, `capacity = 2`, `maxDepth = 6` was substantially slower on the dense layout than the balanced configurations.

These tuned values are spatial-index implementation parameters, not design-rule values. They are now used by the shared `LayerSpatialIndex`, which builds the per-layer QuadTrees reused by spatial rules.

## MinEnclosure Benchmark

### Purpose

This benchmark validates the change from scanning the entire outer layer inside `MinEnclosureRule` to querying the shared `LayerSpatialIndex` first.

The primary metric is the number of calls to `Polygon::contains()`, because containment is the exact narrow-phase operation used to prove that an outer polygon encloses an inner polygon.

### Synthetic layout

Each grid cell contains one `VIA1` polygon and one surrounding `M1` polygon:

```text
M1: 8 x 8
VIA1:  4 x 4
Actual enclosure: 2
Required enclosure: 3
Cell gap: 10
```

Every via is therefore contained by one metal polygon but violates the minimum-enclosure requirement. Expected violations equal the via count.

Shapes are generated in interleaved order:

```text
Metal0, Via0, M1, Via1, ...
```

That ordering explains the brute-force containment counts. The first via finds its metal after one outer-layer check, the second after two, and so on:

```text
1 + 2 + ... + N = N(N + 1) / 2
```

### Compared paths

```text
Brute force
for each VIA1
    scan M1 shapes in order
    call Polygon::contains()
    on the containing metal:
        call distanceTo(..., false)

Spatial
build LayerSpatialIndex once
for each VIA1
    query M1 with the via bounding box
    call Polygon::contains() on returned candidates
    on the containing metal:
        call distanceTo(..., false)
```

`Polygon::contains()` is intentionally retained in the spatial path. A QuadTree query proves only that bounding boxes overlap; it cannot prove polygon containment, particularly for concave geometry.

Index construction is timed separately from spatial rule execution because the production architecture builds the shared index once and reuses it across rules.

### Results

| Vias | Total Shapes | Brute Containment Checks | Spatial Containment Checks | Violations | Check Reduction | Brute Time (ms) | Index Build (ms) | Spatial Rule Time (ms) | Rule Speedup |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 100 | 200 | 5,050 | 100 | 100 | 98.019802% | 3.3705 | 0.3802 | 2.6420 | 1.275738x |
| 500 | 1,000 | 125,250 | 500 | 500 | 99.600798% | 32.7037 | 1.5504 | 14.4142 | 2.268853x |
| 1,000 | 2,000 | 500,500 | 1,000 | 1,000 | 99.800200% | 120.6059 | 2.7031 | 25.3051 | 4.766071x |
| 2,000 | 4,000 | 2,001,000 | 2,000 | 2,000 | 99.900050% | 392.9747 | 6.4042 | 80.9047 | 4.857254x |

The reported rule speedup is:

```text
brute-force rule time / spatial rule time
```

It deliberately excludes index construction. The separate build-time column makes that cost visible while reflecting the production design: index construction is paid once and can be amortized across `MinSpacingRule`, `MinEnclosureRule`, and future spatial consumers.

The spatial path performs exactly one containment check per via in this layout. Violation counts match the brute-force baseline and the known expected count for every tested size.

## Benchmark Driver Workflow

`benchmarks/MinSpacingBenchmark.cpp` is an experimental benchmark driver rather than a fixed production entry point. Its reusable helpers remain stable, while `main()` is intentionally changed for the current experiment.

```text
Dense / sparse comparison
    -> generate several layout sizes
    -> run brute force and QuadTree
    -> record work reduction and speedup

Parameter tuning
    -> generate the selected dense and sparse layouts
    -> sweep capacity and maxDepth
    -> average repeated timings
    -> validate unchanged checks and violations
```

`benchmarks/MinEnclosureBenchmark.cpp` is separate so containment-specific metrics and shared-index build timing do not complicate the spacing benchmark.

## Running Benchmarks

Build in Release mode:

```bash
cmake --build build --config Release
```

Run the configured spacing or enclosure benchmark target for the current build system. Recorded raw outputs can be kept under `benchmarks/results/`.

Release builds should be used for comparisons. Debug-mode timings are not representative of optimized rule execution.

## Conclusions

- Spatial filtering preserved the recorded violation counts in every benchmark.
- Candidate/check counts provide a stable explanation for the measured runtime gains.
- Dense spacing layouts still require exact checks for nearby pairs, but avoid almost all distant pairs as the layout grows.
- Sparse spacing layouts allow the QuadTree to reject every pair before exact distance calculation in the tested scenario.
- Enclosure candidate discovery drops from ordered outer-layer scanning to one returned candidate per via in the tested grid.
- Shared-index construction has a measurable cost, but it is paid once and reused across rules.
- QuadTree parameters need measurement; smaller capacity and deeper subdivision are not inherently faster.
