# A machine-service STATUS: the machine (the cell's source of the service) -> the robot. P0.b; the contract is
# ADR-0024 Amendment P0 as REPLACED by the P0.b pre-registration (P0B_DESIGN.md §3). Relative topic
# `<service>/status` in the robot's slot (ADR-0023), RELIABLE, TRANSIENT_LOCAL, depth 1, EXACTLY ONE publisher,
# republished every status period (100 ms default) AND at once on every change. Level-based: the current truth.
# NOT a safety signal (ADR-0033, invariant 13).
#
# The robot takes the source as LIVE only while `heartbeat` CHANGES (within its max age, on the robot's steady clock)
# and exactly one publisher exists: a source that republishes a frozen image is not live.

std_msgs/Header header
# stamp: the source's clock (ROS time, or the simulation clock). Diagnosis only: liveness never uses it.

uint32 source_session
# Random, non-zero, new at every source start. 0 = no source (a default image says nothing).

uint16 heartbeat
# +1 every status period, minted by the source's own scan (in a simulator: inside the simulator, never by a
# bridge), also when nothing changes.

uint8 state
uint8 STATE_NONE = 0
uint8 STATE_READY = 1
uint8 STATE_BUSY = 2
uint8 STATE_DONE = 3
uint8 STATE_FAULT = 4
uint8 STATE_HOMING = 5
# READY: idle, accepting a request. BUSY: executing the cycle of `cycle_*`. DONE: that cycle completed (level: it
# stays DONE until the next request is accepted). FAULT: a device fault (`fault_code`); it persists until a RESET by
# the machine master / the operator (never the robot). HOMING: after a RESET, returning to home before READY.
# STATE_NONE only in a default image.

uint32 cycle_requester
uint32 cycle_seq
# The identity of the request the current / last cycle serves (0, 0: no cycle yet in this session).
bool cycle_done
# That cycle COMPLETED (false while it runs, and when it was aborted by a fault).

uint32 reject_requester
uint32 reject_seq
uint8 reject_code
uint8 REJECT_NONE = 0
uint8 REJECT_BUSY = 1
uint8 REJECT_FAULTED = 2
uint8 REJECT_TWO_REQUESTERS = 3
# The last REJECTED request (0, 0, REJECT_NONE: none in this session) and why: the source was BUSY (or HOMING), in
# FAULT, or more than one requester publishes on `<service>/request`. A rejected request is consumed: never executed.

uint16 fault_code
uint16 FAULT_NONE = 0
uint16 FAULT_ACTUATOR_TIMEOUT = 1
uint16 FAULT_SENSOR_FAULT = 2
uint16 FAULT_STATION_FAULT = 3
# While FAULT: why (an actuator that did not reach its limit in time, a sensor fault, any other station-reported
# fault); FAULT_NONE otherwise.

string detail
# Diagnosis only (which device, which limit), never a contract value, never parsed.
