# trainit_interfaces

TrainIt's ROS interfaces. **Messages only**, and only what no standard ROS type carries: the stamped,
tool-level state of an end effector (ADR-0022, vNext M2.6) and the grasp candidate any grasp producer
proposes to the runtime (ADR-0026, vNext M3.6), the canonical execution event of an application run
(ADR-0030, D0.5), and the two images of the Robot Task Interface a machine master (a PLC) exchanges with a
robot's task server (ADR-0032, P0.a), and the two images of a machine service a robot's runner exchanges with a
machine (P0.b).

| Type | What it is |
|---|---|
| `msg/EndEffectorState` | The state of ONE end-effector device, as its TrainIt adapter observes it: identity (`<robot_ns>/eef/<id>`), which adapter produced it, availability, the grasp observation per grasp site (with its evidence class and stamp, and the site's grip actuator), and one entry per actuator. Published only by the device's adapter, on the relative topic `eef/<id>/state` (RELIABLE, TRANSIENT_LOCAL, depth 1) |
| `msg/GraspCandidate` | ONE proposed way to grasp an object, from ANY producer (a projected detection, a surface-grasp method, a learned model, a vendor planner, a synthetic publisher): the frame and stamp, a unique `id` and the `provider`, the grasp site's TCP pose, the EXPLICIT unit `approach` (into the object), the suggested `standoff_m`, the `eef` and `site` (ADR-0022), `score` and `confidence`, optional object references, surface normal and producer `metadata`. Its orientation is the producer's only when it owns one (else the identity): the runtime takes the TCP orientation from the application (`align_approach`) |
| `msg/GraspCandidateArray` | The candidates of one producer for one observation, BEST FIRST (the order is the rank, never re-sorted), on `/perception/<producer>/grasp_candidates` (cell-global, RELIABLE, depth 1). An empty array means "no candidate" |
| `msg/ExecutionEvent` | ONE canonical event of an application run, published only by the runner (TMR) on the relative topic `trainit/execution_events` (RELIABLE, VOLATILE, keep-last 100: a late subscriber never receives an old event): the run id and a per-run sequence, the kind (run started / ended, task entered / exited, step entered / exited, process event), the exit status, the canonical task, step id and kind, the process channel / kind / value, and a grasp step's attempt index and candidate; since 0.4.0 (ADR-0030 Amendment P0, APPENDED) the Robot Task Interface `job_id` on every event of a job and the job's `result_code` on RUN_ENDED and the exported task's TASK_EXITED. Never a behavior-tree node identity, never a robot state |
| `msg/RobotTaskCommand` | The Robot Task Interface v1 COMMAND image (ADR-0032), requester -> the robot's task server, on the relative `trainit/robot_task/command`: requester session, heartbeat, command sequence number, command (START / CANCEL / RESET), job id, exported task number. Fixed-width integers only, level-held, PLC-shaped: a PLC adapter copies it 1:1 |
| `msg/RobotTaskStatus` | The Robot Task Interface v1 STATUS image (ADR-0032), the task server -> the requester, on `trainit/robot_task/status`, republished every period and on every change: interface version, export table id, server session, heartbeat, state (UNAVAILABLE / IDLE / RUNNING / STOPPING / FAULTED), ready, acknowledgement (sequence, accepted / rejected, code), the job, its result and result code, the fault code, and two ROS-only diagnosis strings. Every result / rejection code is a constant of the type: stable public values. Not a safety signal (ADR-0033) |
| `msg/ChannelRequest` | A machine-service REQUEST (P0.b), the robot's runner -> the machine, on the relative `<service>/request` (RELIABLE, TRANSIENT_LOCAL, depth 1, exactly one publisher): the source session it is FOR, the requester's session and its own sequence number. Its identity `(requester_session, seq)` is consumed exactly once by the source -- accepted as the cycle, or rejected with a reason; a request for another source session is never executed. Level-held, fixed-width integers: a PLC-side library copies it 1:1 |
| `msg/ChannelStatus` | A machine-service STATUS (P0.b), the machine -> the robot, on `<service>/status`, republished every period and on every change, exactly one publisher: the source session, a heartbeat minted by the source's own scan (the robot takes the source as live only while it CHANGES), the state (READY / BUSY / DONE / FAULT / HOMING), the identity of the request the current or last cycle serves and whether it completed, the last rejected request and why (BUSY / FAULTED / TWO_REQUESTERS), a fault code and a ROS-only diagnosis string. Not a safety signal (ADR-0033) |
| `msg/EndEffectorActuatorState` | One commandable part of the device (nested in the state; it has no topic of its own): its standard command interface, the joints it drives (keys into `sensor_msgs/JointState`, never copied), its status, its applied binary state and whether a switch confirms it, and its last applied command |

Commands stay on standard ROS types, one per actuator: `std_srvs/srv/SetBool` (binary),
`control_msgs/action/GripperCommand` (a single-joint gripper) and
`control_msgs/action/FollowJointTrajectory` (axes and coordinated mechanisms). Verification is a
policy (TMR's `VerifyGrasp` node), not a type.

A default-constructed message means "cannot tell": it never reads as ready, held or released.

Rules:
- every further type needs an ADR;
- the field lists are pinned by the tests (`test/test_end_effector_state.cpp`,
  `test/test_grasp_candidate.cpp`, `test/test_execution_event.cpp`, `test/test_robot_task_interface.cpp`,
  `test/test_machine_channel.cpp`), through rosidl's introspection; a field is only ever APPENDED, a code never
  renumbered (`CODE_STATION_BUSY` 313 was appended by P0.b);
- the end-effector constants are pinned by the canonical token file (`test/data/eef_contract_tokens.txt`, a
  byte copy shared with TMR, the hardware layer and TSA);
- Humble has no type hashes, so a field change ships in lockstep across the TrainIt packages.

License: Apache-2.0 (Community).
