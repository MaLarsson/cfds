#include <SmallVector.hpp>
#include <benchmark/benchmark.h>
#include <memory>
#include <vector>

static void BM_SmallVectorAppendOne(benchmark::State& state) {
    for (auto _ : state) {
        SmallVector<std::unique_ptr<int>, 1> v;
        benchmark::DoNotOptimize(v.data());
        v.append(std::unique_ptr<int>(new int(1)));
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_SmallVectorAppendOne);

static void BM_StdVectorPushBackOne(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<std::unique_ptr<int>> v;
        v.reserve(1);
        benchmark::DoNotOptimize(v.data());
        v.push_back(std::unique_ptr<int>(new int(1)));
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_StdVectorPushBackOne);

static void BM_SmallVectorAppendOverflow(benchmark::State& state) {
    for (auto _ : state) {
        SmallVector<std::unique_ptr<int>, 4> v;
        benchmark::DoNotOptimize(v.data());
        for (int i = 0; i < 5; ++i)
            v.append(std::unique_ptr<int>(new int(i)));
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_SmallVectorAppendOverflow);

static void BM_StdVectorPushBackOverflow(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<std::unique_ptr<int>> v;
        v.reserve(4);
        benchmark::DoNotOptimize(v.data());
        for (int i = 0; i < 5; ++i)
            v.push_back(std::unique_ptr<int>(new int(i)));
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_StdVectorPushBackOverflow);

BENCHMARK_MAIN();
