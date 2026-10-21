# Recording guide

The source JSON, CSV and serial TXT files are preserved without changing measured values or historical metadata. Most metadata is English; occasional original notes and file identifiers are retained as recorded.

| Directory | Stage |
|---|---|
| `pilot` | Single-electrode capacitive measurements and wire/electrode controls |
| `two-electrode` | Early two-electrode recordings; the current model selects six original specimens by `training_ids` |
| `two-electrode-holdout` | Separate evaluation of an earlier model; not the final fast-mode series |
| `two-electrode-blind` | Six earlier withheld-label trials; not training inputs for the current model |
| `diagnostics` | Diagnostic recordings outside the final evaluation |
| `final-series` | Thirty final fast-mode captures, two per specimen FINAL01–FINAL15 |

Final JSON fields include specimen ID, actual class, placement number, six mean voltages, prediction, invalid/correct flags, mode, pulse-pair count, elapsed time and the frozen model hash. Final TXT files contain the corresponding device output. They retain six feature means rather than the individual cycle readings.

Features are ordered `F1, F5, F20, R1, R5, R20`, in millivolts. F/R denote excitation direction; the numeric suffix is the target delay in milliseconds. Actual ADC reads occur sequentially. `null` geometry or mass fields mean the value was not recorded, not zero.

Raw single-electrode values are touch-sensor counts in arbitrary units. They cannot be compared numerically to the two-electrode voltage readings. Original three-class model coefficients are in `../model/three-class-v1.json`; final capture hashes refer to those exact file bytes.

`../analysis/final-series.csv` is a derived convenience table. To regenerate it and verify the results, run `node scripts/analyze.cjs` from the repository root. Early model results, invalid/repeated captures, and self-reported checks remain separate from the final denominator.
