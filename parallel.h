/* This file is adapted from the CMU Problem-Based Benchmark Suite,
 * https://github.com/cmuparlay/pbbslib
 */

#pragma once
#include <iostream>

static std::string scheduler_name();

static int num_workers();

static int worker_id();

// parallel loop from start (inclusive) to end (exclusive) running
// function f.
//    f should map long to void.
//    granularity is the number of iterations to run sequentially
//      if 0 (default) then the scheduler will decide
//    conservative uses a safer scheduler
template <typename F>
static void parallel_for(long start, long end, F f,
			 long granularity = 0,
			 bool conservative = false);

// runs the thunks left and right in parallel.
//    both left and write should map void to void
//    conservative uses a safer scheduler
template <typename Lf, typename Rf>
static void par_do(Lf left, Rf right, bool conservative=false);

//***************************************

// cilkplus
#if defined(CILK)
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <iostream>
#include <sstream>
#define PAR_GRANULARITY 2000

inline std::string scheduler_name() {
  return "Cilk";
}

inline int num_workers() {return __cilkrts_get_nworkers();}
inline int worker_id() {return __cilkrts_get_worker_number();}
inline void set_num_workers(int) {
  throw std::runtime_error("don't know how to set worker count!");
}
// Not sure this still works
//__cilkrts_end_cilk();
//  std::stringstream ss; ss << n;
//  if (0 != __cilkrts_set_param("nworkers", ss.str().c_str()))


template <typename Lf, typename Rf>
inline void par_do(Lf left, Rf right, bool) {
    cilk_spawn right();
    left();
    cilk_sync;
}

template <typename F>
inline void parallel_for(long start, long end, F f,
			 long granularity,
			 bool) {
  if (granularity == 0)
    cilk_for(long i=start; i<end; i++) f(i);
  else if ((end - start) <= granularity)
    for (long i=start; i < end; i++) f(i);
  else {
    long n = end-start;
    long mid = (start + (9*(n+1))/16);
    cilk_spawn parallel_for(start, mid, f, granularity);
    parallel_for(mid, end, f, granularity);
    cilk_sync;
  }
}

// openmp
#elif defined(OPENMP)
#include <omp.h>
#define PAR_GRANULARITY 200000

inline std::string scheduler_name() {
  return "OpenMP";
}

inline int num_workers() { return omp_get_max_threads(); }
inline int worker_id() { return omp_get_thread_num(); }
inline void set_num_workers(int n) { omp_set_num_threads(n); }

template <class F>
inline void parallel_for(long start, long end, F f,
			 long granularity,
			 bool conservative) {
  _Pragma("omp parallel for")
    for(long i=start; i<end; i++) f(i);
}

bool in_par_do = false;

template <typename Lf, typename Rf>
inline void par_do(Lf left, Rf right, bool conservative) {
  if (!in_par_do) {
    in_par_do = true;  // at top level start up tasking
#pragma omp parallel
#pragma omp single
#pragma omp task
    left();
#pragma omp task
    right();
#pragma omp taskwait
    in_par_do = false;
  } else {   // already started
#pragma omp task
    left();
#pragma omp task
    right();
#pragma omp taskwait
  }
}

template <typename Job>
inline void parallel_run(Job job, int num_threads=0) {
  job();
}

// taskparts
#elif defined(TASKPARTS_POSIX)

#include <taskparts/benchmark.hpp>
#include <algorithm>

inline std::string scheduler_name() {
  return "taskparts";
}

bool taskparts_launched = false;

inline int num_workers() { return taskparts::perworker::nb_workers(); }
inline int worker_id() { return taskparts::perworker::my_id(); }

using taskparts_scheduler = taskparts::bench_scheduler;

template <typename Lf, typename Rf>
inline void par_do(Lf left, Rf right, bool) {
  if (taskparts_launched) {
    taskparts::fork2join<Lf, Rf, taskparts_scheduler>(left, right);
  } else {
    left();
    right();
  }
}

template <typename F>
size_t get_granularity(size_t start, size_t end, F f) {
  size_t done = 0;
  size_t sz = 1;
  int ticks = 0;
  do {
    sz = std::min(sz, end - (start + done));
    auto tstart = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < sz; i++) f(start + done + i);
    auto tstop = std::chrono::high_resolution_clock::now();
    ticks = static_cast<int>((tstop - tstart).count());
    done += sz;
    sz *= 2;
  } while (ticks < 1000 && done < (end - start));
  return done;
}

template <typename F>
void parfor_(size_t start, size_t end, F f, size_t granularity,
	     bool conservative) {
  if ((end - start) <= granularity)
    for (size_t i = start; i < end; i++) f(i);
  else {
    size_t n = end - start;
    // Not in middle to avoid clashes on set-associative
    // caches on powers of 2.
    size_t mid = (start + (9 * (n + 1)) / 16);
    par_do([&]() { parfor_(start, mid, f, granularity, conservative); },
	   [&]() { parfor_(mid, end, f, granularity, conservative); },
	   conservative);
  }
}

  
template <typename F>
inline void parallel_for(long start, long end, F f,
			 long granularity,
			 bool conservative) {
  if (end <= start) return;
  if (granularity == 0) {
    long done = get_granularity(start, end, f);
    granularity = std::max(done, (end - start) / (128 * num_workers()));
    parfor_(start + done, end, f, granularity, conservative);
  } else
    parfor_(start, end, f, granularity, conservative);
}

template <typename Benchmark,
	  typename Benchmark_setup=decltype(taskparts::dflt_benchmark_setup),
	  typename Benchmark_teardown=decltype(taskparts::dflt_benchmark_teardown)
>
  auto benchmark_taskparts(const Benchmark& benchmark,
			   Benchmark_setup benchmark_setup=taskparts::dflt_benchmark_setup,
			   Benchmark_teardown benchmark_teardown=taskparts::dflt_benchmark_teardown) {
  taskparts::benchmark_nativeforkjoin([&] (auto sched) {
    taskparts_launched = true;
    benchmark(sched);
  }, benchmark_setup, benchmark_teardown);
}

// c++
#else

inline std::string scheduler_name() {
  return "sequential elision";
}

inline int num_workers() { return 1;}
inline int worker_id() { return 0;}
inline void set_num_workers(int) { ; }
#define PAR_GRANULARITY 1000

template <class F>
inline void parallel_for(long start, long end, F f,
			 long,   // granularity,
			 bool) { // conservative) {
  for (long i=start; i<end; i++) {
    f(i);
  }
}

template <typename Lf, typename Rf>
inline void par_do(Lf left, Rf right, bool) { // conservative) {
  left(); right();
}

template <typename Job>
inline void parallel_run(Job job, int) { // num_threads=0) {
  job();
}

#endif
