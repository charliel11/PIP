#include <Image.h>
#include <benchmark/benchmark.h>
#include <iostream>
#include <tbb/tbb.h>
#include <vcruntime.h>
#include <vector>

using namespace std;

int main() {
    Image a;
    cout << "Hello PIP!" << endl;
    tbb::parallel_invoke([]() { cout << " Hello " << endl; }, []() { cout << " TBB! " << endl; });

    return 0;
}