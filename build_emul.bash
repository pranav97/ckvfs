rm -rf build
mkdir build && cd build
cmake -DWITH_EMU=ON -DSAMSUNG_API=ON ../
make
echo "Run the following command for Tests"
echo "cd build; ./sample_code_async -d /dev/kvemul -n 1000 -q 64 -o 1 -k 16 -v 4096"
