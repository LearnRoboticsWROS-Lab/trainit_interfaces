# ONE canonical event of a TrainIt application run (ADR-0030; this package owns the topic name below): a run, a task
# or a step of the application
# began or ended, or a process event was dispatched. Published ONLY by the runner that executes the
# application (TMR), on the RELATIVE topic `trainit/execution_events` in its robot slot (ADR-0023), RELIABLE,
# VOLATILE, keep-last 100: a subscriber that joins later never receives an event that happened before it
# joined. It is a statement of what the application DID, in the application's own vocabulary: never a
# behavior-tree node name, class or position, never a topic string, never a robot state.

std_msgs/Header header
# stamp: when the runner observed the transition, on the runner node's clock (ROS time, or the simulation
# clock under use_sim_time). frame_id: empty.

string run_id
# One per runner start ("run-<UTC>-<8 hex>"), never empty: a restarted runner is a new run.

uint64 seq
# Per run, from 1, +1 per event: the publication order. Events sharing a stamp are ordered by seq.

uint8 kind
uint8 RUN_STARTED = 1
uint8 RUN_ENDED = 2
uint8 TASK_ENTERED = 3
uint8 TASK_EXITED = 4
uint8 STEP_ENTERED = 5
uint8 STEP_EXITED = 6
uint8 PROCESS_EVENT = 7
# RUN_*: the run, before its first tick / after its last. TASK_*: a canonical task (its tree), on every
# entry (a called or repeated task is entered again). STEP_*: a canonical step that carries an id (D18); a
# step without an id produces no event. PROCESS_EVENT: a process event dispatched to the process controller.

uint8 status
uint8 STATUS_NONE = 0
uint8 SUCCEEDED = 1
uint8 FAILED = 2
uint8 HALTED = 3
# RUN_ENDED, TASK_EXITED and STEP_EXITED only: how it ended (HALTED: stopped by its parent, never finished).
# STATUS_NONE on every other kind.

string task_id
# The canonical task (the application's task name); for STEP_* and PROCESS_EVENT the task the step runs in.
# Empty on RUN_* and on a process event outside any task.

string step_id
string step_kind
# STEP_*: the canonical step id (user-chosen, unique within its task) and kind (move, path, process, dwell,
# detect, grasp, release, tool, task, repeat, retry, fallback). PROCESS_EVENT: the step it was dispatched in, when
# the innermost active scope is an identified step (a called task's own scope never reports its caller's step).
# Empty otherwise.

string process_channel
string process_kind
float64 process_value
# PROCESS_EVENT only. The LOGICAL channel id (ADR-0013: never a topic, an address or a symbol); the kind
# (process_on | process_off | set_value | external); the value: 1 for process_on, 0 for process_off, the
# value of set_value, 0 for external (it carries none). Elsewhere: empty, empty, 0 (meaningless).

uint32 attempt_index
string candidate_id
string candidate_provider
# A grasp step's STEP_* events only: the 1-based count of this step's entries in this run (an attempt), and
# the grasp candidate it takes (ADR-0026 `id` / `provider`) when it grasps from a candidate set and the
# candidate is known at that moment. 0 / empty otherwise. Whether an attempt succeeded is NOT stated here.
