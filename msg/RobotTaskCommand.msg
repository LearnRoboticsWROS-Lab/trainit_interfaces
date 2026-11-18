# The Robot Task Interface v1 COMMAND image (ADR-0032 Decision 4): requester (the cell's ONE machine master, a PLC or
# the fake PLC) -> the robot's RobotTaskServer. A PLC-shaped cyclic image: every field a fixed-width integer, so an
# OPC UA / Modbus TCP / fieldbus adapter copies it 1:1 with no protocol logic. LEVEL-HELD: the requester writes cmd, job_id
# and task together with a NEW cmd_seq and republishes the image every requester period (<= 100 ms) until its next
# command; the server acts on a CHANGE of cmd_seq only (a re-delivered image is a no-op). Relative topic
# `trainit/robot_task/command` in the robot's slot (ADR-0023 Amendment P0), RELIABLE, TRANSIENT_LOCAL, depth 1.
# NOT a safety signal (ADR-0033, invariant 13).

std_msgs/Header header
# stamp: when the requester (or its adapter) wrote this image, on its clock. frame_id: empty.

uint32 requester_session
# Changes at every requester start; 0 = invalid, never adopted. A new session is adopted after its heartbeat has
# changed twice (ADR-0032 Decision 6, rule 1).

uint16 heartbeat
# Incremented every requester period (wrapping). The server declares the requester lost after 1.0 s (default)
# without a change.

uint32 cmd_seq
# The sequence number of the command in this image: the previous + 1, wrapping from 4294967295 to 1 (0 skipped);
# 0 = no command yet in this session. Never changes without a command.

uint8 cmd
uint8 CMD_NONE = 0
uint8 CMD_START = 1
uint8 CMD_CANCEL = 2
uint8 CMD_RESET = 3

uint32 job_id
# START: the new job's id, chosen by the requester, != 0 (a RETAIN, monotonic counter on a PLC). CANCEL: the job to
# cancel. RESET: ignored.

uint16 task
# START: the exported task's NUMBER (1..65535, `export.number` in the application). Ignored otherwise.
