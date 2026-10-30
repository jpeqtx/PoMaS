# Changelog

All notable changes to PoMaS will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Planned
- Brightness control for LCD backlight
- Configurable sleep timeout
- Additional medical scores (APACHE II, SOFA)
- Notes Application
- Page Sorter

---

## [0.92] - 2026-10-30

### Added

#### Calculator
- History: 100 entries with recall via `Fn+2`
- Memory: M+/M-/MR/MC + variables A-J
- Ans Bank: up to 20 saved results (`Fn+b`)
- Complex numbers: four arithmetic operations, polar form, power, exp/sin/cos/log
- Base conversion: DEC/HEX/OCT/BIN (`Fn+5`)
- Bitwise operations: AND/OR/XOR/NOT/SHL/SHR (`Fn+6`)
- RPN mode (`Fn+9`)
- Extended tools: equation solver, numerical analysis, vectors/matrices, geometry (27 shapes), probability simulation, prime factorization

#### CAS
- Polynomial evaluation, differentiation, integration, addition, multiplication
- Roots (complex supported, Durand-Kerner)
- Taylor expansion (arbitrary functions, order 1-7)
- Limit (numerical)
- Symbolic substitution

#### Statistics
- Descriptive statistics, special means (weighted/geometric/harmonic/trimmed)
- Regression: linear, quadratic, power, exponential, logarithmic, logistic, Med-Med, multiple, stepwise
- Hypothesis tests: z, t (1-sample/2-sample/Welch/paired), F, ANOVA (1-way/2-way), chi-square (GOF/independence), Fisher exact
- Non-parametric: Mann-Whitney U, Wilcoxon, Kruskal-Wallis, Friedman, sign test, Spearman, Kendall tau
- Risk indices: OR, RR, RD, NNT, confidence intervals
- Distributions: normal, t, chi-square, F, binomial, Poisson, geometric
- Multivariate: PCA, factor analysis (Varimax), k-means, hierarchical (Ward), decision tree (CART)
- Stratified: Mantel-Haenszel OR/RR, Breslow-Day, Cochran's Q
- Survival: Kaplan-Meier, Log-rank, Cox PH, median survival
- Meta-analysis: inverse variance, DerSimonian-Laird, forest/funnel plots, Egger's test
- Statistical graphs: 17 types
- CSV stream statistics

#### Medical Calculator
- Cardiology / Vitals: MAP, pulse pressure, RPP, shock index
- Pulmonology: P/F ratio, A-a gradient, oxygenation index
- Hematology: ANC, MCV/MCH/MCHC, Mentzer index
- Clinical scores: GCS, CURB-65, qSOFA, Child-Pugh, MELD
- Body composition: BMI, BSA (Mosteller/DuBois), IBW (Devine), ABW
- Renal: CKD-EPI 2021, Cockcroft-Gault, Schwartz
- Lytes / Acid-Base: corrected Ca/Na, anion gap, osmolality, Winter's formula, delta gap, free water deficit
- Endocrine: HOMA-IR, Free T4 index
- Obstetrics: Naegele (EDD), gestational age
- Pharmacokinetics: half-life, loading dose, maintenance dose, dose by weight
- Epidemiology: survival analysis, meta-analysis, outbreak source trace, risk/odds ratio, confidence intervals

#### SciRef
- 386 scientific constants (11 categories)
- 300 scientific laws (7 categories)
- 532 unit conversions (20 categories)
- 118 periodic table elements
- Symbol/name search

#### Graph
- Plot types: y=f(x), parametric, polar, implicit, inequality, sequence, 3D
- Trace, zoom (Z/X/R/B), square aspect (1:1)
- G-Solve: root, extremum, intersection, definite integral
- Tangent / normal at x
- BMP screenshot

#### Tools
- Spreadsheet 45x26 (CSV I/O, Sheet<->L1..L8 conversion)
- uLisp interpreter (GC, REPL, script execution)
- Wi-Fi connection, Maxima integration (client side)
- Power/About (battery, SD info)
- Settings (NVS persistence)
- Auto power off (minute configurable)

### Changed
- Paper UI v2: light mint paper background + graphite text + mint accents
- Menu memoization for selection preservation
- ESC cooldown prevents accidental cascade returns
- uLisp stack expanded to 32KB (when `SET_LOOP_TASK_STACK_SIZE` available in v3.x)
- uLisp eval/apply depth guard (400 levels on v3.x, 100 on v2.x)

### Fixed
- 47 confirmed bugs (0-division, NaN, inf, buffer overflows, format mismatches, recursion depth, GC hazards)
- uLisp GC: lambda env marking, Env mark & sweep
- Input validation: if/cond/let/define/lambda, arithmetic functions
- Graph implicit function drawing, square aspect option
- Statistical calculation fixes (tDist, qrEig, VIF, chiIndep, etc.)
- ctags prototype collision: forward declarations for BaseMode/LawEntry, `static inline` for formatInt/lawAt
- `enum BaseMode : int` underlying type consistency

### Removed
- 9 dead code functions: `Fraction::neg`, `Complex::sqrtC`, `numStr`, `luSolve`, `bGCCall`, `ExprEval::match`, `CasCommands::doFactor`, `StatMultivar::pickList`, `DisplayMgr::clearBody`
- Unused variable: `ansLine`

---

## [0.91] - 2025-XX-XX

### Added
- Initial 6-mode implementation (Calculator, CAS, Statistics, SciRef, Graph, Tools)
- Basic uLisp interpreter
- Core arithmetic, CAS polynomial operations
- Fundamental statistical tests
- Unit conversion database
- Periodic table

---

[Unreleased]: https://github.com/jpeqtx/PoMaS/compare/v0.92...HEAD
[0.92]: https://github.com/jpeqtx/PoMaS/compare/v0.91...v0.92
[0.91]: https://github.com/jpeqtx/PoMaS/releases/tag/v0.91