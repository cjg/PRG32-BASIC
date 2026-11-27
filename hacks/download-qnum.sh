#!/usr/bin/env bash

set -e

SOURCE_DIR=$1
BINARY_DIR=$2

rm -rf "${BINARY_DIR}/Qnum"
git clone https://github.com/Aviato-Dynamics/Qnum.git "${BINARY_DIR}/Qnum"
cp "${BINARY_DIR}/Qnum/qnum.h" "${SOURCE_DIR}/src"

