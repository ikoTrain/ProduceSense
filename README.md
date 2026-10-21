# ProduceSense

**An ESP32-S3 experiment that classifies apples, potatoes and cucumbers from six transient-voltage measurements.** Two metal electrodes, one 10 kΩ resistor, no camera, and on-device nearest-centroid inference.

![The working prototype](docs/assets/setup.jpg)

## Recorded result

| Final fast-mode evaluation | Result |
|---|---:|
| Physical specimens | 15: five per class |
| Electrode placements | 30: two per specimen |
| Correct predictions | 30 / 30 |
| Time per measurement | 7.6 s |
| Training specimens | 6: two per class |

These are results from one recorded session, not a claim of universal accuracy. The model was fixed before the final series. Repeated placements of a specimen are dependent; the series used mixed open-label and withheld-label trials with grouped class order.

## Read the experiment

- [English article](docs/article.md): the single-electrode pilot, controls, physics, wiring, electrode dimensions, algorithm and evaluation.
- [English PDF](docs/ProduceSense-English.pdf).
- [Classifier flowchart](docs/assets/algorithm.svg).
- [Data guide](data/README.md) and [recomputed results](analysis/results.json).

## Hardware

| Part | Configuration used |
|---|---|
| Board | ESP32-S3 N16R8 |
| Series resistor | 10 kΩ |
| Electrodes | Two matching metal teeth from a kitchen cutter |
| Immersed section | Approximately 7.5 mm wide × 13 mm long |
| RGB LED | GPIO48 on the tested board |
| BOOT button | GPIO0 |

![Wiring](docs/assets/wiring.svg)

GPIO5 drives a **10 kΩ resistor** connected to electrode A and GPIO4. GPIO4 is the ADC input. Electrode B connects to GPIO6. There is **no separate capacitor** in the assembly: capacitance in the article’s equivalent circuit describes sample and contact properties.

The actual GPIO mapping of the onboard RGB LED depends on the board variant; N16R8 identifies memory capacity, not a universal board pinout.

## Run the firmware

1. Install Arduino IDE and the Espressif Arduino-ESP32 board support package, or use an existing Arduino CLI installation.
2. Open [firmware/ProduceClassifier/ProduceClassifier.ino](firmware/ProduceClassifier/ProduceClassifier.ino). Keep `model.h` beside it.
3. Select the matching ESP32-S3 board and serial port. The sketch uses the core’s `rgbLedWrite`, `analogRead` and `analogReadMilliVolts` APIs; no separate sensor library is needed.
4. Upload, insert the separated electrodes into the specimen, and press/release BOOT **after the board has booted**. Keep the specimen and wires still and remove your hands during measurement.

| Control / output | Meaning |
|---|---|
| Short BOOT press or serial `r` | Fast: 5 s settling, 6 pulse pairs |
| BOOT held about 2 s, then released; or serial `l` | Slow: 30 s settling, 30 pulse pairs |
| Serial `s` | Stop and turn off the RGB LED |
| Serial `i` | Firmware and state information |
| Serial `v` | Run 12 built-in inference reference vectors |
| Blue | Measurement in progress |
| Red / yellow / green | Apple / potato / cucumber |
| Purple | At least one raw ADC count below 20 or above 4075 |

Serial baud rate: **115200**. Holding BOOT while applying power is a bootloader operation, not the slow-measurement command. A valid signal always produces one of three classes; purple is not unknown-class detection.

## How classification works

For each excitation direction, GPIO4 is sampled at 1, 5 and 20 ms. Fast mode averages six pulse pairs into `[F1, F5, F20, R1, R5, R20]`. Training-set means and standard deviations normalize those features. The nearest of three stored class centroids determines the LED color.

![Algorithm](docs/assets/algorithm.svg)

Training uses the original six specimens, averaged over two placements each. Runtime predictions use one placement. Final test specimens were not added to training. The raw model file retains its original long-protocol description; firmware v1.1 reuses those exact coefficients in fast mode.

## Reproduce the numerical analysis

Requires Node.js; no npm packages or network access are needed.

```sh
node scripts/analyze.cjs
```

The script reconstructs the model from original training recordings, checks its coefficients and SHA-256 against final captures, recalculates all 30 predictions, checks timing and specimen counts, and recomputes the post-hoc training-size comparison. Outputs are written under `analysis/`.

On Windows, the source charts can also be regenerated with Windows PowerShell and .NET charting:

```powershell
powershell -NoProfile -File scripts/charts.ps1
```

To rebuild the static article HTML:

```sh
node scripts/build-docs.cjs
```

## Repository layout

| Directory | Contents |
|---|---|
| `firmware/ProduceClassifier` | Deployed v1.1 firmware and frozen model coefficients |
| `firmware/TwoElectrode` | Earlier measurement firmware |
| `model` | Frozen current and earlier model files |
| `data` | Original recordings and metadata, grouped by experimental stage |
| `analysis` | Machine-readable results and a convenient final-series CSV |
| `docs` | English article, PDF, photographs, charts and editable SVG diagrams |
| `scripts` | Reproducible analysis and document/chart generation |

Original recording identifiers and metadata are retained for traceability. Earlier models and exploratory trials are kept separate from the final three-class result. The independent ten-specimen check is self-reported and is not pooled with logged results.
