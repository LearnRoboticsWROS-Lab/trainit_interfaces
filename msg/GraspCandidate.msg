# ONE proposed way to grasp an object (ADR-0026): a TOOL placement with an EXPLICIT approach, from
# ANY grasp producer -- a geometric projection of a detection, a surface-grasp method, a model-based
# pose estimator plus grasp logic, a learned grasp model, a vendor planner, a synthetic test publisher.
# There is ONE candidate type: what is specific to a producer goes into `provider` and `metadata`,
# never into a field of its own. The runtime never asks how a candidate was made.
#
# A candidate is NOT a detection (an observation: "something was seen here"), NOT a surface patch and
# NOT an object pose: it proposes how the tool reaches and holds the object. It is NOT a motion either:
# the PreGrasp, Approach and Retract, their motion types and profiles belong to the application's grasp
# step, which derives them from this message (PreGrasp = grasp_pose.position - standoff * approach).
#
# A consumer refuses a candidate it cannot use (a non-finite value, a quaternion or an approach whose
# norm is not 1 within 1e-3, a confidence outside 0..1, an unresolvable frame) and never repairs one
# beyond that tolerance.

std_msgs/Header header
# frame_id: the frame of grasp_pose, approach and surface_normal (required).
# stamp: the observation this candidate rests on.

string id
# Unique within its GraspCandidateArray, never empty: the identity a retry and a future outcome
# record (D0) key on, together with `provider`.

string provider
# The producer's row id, never empty, e.g. "<package>.<method>", "policy.<card>", "synthetic".

geometry_msgs/Pose grasp_pose
# The grasp SITE's TCP pose at grasp, in header.frame_id. Its position is where the site's TCP holds
# the object. Its orientation is the producer's full TCP orientation when the producer owns one,
# else the identity: a producer that proposes only a point and an approach (a projected detection)
# never invents the rotation about the approach. In TrainIt vNext M3.6 the runtime takes the TCP
# orientation from the application (`orientation: align_approach`, ADR-0026): the residual rotation
# about the approach is the application's, never the producer's.

geometry_msgs/Vector3 approach
# The UNIT direction the tool travels to reach grasp_pose, in header.frame_id: it points INTO the
# object (for a surface whose normal faces the camera, approach = -normal). Explicit, never inferred
# from an axis of grasp_pose.

float64 standoff_m
# The producer's suggested pre-grasp distance along -approach (metres, >= 0); 0 = none suggested. The
# application's grasp step may override it.

string eef
# The end-effector device id (ADR-0022) the candidate is for; empty = the application's default.

string site
# The grasp site id (ADR-0022) of that device; empty = the device's only site. The site names the TCP.

float64 score
# The producer's rank value. The ARRAY ORDER is the rank (ADR-0016 Decision 1): a consumer never sorts.

float64 confidence
# The producer's confidence, 0..1, distinct from the rank; 0 = the producer states none.

string object_class
# Optional: the class of the object to grasp.

string object_id
# Optional: the id of the object instance.

geometry_msgs/Vector3 surface_normal
# Optional: the measured surface normal the candidate rests on, in header.frame_id; zero = none.

diagnostic_msgs/KeyValue[] metadata
# Optional producer-specific terms (key/value): carried, never interpreted by the runtime.
