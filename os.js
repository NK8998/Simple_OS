var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
var __generator = (this && this.__generator) || function (thisArg, body) {
    var _ = { label: 0, sent: function() { if (t[0] & 1) throw t[1]; return t[1]; }, trys: [], ops: [] }, f, y, t, g = Object.create((typeof Iterator === "function" ? Iterator : Object).prototype);
    return g.next = verb(0), g["throw"] = verb(1), g["return"] = verb(2), typeof Symbol === "function" && (g[Symbol.iterator] = function() { return this; }), g;
    function verb(n) { return function (v) { return step([n, v]); }; }
    function step(op) {
        if (f) throw new TypeError("Generator is already executing.");
        while (g && (g = 0, op[0] && (_ = 0)), _) try {
            if (f = 1, y && (t = op[0] & 2 ? y["return"] : op[0] ? y["throw"] || ((t = y["return"]) && t.call(y), 0) : y.next) && !(t = t.call(y, op[1])).done) return t;
            if (y = 0, t) op = [op[0] & 2, t.value];
            switch (op[0]) {
                case 0: case 1: t = op; break;
                case 4: _.label++; return { value: op[1], done: false };
                case 5: _.label++; y = op[1]; op = [0]; continue;
                case 7: op = _.ops.pop(); _.trys.pop(); continue;
                default:
                    if (!(t = _.trys, t = t.length > 0 && t[t.length - 1]) && (op[0] === 6 || op[0] === 2)) { _ = 0; continue; }
                    if (op[0] === 3 && (!t || (op[1] > t[0] && op[1] < t[3]))) { _.label = op[1]; break; }
                    if (op[0] === 6 && _.label < t[1]) { _.label = t[1]; t = op; break; }
                    if (t && _.label < t[2]) { _.label = t[2]; _.ops.push(op); break; }
                    if (t[2]) _.ops.pop();
                    _.trys.pop(); continue;
            }
            op = body.call(thisArg, _);
        } catch (e) { op = [6, e]; y = 0; } finally { f = t = 0; }
        if (op[0] & 5) throw op[1]; return { value: op[0] ? op[1] : void 0, done: true };
    }
};
var Task = /** @class */ (function () {
    function Task(id, name, run_time) {
        this.state = "";
        this.admission_time = performance.now();
        this.id = id;
        this.name = name;
        this.run_time = run_time;
    }
    Task.prototype.set_run_time = function (run_time) {
        this.run_time = run_time;
    };
    Task.prototype.get_run_time = function () {
        return this.run_time;
    };
    Task.prototype.set_state = function (state) {
        this.state = state;
    };
    Task.prototype.get_state = function () {
        return this.state;
    };
    return Task;
}());
var OS = /** @class */ (function () {
    function OS() {
        var _this = this;
        this.burst_time = 1;
        this.task_running = false;
        this.MAX_READY_QUEUE_LENGTH = 3;
        this.ready_queue = [];
        this.job_queue = [];
        this.requeue_buffer = [];
        this.schedule_loop = function (timeout) {
            _this.add_to_ready_queue();
            _this.run_task();
            setTimeout(function () {
                _this.schedule_loop(timeout);
            }, timeout);
        };
        this.run_task = function () { return __awaiter(_this, void 0, void 0, function () {
            var first_in_ready_queue_1, run_time, new_run_time, burst_time_1;
            var _this = this;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        if (!!this.task_running) return [3 /*break*/, 4];
                        first_in_ready_queue_1 = this.ready_queue.pop();
                        if (!first_in_ready_queue_1) return [3 /*break*/, 3];
                        run_time = first_in_ready_queue_1.get_run_time();
                        new_run_time = run_time - this.burst_time;
                        if (!(new_run_time <= 0)) return [3 /*break*/, 1];
                        console.log("finished execution of ", first_in_ready_queue_1.name);
                        this.task_running = false;
                        return [3 /*break*/, 3];
                    case 1:
                        first_in_ready_queue_1.set_run_time(new_run_time);
                        first_in_ready_queue_1.set_state("running");
                        console.log({ "Running Task:": first_in_ready_queue_1.name });
                        burst_time_1 = new_run_time < this.burst_time ? new_run_time : this.burst_time;
                        this.task_running = true;
                        return [4 /*yield*/, new Promise(function (resolve) {
                                setTimeout(function () {
                                    first_in_ready_queue_1.set_state("ready");
                                    _this.requeue_buffer.unshift(first_in_ready_queue_1);
                                    _this.task_running = false;
                                    resolve(null);
                                }, burst_time_1);
                            })];
                    case 2:
                        _a.sent();
                        _a.label = 3;
                    case 3: return [3 /*break*/, 4];
                    case 4: return [2 /*return*/];
                }
            });
        }); };
        this.add_to_ready_queue = function () {
            console.log(_this.ready_queue.length);
            while (_this.requeue_buffer.length) {
                var task = _this.requeue_buffer.pop();
                if (task)
                    _this.ready_queue.unshift(task);
            }
            if (_this.ready_queue.length < _this.MAX_READY_QUEUE_LENGTH &&
                _this.job_queue.length > 0) {
                var first_in_job_queue = _this.job_queue.pop();
                if (first_in_job_queue) {
                    first_in_job_queue.set_state("ready");
                    _this.ready_queue.unshift(first_in_job_queue);
                }
            }
        };
        this.add_to_job_queue = function (task) {
            task.set_state("new");
            _this.job_queue.unshift(task);
        };
        this.create_task = function (name) {
            var id = performance.now();
            var run_time = Math.round(Math.random() * 10000);
            _this.add_to_job_queue(new Task(id, name, run_time));
        };
        this.schedule_loop(20);
    }
    return OS;
}());
var os_instance = new OS();
os_instance.create_task("browser");
os_instance.create_task("player");
os_instance.create_task("editor");
os_instance.create_task("emulator");
os_instance.create_task("file explorer");
os_instance.create_task("terminal");
