#include <string>
#include <vector>

class Task
{
private:
    std::string state;
    int run_time;
    int id;
    std::string name;
    int addmission_time;

public:
    Task(int id, std::string name, int run_time)
        : id(id), name(name), run_time(run_time), state("new") {};
    void set_run_time(int run_time) { this->run_time = run_time; }
    int get_run_time() { return this->run_time; }
    void set_state(std::string &state) { this->state = state; }
    std::string get_state() { return this->state; }
};

class OS
{
private:
    int burst_time;
    bool task_running = false;
    int MAX_READY_QUEUE_LENGTH;
    std::vector<Task *> ready_queue;
    std::vector<Task *> job_queue;
    std::vector<Task *> requeue_buffer;

    void schedule_loop(int timeout)
    {
    }

public:
    OS(int tick, int burst_time = 1, int MAX_READY_QUEUE_LENGTH = 3)
    {
        this->burst_time = burst_time;
        this->MAX_READY_QUEUE_LENGTH = MAX_READY_QUEUE_LENGTH;
        this->schedule_loop(tick);
    }
};

int main()
{

    return 0;
}