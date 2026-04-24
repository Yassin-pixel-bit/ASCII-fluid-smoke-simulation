#include "multi-thread.h"
#include <thread>
#include <barrier>
#include <semaphore>
#include <memory>
#include <chrono>
#include <atomic>

using namespace std;

class SpinBarrier 
{
    atomic<int> count;
    atomic<int> generation;
    int thread_count;
public:
    SpinBarrier(int n) : count(n), generation(0), thread_count(n) {}

    void arrive_and_wait() 
    {
        int current_gen = generation.load(memory_order_acquire);
        if (count.fetch_sub(1, memory_order_acq_rel) == 1) 
        {
            count.store(thread_count, memory_order_relaxed);
            generation.fetch_add(1, memory_order_release);
        } else 
        {
            // Pure spin loop.
            while (generation.load(memory_order_acquire) == current_gen);
        }
    }


};

static int thread_count = std::max(1u, std::thread::hardware_concurrency() / 2);
static vector<jthread> workers;
// using a unique_ptr to bypass the need for initializing a thread number to std::barrier
// TODO: remove unique_ptr<>
static unique_ptr<SpinBarrier> sync_barrier;
static counting_semaphore<> start_signal{0};
static counting_semaphore<> done_signal{0};

// shared data for the threads
static int cur_bnd;
static vector<float>* cur_x;
static const vector<float>* cur_x0;
static float cur_a;
static float cur_c;
static fluid_container* cur_container;
static bool cur_active_box;

// TODO: simplify
void fluid_worker(stop_token stoken, int thread_idx)
{
    while(!stoken.stop_requested())
    {
        start_signal.acquire();

        if (stoken.stop_requested()) break;

        int grid_stride = cur_container->width + 2;
        int start_x, start_y, end_x, end_y;

        if (cur_active_box)
        {
            start_x = cur_container->min_x;
            start_y = cur_container->min_y;
            end_x = cur_container->max_x;
            end_y = cur_container->max_y;
        }
        else
        {
            start_x = 1; 
            start_y = 1; 
            end_x = cur_container->width; 
            end_y = cur_container->height;
        }

        int total_rows = end_y - start_y + 1;
        
        int base_rows = total_rows / thread_count;
        int remainder = total_rows % thread_count;

        // Distribute the remainder evenly among the first few threads
        int my_start_y = start_y + (thread_idx * base_rows) + std::min(thread_idx, remainder);
        int my_rows = base_rows + (thread_idx < remainder ? 1 : 0);
        int my_end_y = my_start_y + my_rows - 1;

        for (int i = 0; i < LIN_SOL_MAX; i++)
        {
            lin_solve_chunk(*cur_x, *cur_x0, cur_a, cur_c, start_x, end_x, my_start_y, my_end_y, grid_stride, 0);
            sync_barrier->arrive_and_wait();

            lin_solve_chunk(*cur_x, *cur_x0, cur_a, cur_c, start_x, end_x, my_start_y, my_end_y, grid_stride, 1);
            sync_barrier->arrive_and_wait();

            // hard-coding it for some fuck-ass reason
            if (thread_idx == 0)
            {
                set_bnd(cur_bnd, *cur_x, *cur_container);
            }
            sync_barrier->arrive_and_wait();
        }

        done_signal.release();
    }
}

// TODO: see if I can avoid calling it every time in threaded_lin_solve, it's highly possible
void init_fluid_threads() 
{
    if (!workers.empty()) return;
    
    sync_barrier = make_unique<SpinBarrier>(thread_count);
    for (int i = 0; i < thread_count; i++) {
        workers.emplace_back(fluid_worker, i);
    }
}

void shutdown_fluid_threads()
{
    for (auto& t : workers) {
        t.request_stop();
    }
    start_signal.release(thread_count);
    
    workers.clear();
}

void threaded_lin_solve(int boundary_t, std::vector<float>& x, const std::vector<float>& x0, float a, float c, fluid_container& container, bool active_box)
{
    init_fluid_threads();

    cur_bnd = boundary_t;
    cur_x = &x;
    cur_x0 = &x0;
    cur_a = a;
    cur_c = c;
    cur_container = &container;
    cur_active_box = active_box;

    start_signal.release(thread_count);

    for (int i = 0; i < thread_count; i++) 
    {
        done_signal.acquire();
    }
}