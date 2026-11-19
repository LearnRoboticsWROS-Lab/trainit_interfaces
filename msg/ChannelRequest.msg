# A machine-service REQUEST: the robot's runner -> the machine (the cell's source of the service). P0.b; the
# contract is ADR-0024 Amendment P0 as REPLACED by the P0.b pre-registration (P0B_DESIGN.md §3). Relative topic
# `<service>/request` in the robot's slot (ADR-0023), RELIABLE, TRANSIENT_LOCAL, depth 1, exactly ONE publisher: the
# robot's runner process (trainit_task_server or trainit_run_bt). Level-held: the image stays until the next request.
# NOT a safety signal (ADR-0033, invariant 13).
#
# A request has an IDENTITY, (requester_session, seq). The source CONSUMES every identity exactly once -- ACCEPTED as
# its new cycle, or REJECTED with a reason (ChannelStatus) -- so a re-delivered image is a no-op. Matching is by
# identity EQUALITY, never by ordering. A request naming another source_session (an image left from before a source
# restart) is never executed.

std_msgs/Header header
# stamp: the runner node's clock (ROS time, or the simulation clock under use_sim_time). Diagnosis only.

uint32 source_session
# The source session this request is FOR (ChannelStatus.source_session as the robot saw it). Never 0.

uint32 requester_session
# Random, non-zero, new at every start of the requesting process.

uint32 seq
# The requester's own counter for this source session: 1 .. 2^32-1, wrapping to 1 (0 never).
