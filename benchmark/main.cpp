#include <cfds/small_vector.hpp>
#include <benchmark/benchmark.h>
#include <memory>
#include <vector>

static void BM_SmallVectorPushBackOne(benchmark::State& state) {
    for (auto _ : state) {
        cfds::small_vector<std::unique_ptr<int>, 1> v;
        benchmark::DoNotOptimize(v.data());
        v.push_back(std::unique_ptr<int>(new int(1)));
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_SmallVectorPushBackOne);

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

static void BM_SmallVectorPushBackOverflow(benchmark::State& state) {
    for (auto _ : state) {
        cfds::small_vector<std::unique_ptr<int>, 4> v;
        benchmark::DoNotOptimize(v.data());
        for (int i = 0; i < 5; ++i) {
            v.push_back(std::unique_ptr<int>(new int(i)));
        }
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_SmallVectorPushBackOverflow);

static void BM_StdVectorPushBackOverflow(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<std::unique_ptr<int>> v;
        v.reserve(4);
        benchmark::DoNotOptimize(v.data());
        for (int i = 0; i < 5; ++i) {
            v.push_back(std::unique_ptr<int>(new int(i)));
        }
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_StdVectorPushBackOverflow);

BENCHMARK_MAIN();
