# trainit_interfaces

TrainIt's ROS interfaces (ADR-0022, vNext M2.6). **Messages only**, and only what no standard ROS
type carries: the stamped, tool-level state of an end effector.

| Type | What it is |
|---|---|
| `msg/EndEffectorState` | The state of ONE end-effector device, as its TrainIt adapter observes it: identity (`<robot_ns>/eef/<id>`), which adapter produced it, availability, the grasp observation per grasp site (with its evidence class and stamp, and the site's grip actuator), and one entry per actuator. Published only by the device's adapter, on the relative topic `eef/<id>/state` (RELIABLE, TRANSIENT_LOCAL, depth 1) |
| `msg/EndEffectorActuatorState` | One commandable part of the device (nested in the state; it has no topic of its own): its standard command interface, the joints it drives (keys into `sensor_msgs/JointState`, never copied), its status, its applied binary state and whether a switch confirms it, and its last applied command |

Commands stay on standard ROS types, one per actuator: `std_srvs/srv/SetBool` (binary),
`control_msgs/action/GripperCommand` (a single-joint gripper) and
`control_msgs/action/FollowJointTrajectory` (axes and coordinated mechanisms). Verification is a
policy (TMR's `VerifyGrasp` node), not a type.

A default-constructed message means "cannot tell": it never reads as ready, held or released.

Rules:
- every further type needs an ADR;
- the constants are pinned by the canonical token file (`test/data/eef_contract_tokens.txt`, a
  byte copy shared with TMR, the hardware layer and TSA);
- Humble has no type hashes, so a field change ships in lockstep across the TrainIt packages.

License: Apache-2.0 (Community).
