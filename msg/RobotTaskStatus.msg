# The Robot Task Interface v1 STATUS image (ADR-0032 Decision 4): the robot's RobotTaskServer -> the requester.
# Level-based: the current truth, republished every status period (100 ms default) AND at once on every change of
# state, ack or result, so an edge is never lost. Every wire field is a fixed-width integer or a bool; the two strings
# at the end are ROS-only diagnosis, never parsed. Relative topic `trainit/robot_task/status` in the robot's slot,
# RELIABLE, TRANSIENT_LOCAL, depth 1. NOT a safety signal (ADR-0033, invariant 13).

std_msgs/Header header
# stamp: the server's node clock (ROS time, or sim time under use_sim_time). The RTI's own liveness never uses it:
# the heartbeat period and every RTI timeout run on the steady clock (ADR-0032 Decision 9).

uint16 interface_version
uint16 INTERFACE_VERSION = 1
# The RTI version; a requester generated for another version refuses to command.

uint32 export_table_id
# CRC-32 (IEEE) of the exported-task table the server serves (ADR-0032 Decision 3). A requester generated for another
# table refuses to command.

uint32 server_session
# Changes at every server start, never 0. A change while a requester had a job in flight = JOB LOST.

uint16 heartbeat
# Incremented every status period, also when nothing changes and while a motion blocks the job's tick.

uint8 state
uint8 STATE_UNAVAILABLE = 0
uint8 STATE_IDLE = 1
uint8 STATE_RUNNING = 2
uint8 STATE_STOPPING = 3
uint8 STATE_FAULTED = 4

bool ready
# == (state == STATE_IDLE).

uint32 ack_seq
# The last cmd_seq processed (accepted or rejected).
uint8 ack
uint8 ACK_NONE = 0
uint8 ACK_ACCEPTED = 1
uint8 ACK_REJECTED = 2
uint16 ack_code
# A rejection's reason (0 otherwise):
uint16 REJECT_INVALID_REQUEST = 400
uint16 REJECT_UNKNOWN_TASK = 401
uint16 REJECT_BUSY = 402
uint16 REJECT_NOT_READY = 403
uint16 REJECT_FAULTED = 404
uint16 REJECT_DUPLICATE_JOB = 405
uint16 REJECT_NOT_CURRENT_JOB = 406
uint16 REJECT_INVALID_STATE = 407
uint16 REJECT_SEQUENCE_ERROR = 408
uint16 REJECT_NOT_SYNCHRONIZED = 409

uint32 job_id
# The current job (RUNNING / STOPPING), else the last finished one; 0 = none.
uint16 task
# That job's exported task number.

uint8 result
uint8 RESULT_NONE = 0
uint8 RESULT_SUCCEEDED = 1
uint8 RESULT_ENDED = 2
uint8 RESULT_CANCELLED = 3
uint8 RESULT_FAILED = 4
uint16 result_code
# That result's canonical code (ADR-0032 Decision 7; stable public values, never renumbered):
uint16 CODE_NONE = 0
uint16 CODE_OK = 100
uint16 CODE_BIN_EMPTY = 101
uint16 CODE_NOTHING_PICKABLE = 102
uint16 CODE_PICKABLE_EXHAUSTED = 110
uint16 CODE_RECOVERY_EXHAUSTED = 111
uint16 CODE_CANCELLED = 200
uint16 CODE_TERMINAL_FAULT = 300
uint16 CODE_TASK_FAILED = 301
uint16 CODE_TASK_TIMEOUT = 302
uint16 CODE_REQUESTER_LOST = 303
uint16 CODE_RUNTIME_LOST = 304
uint16 CODE_STATION_TIMEOUT = 305
uint16 CODE_STATION_FAULT = 306
uint16 CODE_CANCELLED_WHILE_HOLDING = 307
uint16 CODE_EEF_FAULT = 308
uint16 CODE_CANCELLED_HOLD_UNKNOWN = 309
uint16 CODE_STATION_UNAVAILABLE = 310
uint16 CODE_SERVER_STARTED = 311
uint16 CODE_REQUESTER_CONFLICT = 312
uint16 CODE_STATION_BUSY = 313
# 313 APPENDED by P0.b (trainit_interfaces 0.5.0; ADR-0032 Decision 7 note): a job asked a machine service that was
# still busy or homing, or had not yet taken this process's previous request. A requester treats a code it does not
# know as a generic FAILED.

uint16 fault_code
# While FAULTED: why (a FAILED result_code, or CODE_SERVER_STARTED after every server start); 0 otherwise.

string task_name
# ROS-only: the job's canonical task name. Diagnosis, never parsed.
string detail
# ROS-only: the operator text, the failing step, a later stop cause. Diagnosis, never parsed.
