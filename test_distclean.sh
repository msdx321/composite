#!/bin/sh
cd src && make distclean && make init && make && cd ..
./cos compose composition_scripts/ramdisk_test.toml ramdisk_test
#objdump -Srhtl /home/msdx321/workspace/composite-sqlite/system_binaries/cos_build-sqlite_tests/global.sqlite_tests/tests.sqlite_tests.global.sqlite_tests >| ~/workspace/composite-sqlite/dump-object.txt
./tools/run.sh system_binaries/cos_build-ramdisk_test/cos.img