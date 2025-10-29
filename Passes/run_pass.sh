#!/bin/bash
# run_pass.sh - Run AnalyzeInstrPass on test.c

# Paths
PASS_SO="./build/AnalyzeInstrPass.so"
SRC="test.c"
BC="test.bc"

# Step 1: Compile C to LLVM bitcode
echo "Compiling $SRC to LLVM bitcode..."
clang -O1 -Xclang -disable-llvm-passes -emit-llvm -c "$SRC" -o "$BC"
if [ $? -ne 0 ]; then
    echo "Error: Failed to compile $SRC"
    exit 1
fi

# Step 2: Run your pass using opt
echo "Running AnalyzeInstrPass..."
opt -load-pass-plugin "$PASS_SO" \
    -passes=analyze-mops \
    -disable-output "$BC"
if [ $? -ne 0 ]; then
    echo "Error: Failed to run AnalyzeInstrPass"
    exit 1
fi

echo "Done!"
