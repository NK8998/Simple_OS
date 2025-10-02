#include <iostream>
#include <queue>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <string>
#include <map>

class MemoryManager;
class ProcessManager;
class IOManager;

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

template <typename T, size_t Capacity>
class StaticQueue
{
private:
    T buffer[Capacity];
    size_t head = 0;
    size_t tail = 0;
    size_t count = 0;

public:
    bool push(const T &item)
    {
        if (count == Capacity)
            return false;
        buffer[tail] = item;
        tail = (tail + 1) % Capacity;
        count++;
        return true;
    }

    T pop()
    {
        if (count == 0)
            return nullptr;
        out = buffer[head];
        head = (head + 1) % Capacity;
        count--;
        return out;
    }

    T front()
    {
        if (empty())
            return nullptr;

        return buffer[head];
    }

    bool empty() const { return count == 0; }
    bool full() const { return count == Capacity; }
    size_t size() const { return count; }

    void clear() { head = tail = count = 0; }
};

class MemoryManager
{
private:
    static const int MAX_READY_QUEUE_LENGTH = 3; // mem size
    static const int MAX_JOB_QUEUE_LENGTH = 4;
    static const int MAX_REQUEUE_BUFFER_LENGTH = 10;
    static const int MAX_PAGE_FILE_SIZE = 5;

    int tasks_admitted = 0;

    StaticQueue<Task *, MAX_READY_QUEUE_LENGTH> ready_queue;
    StaticQueue<Task *, MAX_JOB_QUEUE_LENGTH> job_queue;
    StaticQueue<Task *, MAX_REQUEUE_BUFFER_LENGTH> requeue_buffer;
    StaticQueue<Task *, MAX_PAGE_FILE_SIZE> page_file;

    std::vector<Task *> terminated_tasks;

public:
    bool allocate(Task *task)
    {
        return false;
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
            ready_queue.push(requeue_buffer.pop());
        }

        if (ready_queue.size() < MAX_READY_QUEUE_LENGTH && !job_queue.empty())
        {
            Task *task = job_queue.front();
            job_queue.pop();
            task->set_state("ready");
            ready_queue.push(task);
        }
    }

    void task_terminated(Task *task)
    {
        terminated_tasks.push_back(task);
        std::cout << "Task finished: " << task->get_name() << "\n";
    }

    void add_to_job_qeue(Task *task, int task_size)
    {
        tasks_admitted++;
        job_queue.push(task);
        std::cout << "Task submitted" << task->get_name() << std::endl;
    }

    Task *get_ready_task()
    {
        if (ready_queue.empty())
            return nullptr;
        Task *task = ready_queue.front();
        ready_queue.pop();
        return task;
    }

    void add_to_requeue_buffer(Task *task) { requeue_buffer.push(task); }
    std::vector<Task *> get_terminated_tasks() { return terminated_tasks; }
    int get_tasks_submitted() { return tasks_admitted; }
};

class ProcessManager
{
private:
    const int burst_time = 1; // time slice (ms)
    bool task_running = false;
    MemoryManager *mm;

public:
    void set_mm_pointer(MemoryManager *mm)
    {
        this->mm = mm;
    }

    void run_task()
    {
        if (!task_running)
        {
            Task *task = mm->get_ready_task();

            int new_run_time = task->get_run_time() - burst_time;

            if (new_run_time <= 0)
            {
                task->set_state("terminated");
                mm->task_terminated(task);
            }
            else
            {
                task->set_run_time(new_run_time);
                task->set_state("running");
                task_running = true;

                std::cout << "▶ Running Task: " << task->get_name()
                          << " (remaining: " << new_run_time << ")\n";

                std::this_thread::sleep_for(std::chrono::milliseconds(burst_time));

                task->set_state("ready");
                mm->add_to_requeue_buffer(task);
                task_running = false;
            }
        }
    }
};

class IOManager
{
};

class OS
{
private:
    int task_id_counter = 0;
    MemoryManager mm;
    ProcessManager pm;

public:
    OS()
    {
        pm.set_mm_pointer(&mm);
    }

    long get_now()
    {
        using namespace std::chrono;
        return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
    }

    void create_task(const std::string &name)
    {
        int run_time = rand() % 50 + 10; // random between 10–60ms
        Task *task = new Task(++task_id_counter, name, run_time, get_now());
        mm.add_to_job_qeue(task, 10);
        std::cout << "Created task: " << task->get_name() << " with runtime " << run_time << "ms\n";
    }

    void start_scheduler(int tick_ms)
    {
        while (mm.get_terminated_tasks().size() < mm.get_tasks_submitted())
        {
            mm.add_to_ready_queue();
            pm.run_task();
            std::this_thread::sleep_for(std::chrono::milliseconds(tick_ms));
        }
        std::cout << "\n=== Terminated Tasks ===\n";
        for (auto *task : mm.get_terminated_tasks())
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
