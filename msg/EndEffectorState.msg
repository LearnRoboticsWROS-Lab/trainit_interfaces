# The TOOL-level state of ONE end-effector device (ADR-0022), as the device's TrainIt adapter
# observes it. Published ONLY by that adapter, on the relative topic eef/<eef_id>/state
# (RELIABLE, TRANSIENT_LOCAL, depth 1): whenever any field changes (a stamp advance included),
# plus a heartbeat that republishes an unchanged sample, so a dead source ages visibly.
# A default-constructed message means "cannot tell": it never reads as ready, held or released.
# A consumer reads any constant value it does not know as that field's UNKNOWN (or NONE).
# Referenced, never copied: joint position/velocity/effort (sensor_msgs/JointState, by the names
# in actuators[].joints), trajectory tracking (<controller>/controller_state) and the goal
# lifecycle (<action>/_action/status). Configuration (capabilities, targets, site frames) lives in
# the TSA contract, not here.

std_msgs/Header header
# stamp: the newest OBSERVATION in this sample, the maximum of site_stamp[] and actuators[].stamp
#   (a command receipt is not an observation). A heartbeat never advances it. Freshness is judged
#   per site, on site_stamp, never on this stamp.
# frame_id: unused, empty. A frame is never an identity.

string end_effector
# Fully-qualified identity "<robot namespace>/eef/<eef_id>", computed by the adapter from its own
# node namespace, never configured: "/r1/eef/composite_tool"; "/eef/main" at the root.
# An actuator is "<end_effector>/actuators/<id>"; a grasp site is "<end_effector>/sites/<id>".

string adapter
# The adapter implementation row that produced this sample, e.g. "trainit_mock".

uint8 STATUS_UNKNOWN=0      # an actuator is not observed yet, or its source went silent
uint8 STATUS_READY=1        # every actuator IDLE and no tool-level fault
uint8 STATUS_BUSY=2         # available; at least one actuator MOVING
uint8 STATUS_FAULT=3        # an actuator FAULT, or a tool-level fault (detail names it)
uint8 STATUS_UNAVAILABLE=4  # an actuator UNAVAILABLE, or the adapter lost its backend link
uint8 status
# Availability to ACT, aggregated FAULT > UNAVAILABLE > UNKNOWN > BUSY > READY over the actuators.
# Grasp sites never feed it.

# The grasp observation, ONE ENTRY PER GRASP SITE: equal-length arrays in contract order (the
# sensor_msgs/JointState idiom). A simple EEF has exactly one site, "main". Arrays of unequal length
# are a malformed sample, never "not held".
uint8 GRASP_UNAVAILABLE=0   # this adapter cannot observe a grasp at this site on this backend; never "not held"
uint8 GRASP_UNKNOWN=1       # it can, but has no valid observation now (no sample yet, or the source went silent)
uint8 GRASP_NOT_HELD=2
uint8 GRASP_PENDING=3       # the site's grip actuator was engaged; the deciding source has not decided yet
uint8 GRASP_HELD=4
uint8 GRASP_LOST=5          # HELD -> not held with no APPLIED release of the site's grip actuator since,
                            # from any commander; latched until its next applied engage or release
uint8 EVIDENCE_NONE=0                 # if and only if the grasp is UNAVAILABLE or UNKNOWN
uint8 EVIDENCE_COMMAND_ECHO=1         # the command echoed back. Never published by a TrainIt adapter; never confirms
uint8 EVIDENCE_ACTUATION_INFERENCE=2  # inferred from an actuator (a jaw's stall or position band): opt-in only
uint8 EVIDENCE_SENSOR=3               # a physical sensor: vacuum or pressure switch, presence, contact, force
uint8 EVIDENCE_SIM_GROUND_TRUTH=4     # the simulator's own judgement, conditional on geometry
uint8 EVIDENCE_SIM_ATTACHMENT=5       # a constraint the simulator makes on command, whatever the geometry.
                                      # Never published by a TrainIt adapter; never confirms
string[] sites                        # local site ids, unique within the device
string[] site_grip                    # per site: the local id of the actuator that grips it
uint8[] site_grasp                    # GRASP_*, per site
uint8[] site_evidence                 # EVIDENCE_*, per site: the provenance of site_grasp (for PENDING
                                      # and LOST, the class of the site's deciding source)
builtin_interfaces/Time[] site_stamp  # per site: the newest observation site_grasp rests on; never re-stamped

EndEffectorActuatorState[] actuators
# Every DECLARED actuator, in contract order; one not observed yet is listed with status UNKNOWN,
# never omitted.

string detail
# Human-readable cause, naming each actuator or site concerned ("gripper_b: no stage subscriber"),
# rejected commands included ("gripper_b: ENGAGE rejected: <message>"); empty when healthy.
