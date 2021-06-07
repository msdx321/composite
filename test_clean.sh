#!/bin/sh
cd src && make clean && make && cd ..
./cos compose composition_scripts/sqlite_tests.toml sqlite_tests
./tools/run.sh system_binaries/cos_build-sqlite_tests/cos.img
