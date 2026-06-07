# PasswordVault

> Minimal USB HID Password Token for CH552

PasswordVault is a lightweight open-source firmware project for the WCH CH552 microcontroller. The device enumerates as a standard USB HID keyboard and can automatically type a predefined password after a button press or touch event.

## Features

- CH552 USB HID keyboard implementation
- Driverless operation on Windows, Linux and macOS
- Small memory footprint
- Simple hardware design
- Open-source (MIT License)

## Use Cases

- Lab and test environments
- Demo HID projects
- USB firmware learning
- Embedded development experiments

## Security Notice

This firmware stores the password directly in flash memory. It is intended for educational, hobby, and demonstration purposes.

Do not use it for protecting high-value credentials without additional security measures.

## Hardware

- WCH CH552E / CH552G
- USB connector
- Push button or touch sensor
- Optional status LED

## Building

### Requirements

- SDCC
- GNU Make

### Compile

```bash
make clean
make all
```

## Flashing

Use your preferred CH55x flashing tool to program the generated firmware image.

## Repository Structure

```text
src/        Firmware sources
include/    Header files
README.md   Project documentation
LICENSE     MIT license
```

## USB VID/PID

If you distribute modified hardware, ensure that you have the legal right to use the configured USB VID/PID identifiers.

## Contributing

Pull requests, bug reports, and improvements are welcome.

## Author

DPRCZ

## License

MIT License
Copyright (c) 2026 DPRCZ
