cmake -S ./oneTBB -B ./oneTBB/build -DTBB_TEST=OFF -DCMAKE_INSTALL_PREFIX=install/Release
cmake --build ./oneTBB/build --target install --config Release
cmake -S ./oneTBB -B ./oneTBB/build -DTBB_TEST=OFF -DCMAKE_INSTALL_PREFIX=install/Debug
cmake --build ./oneTBB/build --target install --config Debug

cmake -S ./googletest -B ./googletest/build -DCMAKE_INSTALL_PREFIX=install/Release
cmake --build ./googletest/build --target install --config Release
cmake -S ./googletest -B ./googletest/build -DCMAKE_INSTALL_PREFIX=install/Debug
cmake --build ./googletest/build --target install --config Debug

cmake -S ./benchmark -B ./benchmark/build -DBENCHMARK_ENABLE_TESTING=OFF -DCMAKE_INSTALL_PREFIX=install/Release
cmake --build ./benchmark/build --target install --config Release
cmake -S ./benchmark -B ./benchmark/build -DBENCHMARK_ENABLE_TESTING=OFF -DCMAKE_INSTALL_PREFIX=install/Debug
cmake --build ./benchmark/build --target install --config Debug