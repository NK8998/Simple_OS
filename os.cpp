#include <iostream>
#include <queue>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <string>
#include <map>
#include <thread>
#include <optional>
#include <mutex>
#include <sstream>

std::mutex ready_mtx;
std::mutex job_mtx;
std::mutex page_mtx;
std::mutex waiting_mtx;
std::mutex terminated_mtx;
std::mutex admitted_mtx;
std::mutex print_mtx;

class MemoryManager;
class ProcessManager;
class IOManager;

struct MemoryBlock
{
    int start;
    int size;
};

void safe_print_function(const std::string &s)
{
    std::lock_guard<std::mutex> print_lock(print_mtx);
    std::cout << s << std::endl;
}

class Task
{
private:
    int id;
    std::string name;
    std::string state;
    int run_time;
    long admission_time;
    long io_task_duration;

public:
    Task(int id, std::string name, int run_time, long admission_time, long io_duration)
        : id(id), name(std::move(name)), run_time(run_time), admission_time(admission_time), state("new"), io_task_duration(io_duration) {}

    int get_id() const { return id; }
    std::string get_name() const { return name; }
    int get_run_time() const { return run_time; }
    void set_run_time(int rt) { run_time = rt; }
    std::string get_state() const { return state; }
    void set_state(const std::string &s) { state = s; }
    long get_io_duration() const { return io_task_duration; }
    void set_io_duration(long io_duration) { io_task_duration = io_duration; }
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
        if (empty())
            throw std::runtime_error("Queue empty");
        T item = std::move(buffer[head]);
        head = (head + 1) % Capacity;
        count--;
        return item;
    }

    std::optional<T> front() const
    {
        if (empty())
            return std::nullopt;
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
    int tasks_admitted = 0;

    static const int MAX_READY_QUEUE_LENGTH = 3; // mem size
    static const int MAX_JOB_QUEUE_LENGTH = 6;
    static const int MAX_PAGE_FILE_SIZE = 3;
    static const int MAX_WAITING_LENGTH = 4;

    StaticQueue<Task *, MAX_READY_QUEUE_LENGTH> ready_queue;
    StaticQueue<Task *, MAX_JOB_QUEUE_LENGTH> job_queue;
    StaticQueue<Task *, MAX_PAGE_FILE_SIZE> page_file;
    StaticQueue<Task *, MAX_WAITING_LENGTH> waiting_queue;

    std::vector<Task *> terminated_tasks;

public:
    void add_to_ready_queue()
    {

        std::lock_guard<std::mutex> ready_lock(ready_mtx);
        std::lock_guard<std::mutex> job_lock(job_mtx);
        std::lock_guard<std::mutex> page_lock(page_mtx);

        if (!ready_queue.full())
        {

            Task *task = nullptr;

            if (!job_queue.empty())
            {
                auto t = job_queue.front();
                if (t)
                {
                    task = job_queue.pop();
                }
            }
            else if (!page_file.empty())
            {
                auto t = page_file.front();
                if (t)
                {
                    task = page_file.pop();
                }
            }
            else
            {
                safe_print_function("job queue size: " + std::to_string(job_queue.size()));
                safe_print_function("page queue size: " + std::to_string(page_file.size()));
                safe_print_function("ready queue size: " + std::to_string(ready_queue.size()));
                safe_print_function("unhandled operation");
            }

            if (task)
            {
                safe_print_function("adding: " + task->get_name());
                task->set_state("ready");
                ready_queue.push(task);
            }
        }
    }

    void task_terminated(Task *task)
    {
        std::lock_guard<std::mutex> term_lock(terminated_mtx);
        terminated_tasks.push_back(task);
        safe_print_function("Task finished: " + task->get_name());
    }

    void add_to_job_queue(Task *task, int task_size)
    {
        std::lock_guard<std::mutex> lock(job_mtx);

        if (!job_queue.full())
        {
            tasks_admitted++;
            job_queue.push(task);
            safe_print_function("Task submitted: " + task->get_name());
            safe_print_function("job queue size: " + std::to_string(job_queue.size()));
        }
        else
        {
            safe_print_function("Job queue full... upgrade your system");
        }
    }

    Task *get_ready_task()
    {
        std::lock_guard<std::mutex> ready_lock(ready_mtx);

        if (ready_queue.empty())
            return nullptr;
        auto t = ready_queue.front();
        if (t)
        {
            return ready_queue.pop();
        }

        return nullptr;
    }

    Task *get_waiting_task()
    {
        std::lock_guard<std::mutex> waiting_lock(waiting_mtx);

        if (waiting_queue.empty())
            return nullptr;
        auto t = waiting_queue.front();
        if (t)
        {
            return waiting_queue.pop();
        }
        return nullptr;
    }

    void add_to_waiting_queue(Task *task)
    {
        std::lock_guard<std::mutex> waiting_lock(waiting_mtx);

        if (!waiting_queue.full())
        {
            waiting_queue.push(task);
        }
        else
        {
            // If full then move task to ready queue or page so it can be retried later
            add_to_ready_or_page(task);
            safe_print_function("waiting queue full, will retry later");
        }
    }

    void add_to_ready_or_page(Task *task)
    {
        std::lock_guard<std::mutex> ready_lock(ready_mtx);
        std::lock_guard<std::mutex> page_lock(page_mtx);

        if (!ready_queue.full())
        {
            ready_queue.push(task);
        }
        else
        {
            if (!page_file.full())
            {
                page_file.push(task);
            }
            else
            {
                safe_print_function("page file full.....sorry. Terminating task anyway");
                task_terminated(task);
            }
        }
    }
    std::vector<Task *> get_terminated_tasks()
    {

        std::lock_guard<std::mutex> term_lock(terminated_mtx);
        return terminated_tasks;
    }
    int get_tasks_submitted()
    {
        std::lock_guard<std::mutex> admitted_lock(admitted_mtx);
        return tasks_admitted;
    }
};

class ProcessManager
{
private:
    const int burst_time = 4; // time slice (ms)
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
                task_running = false;
            }
            else
            {
                if (task->get_io_duration() > 0)
                {
                    safe_print_function("Sending " + task->get_name() + "to waiting queue");
                    task->set_state("waiting");
                    mm->add_to_waiting_queue(task);
                    task_running = false;
                }
                else
                {
                    task->set_run_time(new_run_time);
                    task->set_state("running");
                    task_running = true;

                    safe_print_function("-> Running Task: " + task->get_name() + " (remaining: " + std::to_string(new_run_time) + ")" + "IO Duration: " + std::to_string(task->get_io_duration()));

                    std::this_thread::sleep_for(std::chrono::milliseconds(burst_time));

                    task->set_state("ready");
                    mm->add_to_ready_or_page(task);
                    task_running = false;
                }
            }
        }
    }
};

class IOManager
{
private:
    MemoryManager *mm;

public:
    void set_mm_pointer(MemoryManager *mm) { this->mm = mm; }

    void handle_waiting_tasks()
    {
        Task *task = mm->get_waiting_task();
        if (task)
        {
            std::string name = task->get_name();

            safe_print_function("Fulfilling request for " + name);
            std::this_thread::sleep_for(std::chrono::milliseconds(task->get_io_duration()));

            task->set_io_duration(0);
            // add back to ready queue
            mm->add_to_ready_or_page(task);
            safe_print_function("Done fulfilling request for " + name);
        }
    }
};

class OS
{
private:
    int tick_ms;
    int task_id_counter = 0;
    MemoryManager mm;
    ProcessManager pm;
    IOManager im;

public:
    OS(int tick)
    {
        tick_ms = tick;
        pm.set_mm_pointer(&mm);
        im.set_mm_pointer(&mm);
    }

    long get_now()
    {
        using namespace std::chrono;
        return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
    }

    void create_task(const std::string &name)
    {
        int run_time = rand() % 50 + 10;
        long io_duration = rand() % 100 + 10;

        Task *task = new Task(++task_id_counter, name, run_time, get_now(), io_duration);
        safe_print_function("Created task: " + task->get_name() + " with runtime " + std::to_string(run_time));
        mm.add_to_job_queue(task, 10);
    }

    void start_main_thread()
    {
        // std::ostringstream ss;

        // ss << "Thread ID: " << std::this_thread::get_id();

        // safe_print_function(ss.str());

        while (mm.get_terminated_tasks().size() < mm.get_tasks_submitted())
        {
            mm.add_to_ready_queue();
            pm.run_task();
            std::this_thread::sleep_for(std::chrono::milliseconds(tick_ms));
        }
    }

    void start_io_thread()
    {
        // std::ostringstream ss;

        // ss << "Thread ID: " << std::this_thread::get_id();

        // safe_print_function(ss.str());

        while (mm.get_terminated_tasks().size() < mm.get_tasks_submitted())
        {
            im.handle_waiting_tasks();
            std::this_thread::sleep_for(std::chrono::microseconds(tick_ms));
        }
    }

    void start_scheduler()
    {

        std::thread main_thread(&OS::start_main_thread, this);
        std::thread io_thread(&OS::start_io_thread, this);

        if (io_thread.joinable())
        {
            io_thread.join();
        }

        if (main_thread.joinable())
        {
            main_thread.join();
        }

        std::cout << "\n=== Terminated Tasks ===\n";
        for (auto *task : mm.get_terminated_tasks())
        {
            std::cout << "||  " << task->get_name() << " (id " << task->get_id() << ")\n";
        }
    }
};

int main()
{
    try
    {
        srand(time(nullptr));

        OS os(1);

        os.create_task("browser");
        os.create_task("player");
        os.create_task("editor");
        os.create_task("emulator");
        os.create_task("file explorer");
        os.create_task("terminal");

        os.start_scheduler();
    }
    catch (const std::exception &e)
    {
        std::cerr << "unhandled exception: " << e.what() << std::endl;
    }

    return 0;
}
