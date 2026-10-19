# The grasp candidates ONE producer proposes for one observation (ADR-0026), BEST FIRST: the order is
# the rank (ADR-0016 Decision 1) and a consumer never re-orders it. Published on the cell-global topic
# /perception/<producer>/grasp_candidates (RELIABLE, depth 1; ADR-0023 Decision 4, like detections).
#
# An EMPTY array is a valid answer -- "no candidate" -- and never malformed data.

std_msgs/Header header
# stamp: the observation the candidates rest on. frame_id: informative; each candidate names its own
# frame in its header.

GraspCandidate[] candidates
# best first
