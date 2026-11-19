// ADR-0032: the Robot Task Interface v1 images are exactly the ADR's. Pinned through rosidl's introspection:
//  1. the fields of both images, in order, with their types: a PLC adapter copies them 1:1 (Decision 4), so the
//     layout is a public wire contract -- fields may only be APPENDED;
//  2. the closed vocabularies (cmd, state, ack, result) and every result / rejection code: stable public values,
//     never renumbered or reused (Decision 7);
//  3. a default image says "nothing": no command, UNAVAILABLE, no job, no result -- never a disguised IDLE / START;
//  4. the package version that ships them (0.4.0 or later).
#include <cstdio>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <rosidl_typesupport_introspection_cpp/field_types.hpp>
#include <rosidl_typesupport_introspection_cpp/message_introspection.hpp>
#include <rosidl_typesupport_introspection_cpp/message_type_support_decl.hpp>

#include "trainit_interfaces/msg/robot_task_command.hpp"
#include "trainit_interfaces/msg/robot_task_status.hpp"

namespace
{
using trainit_interfaces::msg::RobotTaskCommand;
using trainit_interfaces::msg::RobotTaskStatus;
using rosidl_typesupport_introspection_cpp::MessageMembers;

template <typename MsgT>
const MessageMembers* membersOf()
{
  const auto* ts = rosidl_typesupport_introspection_cpp::get_message_type_support_handle<MsgT>();
  return static_cast<const MessageMembers*>(ts->data);
}

std::vector<std::string> fieldsOf(const MessageMembers* m)
{
  namespace ft = rosidl_typesupport_introspection_cpp;
  std::vector<std::string> out;
  for (uint32_t i = 0; i < m->member_count_; ++i)
  {
    const auto& f = m->members_[i];
    std::string type;
    switch (f.type_id_)
    {
      case ft::ROS_TYPE_STRING: type = "string"; break;
      case ft::ROS_TYPE_BOOLEAN: type = "bool"; break;
      case ft::ROS_TYPE_UINT8: type = "uint8"; break;
      case ft::ROS_TYPE_UINT16: type = "uint16"; break;
      case ft::ROS_TYPE_UINT32: type = "uint32"; break;
      case ft::ROS_TYPE_MESSAGE:
      {
        const auto* sub = static_cast<const MessageMembers*>(f.members_->data);
        std::string ns = sub->message_namespace_;
        ns = ns.substr(0, ns.find("::"));
        type = ns + "/" + sub->message_name_;
        break;
      }
      default: type = "type_id_" + std::to_string(f.type_id_);
    }
    if (f.is_array_) type += "[]";
    out.push_back(std::string(f.name_) + ":" + type);
  }
  return out;
}

std::string readFile(const char* path)
{
  std::ifstream in(path);
  EXPECT_TRUE(in.is_open()) << path;
  std::stringstream s;
  s << in.rdbuf();
  return s.str();
}
}  // namespace

TEST(RobotTaskInterface, TheCommandImageIsExactlyTheAdrsInOrder)
{
  EXPECT_EQ(fieldsOf(membersOf<RobotTaskCommand>()),
            (std::vector<std::string>{"header:std_msgs/Header", "requester_session:uint32", "heartbeat:uint16",
                                      "cmd_seq:uint32", "cmd:uint8", "job_id:uint32", "task:uint16"}));
}

TEST(RobotTaskInterface, TheStatusImageIsExactlyTheAdrsInOrder)
{
  EXPECT_EQ(fieldsOf(membersOf<RobotTaskStatus>()),
            (std::vector<std::string>{
              "header:std_msgs/Header", "interface_version:uint16", "export_table_id:uint32",
              "server_session:uint32", "heartbeat:uint16", "state:uint8", "ready:bool", "ack_seq:uint32", "ack:uint8",
              "ack_code:uint16", "job_id:uint32", "task:uint16", "result:uint8", "result_code:uint16",
              "fault_code:uint16", "task_name:string", "detail:string"}));
}

TEST(RobotTaskInterface, TheClosedVocabulariesAreTheAdrs)
{
  EXPECT_EQ(RobotTaskCommand::CMD_NONE, 0);
  EXPECT_EQ(RobotTaskCommand::CMD_START, 1);
  EXPECT_EQ(RobotTaskCommand::CMD_CANCEL, 2);
  EXPECT_EQ(RobotTaskCommand::CMD_RESET, 3);
  EXPECT_EQ(RobotTaskStatus::INTERFACE_VERSION, 1);
  EXPECT_EQ(RobotTaskStatus::STATE_UNAVAILABLE, 0);
  EXPECT_EQ(RobotTaskStatus::STATE_IDLE, 1);
  EXPECT_EQ(RobotTaskStatus::STATE_RUNNING, 2);
  EXPECT_EQ(RobotTaskStatus::STATE_STOPPING, 3);
  EXPECT_EQ(RobotTaskStatus::STATE_FAULTED, 4);
  EXPECT_EQ(RobotTaskStatus::ACK_NONE, 0);
  EXPECT_EQ(RobotTaskStatus::ACK_ACCEPTED, 1);
  EXPECT_EQ(RobotTaskStatus::ACK_REJECTED, 2);
  EXPECT_EQ(RobotTaskStatus::RESULT_NONE, 0);
  EXPECT_EQ(RobotTaskStatus::RESULT_SUCCEEDED, 1);
  EXPECT_EQ(RobotTaskStatus::RESULT_ENDED, 2);
  EXPECT_EQ(RobotTaskStatus::RESULT_CANCELLED, 3);
  EXPECT_EQ(RobotTaskStatus::RESULT_FAILED, 4);
}

TEST(RobotTaskInterface, TheCodesAreTheAdrsStablePublicValues)
{
  EXPECT_EQ(RobotTaskStatus::CODE_NONE, 0);
  EXPECT_EQ(RobotTaskStatus::CODE_OK, 100);
  EXPECT_EQ(RobotTaskStatus::CODE_BIN_EMPTY, 101);
  EXPECT_EQ(RobotTaskStatus::CODE_NOTHING_PICKABLE, 102);
  EXPECT_EQ(RobotTaskStatus::CODE_PICKABLE_EXHAUSTED, 110);
  EXPECT_EQ(RobotTaskStatus::CODE_RECOVERY_EXHAUSTED, 111);
  EXPECT_EQ(RobotTaskStatus::CODE_CANCELLED, 200);
  EXPECT_EQ(RobotTaskStatus::CODE_TERMINAL_FAULT, 300);
  EXPECT_EQ(RobotTaskStatus::CODE_TASK_FAILED, 301);
  EXPECT_EQ(RobotTaskStatus::CODE_TASK_TIMEOUT, 302);
  EXPECT_EQ(RobotTaskStatus::CODE_REQUESTER_LOST, 303);
  EXPECT_EQ(RobotTaskStatus::CODE_RUNTIME_LOST, 304);
  EXPECT_EQ(RobotTaskStatus::CODE_STATION_TIMEOUT, 305);
  EXPECT_EQ(RobotTaskStatus::CODE_STATION_FAULT, 306);
  EXPECT_EQ(RobotTaskStatus::CODE_CANCELLED_WHILE_HOLDING, 307);
  EXPECT_EQ(RobotTaskStatus::CODE_EEF_FAULT, 308);
  EXPECT_EQ(RobotTaskStatus::CODE_CANCELLED_HOLD_UNKNOWN, 309);
  EXPECT_EQ(RobotTaskStatus::CODE_STATION_UNAVAILABLE, 310);
  EXPECT_EQ(RobotTaskStatus::CODE_SERVER_STARTED, 311);
  EXPECT_EQ(RobotTaskStatus::CODE_REQUESTER_CONFLICT, 312);
  EXPECT_EQ(RobotTaskStatus::CODE_STATION_BUSY, 313);   // appended by P0.b (0.5.0)
  EXPECT_EQ(RobotTaskStatus::REJECT_INVALID_REQUEST, 400);
  EXPECT_EQ(RobotTaskStatus::REJECT_UNKNOWN_TASK, 401);
  EXPECT_EQ(RobotTaskStatus::REJECT_BUSY, 402);
  EXPECT_EQ(RobotTaskStatus::REJECT_NOT_READY, 403);
  EXPECT_EQ(RobotTaskStatus::REJECT_FAULTED, 404);
  EXPECT_EQ(RobotTaskStatus::REJECT_DUPLICATE_JOB, 405);
  EXPECT_EQ(RobotTaskStatus::REJECT_NOT_CURRENT_JOB, 406);
  EXPECT_EQ(RobotTaskStatus::REJECT_INVALID_STATE, 407);
  EXPECT_EQ(RobotTaskStatus::REJECT_SEQUENCE_ERROR, 408);
  EXPECT_EQ(RobotTaskStatus::REJECT_NOT_SYNCHRONIZED, 409);
}

TEST(RobotTaskInterface, ADefaultImageSaysNothing)
{
  const RobotTaskCommand c;
  EXPECT_EQ(c.requester_session, 0u);   // 0 = invalid, never adopted
  EXPECT_EQ(c.cmd_seq, 0u);
  EXPECT_EQ(c.cmd, RobotTaskCommand::CMD_NONE);
  EXPECT_EQ(c.job_id, 0u);
  const RobotTaskStatus s;
  EXPECT_EQ(s.interface_version, 0u);   // not a status a requester accepts
  EXPECT_EQ(s.server_session, 0u);
  EXPECT_EQ(s.state, RobotTaskStatus::STATE_UNAVAILABLE);
  EXPECT_FALSE(s.ready);
  EXPECT_EQ(s.ack, RobotTaskStatus::ACK_NONE);
  EXPECT_EQ(s.job_id, 0u);
  EXPECT_EQ(s.result, RobotTaskStatus::RESULT_NONE);
  EXPECT_EQ(s.result_code, RobotTaskStatus::CODE_NONE);
  EXPECT_EQ(s.fault_code, 0u);
}

TEST(RobotTaskInterface, ThePackageThatShipsItIs0_4_0OrLater)
{
  // The images shipped with 0.4.0 (ADR-0032); 0.5.0 (P0.b) only APPENDS the constant CODE_STATION_BUSY 313 (pinned in
  // test_machine_channel.cpp) -- the field layout pinned above is unchanged.
  const std::string xml = readFile(TRAINIT_INTERFACES_PACKAGE_XML);
  std::smatch m;
  ASSERT_TRUE(std::regex_search(xml, m, std::regex("<version>([^<]+)</version>")));
  int major = 0, minor = 0, patch = 0;
  ASSERT_EQ(std::sscanf(m[1].str().c_str(), "%d.%d.%d", &major, &minor, &patch), 3) << m[1].str();
  EXPECT_TRUE(major > 0 || minor >= 4) << "the RTI images ship with trainit_interfaces 0.4.0 (ADR-0032); got "
                                       << m[1].str();
}
