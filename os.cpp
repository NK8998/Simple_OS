#include <iostream>
#include <queue>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <string>
#include <map>

struct MemoryBlock
{
    int start;
    int size;
};

class Task
{
private:
    int id;
    std::string name;
    std::string state;
    int run_time;
    long admission_time;

public:
    Task(int id, std::string name, int run_time, long admission_time)
        : id(id), name(std::move(name)), run_time(run_time), admission_time(admission_time), state("new") {}

    int get_id() const { return id; }
    std::string get_name() const { return name; }
    int get_run_time() const { return run_time; }
    void set_run_time(int rt) { run_time = rt; }
    std::string get_state() const { return state; }
    void set_state(const std::string &s) { state = s; }
};

class ProcessManager
{
private:
    const int burst_time = 1; // time slice (ms)
    bool task_running = false;

public:
    void run_task()
    {
        if (!task_running && !ready_queue.empty())
        {
            Task *task = ready_queue.front();
            ready_queue.pop();

            int new_run_time = task->get_run_time() - burst_time;

            if (new_run_time <= 0)
            {
                task->set_state("terminated");
                terminated_tasks.push_back(task);
                std::cout << "✅ Task finished: " << task->get_name() << "\n";
            }
            else
            {
                task->set_run_time(new_run_time);
                task->set_state("running");
                task_running = true;

                std::cout << "▶ Running Task: " << task->get_name()
                          << " (remaining: " << new_run_time << ")\n";

                // Simulate execution
                std::this_thread::sleep_for(std::chrono::milliseconds(burst_time));

                task->set_state("ready");
                requeue_buffer.push(task);
                task_running = false;
            }
        }
    }
};

class MemoryManager
{
private:
    const int MAX_READY_QUEUE_LENGTH = 3;

    int tasks_admitted = 0;
    std::queue<Task *> ready_queue;
    std::queue<Task *> job_queue;
    std::queue<Task *> requeue_buffer;
    std::vector<Task *> terminated_tasks;
    // std::vector<MemoryBlock> free_blocks;
    std::vector<Task *> page_file;
    // std::map<Task *, MemoryBlock> task_allocations;

    int total_ram = 100;
    int used_ram = 0;

public:
    bool allocate(Task *task)
    {
    }
    void deallocate(Task *task)
    {
    }
    void check_page_file(ProcessManager *pm)
    {
    }

    void add_to_ready_queue()
    {
        while (!requeue_buffer.empty())
        {
            ready_queue.push(requeue_buffer.front());
            requeue_buffer.pop();
        }

        if (ready_queue.size() < MAX_READY_QUEUE_LENGTH && !job_queue.empty())
        {
            Task *task = job_queue.front();
            job_queue.pop();
            task->set_state("ready");
            ready_queue.push(task);
        }
    }

    void submit_task(Task *task, int task_size)
    {
        tasks_admitted++;
        job_queue.push(task);
        std::cout << "Task submitted" << task->get_name() << std::endl;
    }

    std::vector<Task *> get_terminated_tasks() { return terminated_tasks; }
    int get_tasks_submitted() { return tasks_admitted; }
};

class Scheduler
{
};

class OS
{
private:
    int task_id_counter = 0;
    ProcessManager *pm = new ProcessManager();
    MemoryManager *mm = new MemoryManager();

public:
    long get_now()
    {
        using namespace std::chrono;
        return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
    }

    void create_task(const std::string &name)
    {
        int run_time = rand() % 50 + 10; // random between 10–60ms
        Task *task = new Task(++task_id_counter, name, run_time, get_now());
        pm->submit_task(task);
        std::cout << "Created task: " << task->get_name() << " with runtime " << run_time << "ms\n";
    }

    void start_scheduler(int tick_ms)
    {
        while (pm->get_terminated_tasks().size() < pm->get_tasks_submitted())
        {
            mm->check_page_file(pm);
            pm->add_to_ready_queue();
            pm->run_task();
            std::this_thread::sleep_for(std::chrono::milliseconds(tick_ms));
        }
        std::cout << "\n=== Terminated Tasks ===\n";
        for (auto *task : pm->get_terminated_tasks())
        {
            std::cout << "✔ " << task->get_name() << " (id " << task->get_id() << ")\n";
        }
    }
};

int main()
{
    srand(time(nullptr));

    OS os;
    os.create_task("browser");
    os.create_task("player");
    os.create_task("editor");
    os.create_task("emulator");
    os.create_task("file explorer");
    os.create_task("terminal");

    os.start_scheduler(20); // run for 200 ticks, 20ms each
    return 0;
}
