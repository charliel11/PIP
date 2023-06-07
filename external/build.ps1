$config = @("Debug", "Release", "RelWithDebInfo")

$d = "oneTBB"
foreach ($c in $config) {
    cmake -S $d -B $d/build -DCMAKE_INSTALL_PREFIX="install/$c" -DTBB_TEST=OFF
    cmake --build $d/build --config $c --target install
}
Remove-Item $d/build -Force

$d = "googletest"
foreach ($c in $config) {
    cmake -S $d -B $d/build -DCMAKE_INSTALL_PREFIX="install/$c" -DBUILD_SHARED_LIBS=ON
    cmake --build $d/build --config $c --target install
}
Remove-Item $d/build -Force

$d = "benchmark"
foreach ($c in $config) {
    cmake -S $d -B $d/build -DCMAKE_INSTALL_PREFIX="install/$c" -DBENCHMARK_ENABLE_TESTING=OFF
    cmake --build $d/build --config $c --target install
}
Remove-Item $d/build -Force