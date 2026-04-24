#include "multi-thread.h"
#include <thread>
#include <semaphore>
#include <memory>
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
        } 
        else 
        {
            while (generation.load(memory_order_acquire) == current_gen);
        }
    }
};

struct lin_solve_data
{
    int bnd;
    vector<float> *x;
    const vector<float> *x0;
    float a, c;
    fluid_container *container;
    bool active_box;
};

int thread_count = 1;
static vector<jthread> workers;
static atomic_flag initialized = ATOMIC_FLAG_INIT;

// using a unique_ptr to bypass the need for initializing a thread number
static unique_ptr<SpinBarrier> sync_barrier;
static counting_semaphore<> start_signal{0};
static counting_semaphore<> done_signal{0};

// shared data for the threads
lin_solve_data cur_lin_solve;

static inline void get_bounds(fluid_container* container, bool active_box, int& start_x, int& start_y, int& end_x, int& end_y)
{
    if (active_box)
    {
        start_x = container->min_x; 
        start_y = container->min_y;
        end_x = container->max_x;
        end_y = container->max_y;
    }
    else
    {
        start_x = 1;
        start_y = 1;
        end_x = container->width; 
        end_y = container->height;
    }
}

static inline void get_row_slice(int thread_idx, int start_y, int end_y, int& my_start_y, int& my_end_y)
{
    int total_rows = end_y - start_y + 1;
    int base_rows  = total_rows / thread_count;
    int remainder  = total_rows % thread_count;

    my_start_y  = start_y + (thread_idx * base_rows) + min(thread_idx, remainder);
    int my_rows = base_rows + (thread_idx < remainder ? 1 : 0);
    my_end_y    = my_start_y + my_rows - 1;
}

static void do_lin_solve(int thread_idx)
{
    const lin_solve_data& job = cur_lin_solve;

    int start_x, start_y, end_x, end_y;
    get_bounds(job.container, job.active_box, start_x, start_y, end_x, end_y);

    int my_start_y, my_end_y;
    get_row_slice(thread_idx, start_y, end_y, my_start_y, my_end_y);

    int grid_stride = job.container->width + 2;

    for (int i = 0; i < LIN_SOL_MAX; i++)
    {
        lin_solve_chunk(*job.x, *job.x0, job.a, job.c, start_x, end_x, my_start_y, my_end_y, grid_stride, 0);
        sync_barrier->arrive_and_wait();

        lin_solve_chunk(*job.x, *job.x0, job.a, job.c, start_x, end_x, my_start_y, my_end_y, grid_stride, 1);
        sync_barrier->arrive_and_wait();

        if (thread_idx == 0)
            set_bnd(job.bnd, *job.x, *job.container);
        sync_barrier->arrive_and_wait();
    }
}

void fluid_worker(stop_token stoken, int thread_idx)
{
    while (!stoken.stop_requested())
    {
        start_signal.acquire();

        if (stoken.stop_requested()) break;

        do_lin_solve(thread_idx);

        done_signal.release();
    }
}

void init_fluid_threads() 
{
    if (initialized.test_and_set()) return;

    sync_barrier = make_unique<SpinBarrier>(thread_count);
    for (int i = 0; i < thread_count; i++)
        workers.emplace_back(fluid_worker, i);
}

void shutdown_fluid_threads()
{
    for (auto& t : workers) 
    {
        t.request_stop();
    }
    start_signal.release(thread_count);
    
    workers.clear();
}

void threaded_lin_solve(int boundary_t, std::vector<float>& x, const std::vector<float>& x0, float a, float c, fluid_container& container, bool active_box)
{
    init_fluid_threads();

    cur_lin_solve = { boundary_t, &x, &x0, a, c, &container, active_box };

    start_signal.release(thread_count);

    for (int i = 0; i < thread_count; i++)
        done_signal.acquire();
}