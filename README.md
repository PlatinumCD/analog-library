# Analog MVM Library

This project provids an implementation of an Analog Matrix-Vector Multiplication (MVM) library, including classes and methods to manage matrices and vectors, quantize and dequantize their values, and perform MVM operations on a specified hardware infrastructure.

## Overview

The core components of this project are:

- **AnalogMatrix**: Represents a matrix and provides methods for quantizing, printing, and other matrix operations compatible with MVM analog intrinsic calls.
- **AnalogVector**: Represents a vector and provides methods for quantizing, printing, and other vector operations compatible with MVM analog intrinsic calls.
- **AnalogContext**: Manages scaling factors for matrices and vectors.
- **Analog Operations**: Provides functions to set matrices and vectors to tiles, load vectors into tiles, perform computations on tiles, store vectors from tiles, and move vectors between tiles.

## Dependencies

This project is intended to run with specific infrastructure and libraries. Ensure you have the following:

1. **Analog Library**: [https://github.com/PlatinumCD/analog-library](https://github.com/PlatinumCD/analog-library) (main branch)
2. **Custom LLVM**: [https://github.com/PlatinumCD/llvm-project](https://github.com/PlatinumCD/llvm-project) (llvm-riscv branch)
3. **Custom SST Elements**: [https://github.com/PlatinumCD/sst-elements](https://github.com/PlatinumCD/sst-elements) (refactor branch)
4. **Build Infrastructure**: [https://github.com/PlatinumCD/RISCV-Analog-Coprocessor](https://github.com/PlatinumCD/RISCV-Analog-Coprocessor) (main branch)

## Files and Their Purpose

- **`analog/AnalogMatrix.h`**: Contains the `AnalogMatrix` class, which manages matrices and supports MVM operations.
- **`analog/AnalogVector.h`**: Contains the `AnalogVector` class, which manages vectors and supports MVM operations.
- **`analog/AnalogDataType.h`**: Defines the `AnalogDataType` class, a base class for data types used in analog computations.
- **`analog/AnalogContext.h`**: Defines the `AnalogContext` class, which tracks array scale factors for matrices and vectors.
- **`analog/analog_operations.h`**: Contains functions for setting, loading, computing, storing, and moving vectors and matrices within tiles.
- **`analog/analog.h`**: Includes the necessary headers and defines constants for the default matrix and vector sizes for the device.

## Getting Started

### Prerequisites

Ensure you have all dependencies properly installed and set up as per the instructions in their respective repositories.

### Installation

1. Clone the main repository:
   ```sh
   git clone https://github.com/PlatinumCD/RISCV-Analog-Coprocessor.git
   cd RISCV-Analog-Coprocessor
   ```

2. Follow the build instructions provided in the `RISCV-Analog-Coprocessor` repository to set up the build infrastructure.

### Building the Project

1. Ensure all dependencies (Analog Library, Custom LLVM, Custom SST Elements) are properly checked out and built as per their respective instructions.
2. Build the project using the provided build scripts from the `RISCV-Analog-Coprocessor` repository.

## Contributing

Contributions are welcome! Please fork the repository and open a pull request with your changes. Ensure that your code is properly documented and adheres to the coding standards of the project.
