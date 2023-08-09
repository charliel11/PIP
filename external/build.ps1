if ($IsWindows) {
    function Invoke-CmdScript {
        param(
            [String] $scriptName
        )
        $cmdLine = """$scriptName"" $args & set"
        & $Env:SystemRoot\system32\cmd.exe /c $cmdLine |
        select-string '^([^=]*)=(.*)$' | foreach-object {
            $varName = $_.Matches[0].Groups[1].Value
            $varValue = $_.Matches[0].Groups[2].Value
            set-item Env:$varName $varValue
        }
    }
    
    Invoke-CmdScript("C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat")
    $CC = "-DCMAKE_C_COMPILER:string=clang-cl" 
    $CXX = "-DCMAKE_CXX_COMPILER:string=clang-cl"
}
if ($IsLinux) {
    $CC = "-DCMAKE_C_COMPILER:string=clang"
    $CXX = "-DCMAKE_CXX_COMPILER:string=clang++"
    $GENERATOR = "-DCMAKE_MAKE_PROGRAM:FILEPATH=/home/charlie/project/PIP/external/install/Release/bin/ninja"
}

$d = "ninja"
cmake -S $d -B $d/build -DCMAKE_INSTALL_PREFIX="install/Release"
cmake --build $d/build --config Release --target install -j 20
Remove-Item $d/build -Force -Recurse


$config = @("Debug", "Release", "RelWithDebInfo")
$d = "oneTBB"
foreach ($c in $config) {
    cmake -S $d -B $d/build -DCMAKE_INSTALL_PREFIX="install/$c" -DBUILD_SHARED_LIBS=ON -DTBB_TEST=OFF "-DCMAKE_BUILD_TYPE:string=$c" -G "Ninja" $CC $CXX $GENERATOR
    cmake --build $d/build --config $c --target install -j 20
}
Remove-Item $d/build -Force -Recurse

$d = "googletest"
foreach ($c in $config) {
    cmake -S $d -B $d/build -DCMAKE_INSTALL_PREFIX="install/$c" -DBUILD_SHARED_LIBS=ON "-DCMAKE_BUILD_TYPE:string=$c" -G "Ninja" $CC $CXX $GENERATOR
    cmake --build $d/build --config $c --target install -j 20
}
Remove-Item $d/build -Force -Recurse

$d = "benchmark"
foreach ($c in $config) {
    cmake -S $d -B $d/build -DCMAKE_INSTALL_PREFIX="install/$c" -DBENCHMARK_ENABLE_TESTING=OFF "-DCMAKE_BUILD_TYPE:string=$c" -G "Ninja" $CC $CXX $GENERATOR
    cmake --build $d/build --config $c --target install -j 20
}
Remove-Item $d/build -Force -Recurse