// P0.b: the machine-service images (ChannelRequest, ChannelStatus) are exactly the pre-registered contract
// (ADR-0024 Amendment P0 as REPLACED by P0B_DESIGN.md §3.2). Pinned through rosidl's introspection:
//  1. the fields of both images, in order, with their types: a PLC-side library (P0.c) copies them 1:1, so the
//     layout is a public wire contract -- fields may only be APPENDED;
//  2. the closed vocabularies (state, reject_code, fault_code): stable public values, never renumbered;
//  3. a default image says "nothing": no session, no state, no cycle, no rejection, no fault -- never a disguised
//     READY / DONE, and never a request a source could execute (source_session 0, seq 0);
//  4. the Robot Task Interface code STATION_BUSY 313, appended by P0.b;
//  5. the package version that ships them (0.5.0).
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <rosidl_typesupport_introspection_cpp/field_types.hpp>
#include <rosidl_typesupport_introspection_cpp/message_introspection.hpp>
#include <rosidl_typesupport_introspection_cpp/message_type_support_decl.hpp>

#include "trainit_interfaces/msg/channel_request.hpp"
#include "trainit_interfaces/msg/channel_status.hpp"
#include "trainit_interfaces/msg/robot_task_status.hpp"

namespace
{
using trainit_interfaces::msg::ChannelRequest;
using trainit_interfaces::msg::ChannelStatus;
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

TEST(MachineChannel, TheRequestImageIsExactlyTheContractInOrder)
{
  EXPECT_EQ(fieldsOf(membersOf<ChannelRequest>()),
            (std::vector<std::string>{"header:std_msgs/Header", "source_session:uint32", "requester_session:uint32",
                                      "seq:uint32"}));
}

TEST(MachineChannel, TheStatusImageIsExactlyTheContractInOrder)
{
  EXPECT_EQ(fieldsOf(membersOf<ChannelStatus>()),
            (std::vector<std::string>{
              "header:std_msgs/Header", "source_session:uint32", "heartbeat:uint16", "state:uint8",
              "cycle_requester:uint32", "cycle_seq:uint32", "cycle_done:bool", "reject_requester:uint32",
              "reject_seq:uint32", "reject_code:uint8", "fault_code:uint16", "detail:string"}));
}

TEST(MachineChannel, TheClosedVocabulariesAreTheContracts)
{
  EXPECT_EQ(ChannelStatus::STATE_NONE, 0);
  EXPECT_EQ(ChannelStatus::STATE_READY, 1);
  EXPECT_EQ(ChannelStatus::STATE_BUSY, 2);
  EXPECT_EQ(ChannelStatus::STATE_DONE, 3);
  EXPECT_EQ(ChannelStatus::STATE_FAULT, 4);
  EXPECT_EQ(ChannelStatus::STATE_HOMING, 5);
  EXPECT_EQ(ChannelStatus::REJECT_NONE, 0);
  EXPECT_EQ(ChannelStatus::REJECT_BUSY, 1);
  EXPECT_EQ(ChannelStatus::REJECT_FAULTED, 2);
  EXPECT_EQ(ChannelStatus::REJECT_TWO_REQUESTERS, 3);
  EXPECT_EQ(ChannelStatus::FAULT_NONE, 0);
  EXPECT_EQ(ChannelStatus::FAULT_ACTUATOR_TIMEOUT, 1);
  EXPECT_EQ(ChannelStatus::FAULT_SENSOR_FAULT, 2);
  EXPECT_EQ(ChannelStatus::FAULT_STATION_FAULT, 3);
}

TEST(MachineChannel, ADefaultImageSaysNothing)
{
  const ChannelRequest r;
  EXPECT_EQ(r.source_session, 0u);     // 0 is never a source's session: never executed
  EXPECT_EQ(r.requester_session, 0u);
  EXPECT_EQ(r.seq, 0u);                // 0 is never a request's seq
  const ChannelStatus s;
  EXPECT_EQ(s.source_session, 0u);     // no source
  EXPECT_EQ(s.state, ChannelStatus::STATE_NONE);   // neither READY nor DONE
  EXPECT_EQ(s.cycle_requester, 0u);
  EXPECT_EQ(s.cycle_seq, 0u);
  EXPECT_FALSE(s.cycle_done);
  EXPECT_EQ(s.reject_requester, 0u);
  EXPECT_EQ(s.reject_seq, 0u);
  EXPECT_EQ(s.reject_code, ChannelStatus::REJECT_NONE);
  EXPECT_EQ(s.fault_code, ChannelStatus::FAULT_NONE);
}

TEST(MachineChannel, StationBusyIsAppendedAs313)
{
  // ADR-0032 Decision 7 note (P0.b): appended after 312, never renumbering a published value.
  EXPECT_EQ(RobotTaskStatus::CODE_REQUESTER_CONFLICT, 312);
  EXPECT_EQ(RobotTaskStatus::CODE_STATION_BUSY, 313);
}

TEST(MachineChannel, ThePackageThatShipsThemIs0_5_0)
{
  const std::string xml = readFile(TRAINIT_INTERFACES_PACKAGE_XML);
  std::smatch m;
  ASSERT_TRUE(std::regex_search(xml, m, std::regex("<version>([^<]+)</version>")));
  EXPECT_EQ(m[1].str(), "0.5.0") << "ChannelRequest / ChannelStatus and STATION_BUSY 313 ship with trainit_interfaces "
                                    "0.5.0 (P0.b)";
}
