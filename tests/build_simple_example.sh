EXAMPLE=simple_example.cpp
EXE_OUTPUT=$(basename $EXAMPLE .cpp)

# Define paths
COMPILER=$BUILD_DEST/llvm/bin/clang++
TARGET=riscv64-unknown-linux-musl
TOOLCHAIN=$BUILD_DEST/riscv
SYSROOT=$BUILD_DEST/riscv/sysroot

# Define the full command using the variables
CC="$COMPILER --target=$TARGET --gcc-toolchain=$TOOLCHAIN --sysroot=$SYSROOT"
CXX_FLAGS="-static"

# Compile the OpenMP example
$CC $CXX_FLAGS $EXAMPLE -o $EXE_OUTPUT
