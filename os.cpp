#include <iostream>
#include <queue>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <string>

class Task {
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
    void set_state(const std::string& s) { state = s; }
};

class OS {
private:
    const int burst_time = 1;             // time slice (ms)
    const int MAX_READY_QUEUE_LENGTH = 3;
    bool task_running = false;

    std::queue<Task*> ready_queue;
    std::queue<Task*> job_queue;
    std::queue<Task*> requeue_buffer;
    std::vector<Task*> terminated_tasks;

    int task_id_counter = 0;

public:
    void create_task(const std::string& name) {
        int run_time = rand() % 50 + 10; // random between 10–60ms
        Task* task = new Task(++task_id_counter, name, run_time, get_now());
        job_queue.push(task);
        std::cout << "Created task: " << task->get_name() << " with runtime " << run_time << "ms\n";
    }

    void start_scheduler(int iterations, int tick_ms) {
        for (int i = 0; i < iterations; i++) {
            add_to_ready_queue();
            run_task();
            std::this_thread::sleep_for(std::chrono::milliseconds(tick_ms));
        }
        std::cout << "\n=== Terminated Tasks ===\n";
        for (auto* task : terminated_tasks) {
            std::cout << "✔ " << task->get_name() << " (id " << task->get_id() << ")\n";
        }
    }

private:
    long get_now() {
        using namespace std::chrono;
        return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
    }

    void run_task() {
        if (!task_running && !ready_queue.empty()) {
            Task* task = ready_queue.front();
            ready_queue.pop();

            int new_run_time = task->get_run_time() - burst_time;

            if (new_run_time <= 0) {
                task->set_state("terminated");
                terminated_tasks.push_back(task);
                std::cout << "✅ Task finished: " << task->get_name() << "\n";
            } else {
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

    void add_to_ready_queue() {
        while (!requeue_buffer.empty()) {
            ready_queue.push(requeue_buffer.front());
            requeue_buffer.pop();
        }

        if (ready_queue.size() < MAX_READY_QUEUE_LENGTH && !job_queue.empty()) {
            Task* task = job_queue.front();
            job_queue.pop();
            task->set_state("ready");
            ready_queue.push(task);
        }
    }
};

int main() {
    srand(time(nullptr));

    OS os;
    os.create_task("browser");
    os.create_task("player");
    os.create_task("editor");
    os.create_task("emulator");
    os.create_task("file explorer");
    os.create_task("terminal");

    os.start_scheduler(200, 20); // run for 200 ticks, 20ms each
    return 0;
}
