# Rpi4B GPIO Propping Tool

**Version:** [UNRELEASED]
**Date-of-latest-release:** 17/11/2024
**Author:** Sebastian Lindau-Skands
**Email:** [slindauskands@gmail.com](mailto:slindauskands@gmail.com)

## Overview
The Rpi4B GPIO Propping Tool is designed to measure and dump data from the GPIO pins of a Raspberry Pi 4B.
This tool allows for high-frequency sampling and is optimized for performance on an overclocked Raspberry Pi 4B.
The tool support up to 13MHz sampling on non-overclocked models, and up to 20MHz on overclocked models (running 2.2GHz)

## Usage

### Command Line Arguments

- `-t` : Sample time (in seconds) for time-based sampling.
- `-s` : Sample size (number of samples) for size-based sampling. (Note that only -s or -t needs to be set.)
- `-f` : Sample frequency (in KHz).
- `-n` : Sample name.
- `-p` : Pin(s). (currently only 1 pin can be specified, but for the future this might change)
- `-a` : Acknowledge that sampling rates above 13MHz require overclocking on the Raspberry Pi.

### Return Values

- `-1` : Invalid amount of arguments.
- `-2` : Abandoned by user (no overclocking on Raspberry Pi).

## Example
```bash
./data_proper -t 10 -f 20000 -n "example_sample" -a
```
This command will sample data for 10 seconds at a frequency of 20 MHz and save it with the name "example_sample", acknowledging that overclocking is required for high-frequency sampling.

## Installation

1. Clone the repository:

bash
```bash
git clone https://github.com/lynet_101/rpi4b-gpio-propping-tool.git
```

2. Navigate to the project directory:

```bash
    cd rpi4b-gpio-propping-tool
```
3. Compile the source code:

```bash
    gcc -o data_proper data_proper.c -lrt -O3
```

## Contributing
Contributions are welcome! Please fork the repository and submit a pull request with your changes.

## License
This project is licensed under the MIT License. See the [LICENSE](https://github.com/Lynet101/rpi4b-gpio-propping-tool/blob/main/LICENSE) file for details.

## Contact
For any questions or issues, please contact Sebastian Lindau-Skands at [slindauskands@gmail.com](mailto:slindauskands@gmail.com).
