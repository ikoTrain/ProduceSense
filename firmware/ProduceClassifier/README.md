# ProduceClassifier v1.1

The deployed sketch is `ProduceClassifier.ino` with `model.h` in the same directory. The firmware uses the Arduino-ESP32 core and runs inference on the ESP32-S3.

## Connections

- GPIO5 → 10 kΩ resistor → electrode A and GPIO4.
- Electrode B → GPIO6.
- GPIO4 is the ADC input, not a driven output.
- The tested board uses GPIO48 for the onboard addressable RGB LED and GPIO0 for BOOT.

See the [drawn circuit](../../docs/assets/wiring.svg). There is no separate capacitor.

## Operation

After the board has booted, insert the two separated electrodes, keep the specimen still, then press/release BOOT. Blue means measurement in progress. Red, yellow and green mean apple, potato and cucumber respectively. Purple means an out-of-range raw ADC sample was observed.

Fast mode uses a 5-second settling interval and six pulse pairs. Slow mode uses 30 seconds and 30 pairs. Each pulse records at target offsets of 1, 5 and 20 ms. Both outputs are LOW during the 30 ms interval before each pulse. There is a 400 ms gap between pairs.

Short BOOT press selects fast mode. Hold BOOT approximately two seconds and release for slow mode. Do not hold BOOT during power-up to start a measurement; it is also a bootloader button.

Serial at 115200 baud: `r` fast, `l` slow, `s` stop, `i` status, `v` inference reference test.

`model.h` contains the frozen three-class model and 12 reference vectors. The references check implementation parity; they are not 12 independent accuracy tests. `MODEL_MU`, `MODEL_SD` and `CENTERS` correspond to `../../model/three-class-v1.json`.

The firmware is preserved from the working device. Repository analysis verifies the frozen model and captured predictions without changing or reflashing the board.
