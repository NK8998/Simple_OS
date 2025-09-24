class Task {
  public id: number;
  public name: string;
  private state: string = "";
  private run_time: number;
  public admission_time: number = performance.now();

  constructor(id: number, name: string, run_time: number) {
    this.id = id;
    this.name = name;
    this.run_time = run_time;
  }

  set_run_time(run_time: number) {
    this.run_time = run_time;
  }

  get_run_time(): number {
    return this.run_time;
  }

  set_state(state: string) {
    this.state = state;
  }

  get_state(): string {
    return this.state;
  }
}

class OS {
  private burst_time = 1;
  private task_running = false;
  private MAX_READY_QUEUE_LENGTH = 3;
  private ready_queue: Task[] = [];
  private job_queue: Task[] = [];
  private requeue_buffer: Task[] = [];

  constructor() {
    this.schedule_loop(20);
  }

  private schedule_loop = (timeout: number) => {
    this.add_to_ready_queue();
    this.run_task();

    setTimeout(() => {
      this.schedule_loop(timeout);
    }, timeout);
  };

  private run_task = async () => {
    if (!this.task_running) {
      const first_in_ready_queue = this.ready_queue.pop();
      if (first_in_ready_queue) {
        const run_time = first_in_ready_queue.get_run_time();
        const new_run_time = run_time - this.burst_time;

        if (new_run_time <= 0) {
          console.log("finished execution of ", first_in_ready_queue.name);
          this.task_running = false;
        } else {
          first_in_ready_queue.set_run_time(new_run_time);
          first_in_ready_queue.set_state("running");

          console.log({ "Running Task:": first_in_ready_queue.name });

          const burst_time =
            new_run_time < this.burst_time ? new_run_time : this.burst_time;
          this.task_running = true;

          await new Promise((resolve) => {
            setTimeout(() => {
              first_in_ready_queue.set_state("ready");
              this.requeue_buffer.unshift(first_in_ready_queue);
              this.task_running = false;
              resolve(null);
            }, burst_time);
          });
        }
      }
    } else {
      //console.log("task running");
    }
  };

  private add_to_ready_queue = () => {
    console.log(this.ready_queue.length);
    while (this.requeue_buffer.length) {
      const task = this.requeue_buffer.pop();
      if (task) this.ready_queue.unshift(task);
    }

    if (
      this.ready_queue.length < this.MAX_READY_QUEUE_LENGTH &&
      this.job_queue.length > 0
    ) {
      const first_in_job_queue = this.job_queue.pop();

      if (first_in_job_queue) {
        first_in_job_queue.set_state("ready");
        this.ready_queue.unshift(first_in_job_queue);
      }
    }
  };

  private add_to_job_queue = (task: Task) => {
    task.set_state("new");
    this.job_queue.unshift(task);
  };

  public create_task = (name: string) => {
    const id = performance.now();
    const run_time = Math.round(Math.random() * 10000);
    this.add_to_job_queue(new Task(id, name, run_time));
  };
}

const os_instance = new OS();
os_instance.create_task("browser");
os_instance.create_task("player");
os_instance.create_task("editor");
os_instance.create_task("emulator");
os_instance.create_task("file explorer");
os_instance.create_task("terminal");
