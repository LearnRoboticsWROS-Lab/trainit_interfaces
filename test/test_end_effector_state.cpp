// ADR-0022 D3: the two messages are exactly the ADR's. Three things are checked:
//  1. the field list, in order, through rosidl's own introspection (no field added or dropped);
//  2. every constant, against the pinned canonical token file (a byte copy of the one TMR, the
//     hardware layer and TSA check their own tables against);
//  3. a default-constructed message means "cannot tell": it never reads as ready, held or released.
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>
#include <rosidl_typesupport_introspection_cpp/message_introspection.hpp>
#include <rosidl_typesupport_introspection_cpp/message_type_support_decl.hpp>

#include "trainit_interfaces/msg/end_effector_actuator_state.hpp"
#include "trainit_interfaces/msg/end_effector_state.hpp"

namespace
{
using trainit_interfaces::msg::EndEffectorActuatorState;
using trainit_interfaces::msg::EndEffectorState;

// The pinned copy of the canonical token file. A local edit of the copy is caught here, before
// any table is compared with it.
constexpr const char* kTokensSha256 = "55048ea44d466979f57c28a634295a060a79b837fe614a30c49f60572a2bd5de";

template <typename MsgT>
std::vector<std::string> fieldNames()
{
  const auto* ts =
    rosidl_typesupport_introspection_cpp::get_message_type_support_handle<MsgT>();
  const auto* members =
    static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers*>(ts->data);
  std::vector<std::string> names;
  for (uint32_t i = 0; i < members->member_count_; ++i) names.emplace_back(members->members_[i].name_);
  return names;
}

// group -> name -> value, from "<group> <name> <value>" lines ('#' starts a comment line).
std::map<std::string, std::map<std::string, std::string>> loadTokens()
{
  std::ifstream in(TRAINIT_EEF_TOKENS_FILE);
  EXPECT_TRUE(in.is_open()) << TRAINIT_EEF_TOKENS_FILE;
  std::map<std::string, std::map<std::string, std::string>> t;
  std::string line;
  while (std::getline(in, line))
  {
    if (line.empty() || line[0] == '#') continue;
    std::istringstream ls(line);
    std::string g, n, v;
    ls >> g >> n >> v;
    t[g][n] = v;
  }
  return t;
}

std::string upper(std::string s)
{
  for (auto& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  return s;
}

// The message's constants of one group, by their token name (lower case, prefix dropped).
using Constants = std::map<std::string, int>;

Constants toolStatus()
{
  return {{"unknown", EndEffectorState::STATUS_UNKNOWN}, {"ready", EndEffectorState::STATUS_READY},
          {"busy", EndEffectorState::STATUS_BUSY}, {"fault", EndEffectorState::STATUS_FAULT},
          {"unavailable", EndEffectorState::STATUS_UNAVAILABLE}};
}
Constants grasp()
{
  return {{"unavailable", EndEffectorState::GRASP_UNAVAILABLE}, {"unknown", EndEffectorState::GRASP_UNKNOWN},
          {"not_held", EndEffectorState::GRASP_NOT_HELD}, {"pending", EndEffectorState::GRASP_PENDING},
          {"held", EndEffectorState::GRASP_HELD}, {"lost", EndEffectorState::GRASP_LOST}};
}
Constants evidence()
{
  return {{"none", EndEffectorState::EVIDENCE_NONE},
          {"command_echo", EndEffectorState::EVIDENCE_COMMAND_ECHO},
          {"actuation_inference", EndEffectorState::EVIDENCE_ACTUATION_INFERENCE},
          {"sensor", EndEffectorState::EVIDENCE_SENSOR},
          {"sim_ground_truth", EndEffectorState::EVIDENCE_SIM_GROUND_TRUTH},
          {"sim_attachment", EndEffectorState::EVIDENCE_SIM_ATTACHMENT}};
}
Constants commandInterface()
{
  return {{"unknown", EndEffectorActuatorState::INTERFACE_UNKNOWN},
          {"set_bool", EndEffectorActuatorState::INTERFACE_SET_BOOL},
          {"gripper_command", EndEffectorActuatorState::INTERFACE_GRIPPER_COMMAND},
          {"follow_joint_trajectory", EndEffectorActuatorState::INTERFACE_FOLLOW_JOINT_TRAJECTORY}};
}
Constants actuatorStatus()
{
  return {{"unknown", EndEffectorActuatorState::STATUS_UNKNOWN}, {"idle", EndEffectorActuatorState::STATUS_IDLE},
          {"moving", EndEffectorActuatorState::STATUS_MOVING}, {"fault", EndEffectorActuatorState::STATUS_FAULT},
          {"unavailable", EndEffectorActuatorState::STATUS_UNAVAILABLE}};
}
Constants binary()
{
  return {{"unknown", EndEffectorActuatorState::BINARY_UNKNOWN},
          {"released", EndEffectorActuatorState::BINARY_RELEASED},
          {"engaged", EndEffectorActuatorState::BINARY_ENGAGED}};
}
Constants command()
{
  return {{"none", EndEffectorActuatorState::COMMAND_NONE}, {"engage", EndEffectorActuatorState::COMMAND_ENGAGE},
          {"release", EndEffectorActuatorState::COMMAND_RELEASE},
          {"target", EndEffectorActuatorState::COMMAND_TARGET}};
}

void expectGroup(const std::map<std::string, std::map<std::string, std::string>>& tokens,
                 const std::string& group, const Constants& constants)
{
  ASSERT_TRUE(tokens.count(group)) << "the token file has no group " << group;
  const auto& want = tokens.at(group);
  EXPECT_EQ(want.size(), constants.size()) << group << ": the message and the token file differ in size";
  for (const auto& [name, value] : want)
  {
    ASSERT_TRUE(constants.count(name)) << group << ": the message has no constant for token " << name;
    EXPECT_EQ(std::stoi(value), constants.at(name)) << group << "." << name;
  }
}
}  // namespace

TEST(EndEffectorState, TheTokenFileIsThePinnedCopy)
{
  EXPECT_EQ(std::string(TRAINIT_EEF_TOKENS_SHA256), kTokensSha256)
    << "test/data/eef_contract_tokens.txt is not the pinned canonical copy (ADR-0022)";
}

TEST(EndEffectorState, FieldsAreExactlyTheAdrs)
{
  EXPECT_EQ(fieldNames<EndEffectorState>(),
            (std::vector<std::string>{"header", "end_effector", "adapter", "status", "sites", "site_grip",
                                      "site_grasp", "site_evidence", "site_stamp", "actuators", "detail"}));
  EXPECT_EQ(fieldNames<EndEffectorActuatorState>(),
            (std::vector<std::string>{"actuator", "command_interface", "joints", "status", "binary",
                                      "binary_confirmed", "stamp", "last_command", "last_command_stamp"}));
}

TEST(EndEffectorState, ConstantsMatchTheCanonicalTokens)
{
  const auto tokens = loadTokens();
  expectGroup(tokens, "tool_status", toolStatus());
  expectGroup(tokens, "grasp", grasp());
  expectGroup(tokens, "evidence", evidence());
  expectGroup(tokens, "command_interface", commandInterface());
  expectGroup(tokens, "actuator_status", actuatorStatus());
  expectGroup(tokens, "binary", binary());
  expectGroup(tokens, "command", command());
}

TEST(EndEffectorState, ADefaultToolStateMeansCannotTell)
{
  const EndEffectorState s;
  EXPECT_TRUE(s.end_effector.empty());  // so VerifyGrasp's identity check fails: WRONG_EEF
  EXPECT_TRUE(s.adapter.empty());
  EXPECT_EQ(s.status, EndEffectorState::STATUS_UNKNOWN);
  EXPECT_TRUE(s.sites.empty() && s.site_grip.empty() && s.site_grasp.empty() && s.site_evidence.empty() &&
              s.site_stamp.empty());
  EXPECT_TRUE(s.actuators.empty());
  EXPECT_TRUE(s.detail.empty());
  EXPECT_EQ(s.header.stamp.sec, 0);
  EXPECT_EQ(s.header.stamp.nanosec, 0u);
  EXPECT_TRUE(s.header.frame_id.empty());
}

TEST(EndEffectorState, ADefaultActuatorStateMeansCannotTell)
{
  const EndEffectorActuatorState a;
  EXPECT_TRUE(a.actuator.empty());
  EXPECT_EQ(a.command_interface, EndEffectorActuatorState::INTERFACE_UNKNOWN);
  EXPECT_TRUE(a.joints.empty());
  EXPECT_EQ(a.status, EndEffectorActuatorState::STATUS_UNKNOWN);
  EXPECT_EQ(a.binary, EndEffectorActuatorState::BINARY_UNKNOWN);
  EXPECT_FALSE(a.binary_confirmed);  // false means `binary` is an echo, never a confirmation
  EXPECT_EQ(a.last_command, EndEffectorActuatorState::COMMAND_NONE);
  EXPECT_EQ(a.last_command_stamp.sec, 0);
  EXPECT_EQ(a.stamp.sec, 0);
}

TEST(EndEffectorState, TheUnavailableAndUnknownGraspsAreNotNotHeld)
{
  // "cannot tell" is never "not held": 0 and 1 are not NOT_HELD, and evidence NONE is 0.
  EXPECT_NE(EndEffectorState::GRASP_UNAVAILABLE, EndEffectorState::GRASP_NOT_HELD);
  EXPECT_NE(EndEffectorState::GRASP_UNKNOWN, EndEffectorState::GRASP_NOT_HELD);
  EXPECT_EQ(EndEffectorState::GRASP_UNAVAILABLE, 0);
  EXPECT_EQ(EndEffectorState::EVIDENCE_NONE, 0);
}
