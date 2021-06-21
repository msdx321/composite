#!/bin/sh
cd src && make && cd ..
./cos compose composition_scripts/ext4_test.toml ext4_test
objdump -Srhtl /home/msdx321/workspace/composite-sqlite/system_binaries/cos_build-ext4_test/global.ext4/filesystem.ext4.global.ext4 >| ~/workspace/composite-sqlite/dump-object.txt
./tools/run.sh system_binaries/cos_build-ext4_test/cos.img