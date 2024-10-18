#!/bin/bash
python3 wasi-testsuite/test-runner/wasi_test_runner.py -t ./wasi-testsuite/tests/assemblyscript/testsuite/ ./wasi-testsuite/tests/c/testsuite/ -r ./bytebox_adapter.sh
