# ONE commandable part of an end effector (ADR-0022), as the device's adapter observes it.
# Nested in EndEffectorState only; it has no topic of its own. NEVER a grasp observation.
# Joint values are NOT copied here: read sensor_msgs/JointState by the names in `joints`.

string actuator
# Local id, unique within the device; identity "<end_effector>/actuators/<actuator>".

uint8 INTERFACE_UNKNOWN=0
uint8 INTERFACE_SET_BOOL=1                 # std_srvs/srv/SetBool
uint8 INTERFACE_GRIPPER_COMMAND=2          # control_msgs/action/GripperCommand
uint8 INTERFACE_FOLLOW_JOINT_TRAJECTORY=3  # control_msgs/action/FollowJointTrajectory
uint8 command_interface
# The one standard interface this actuator is commanded through.

string[] joints
# Keys into sensor_msgs/JointState: the robot's joint_states, or eef/<eef_id>/joint_states for
# joints the adapter owns until they are merged. The joints this actuator drives, in contract
# order; never a mimic follower or a description pseudo-joint; empty for a joint-less actuator.
# For a SET_BOOL actuator their JointState is description-compatibility state, never evidence.

uint8 STATUS_UNKNOWN=0      # not observed yet, or its source went silent
uint8 STATUS_IDLE=1         # available, no command executing
uint8 STATUS_MOVING=2       # a command is executing (a goal active; a valve switching before its switch confirms)
uint8 STATUS_FAULT=3        # the device or its server reports a fault (EndEffectorState.detail says which)
uint8 STATUS_UNAVAILABLE=4  # its server, or the hardware link behind it, is not available
uint8 status

uint8 BINARY_UNKNOWN=0      # not a SET_BOOL actuator, or not observed yet
uint8 BINARY_RELEASED=1
uint8 BINARY_ENGAGED=2      # the ACTIVE state (vacuum on, locked, jaw closed) -- never "holding"
uint8 binary
# SET_BOOL only: the applied state.

bool binary_confirmed
# True only while a sensor ON the actuator (an end-of-stroke or pressure switch) reports `binary`;
# false means `binary` is the applied command, an echo. An actuation fact, never a grasp.

builtin_interfaces/Time stamp
# The newest observation of THIS actuator: a switch sample, an event of a goal the adapter serves,
# or, for an echo, the command receipt. Never re-stamped.

uint8 COMMAND_NONE=0
uint8 COMMAND_ENGAGE=1      # SetBool data=true, or a goal to the actuator's declared engage target
uint8 COMMAND_RELEASE=2     # SetBool data=false, or a goal to its declared release target
uint8 COMMAND_TARGET=3      # a goal to any other named target
uint8 last_command
# The newest APPLIED command (SetBool success:true, or an accepted goal) this adapter saw for this
# actuator, from ANY commander. A rejected command is not recorded here: EndEffectorState.detail
# names it, and its commander has the standard response.
builtin_interfaces/Time last_command_stamp   # when the adapter applied it; zero if and only if COMMAND_NONE
