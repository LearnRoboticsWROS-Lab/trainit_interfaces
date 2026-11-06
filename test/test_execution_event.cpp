// ADR-0030: the execution event is exactly the ADR's. Pinned through rosidl's introspection:
//  1. the fields, in order, with their types (Humble has no type hashes: a change ships in lockstep);
//  2. the closed vocabularies (kind, status): their values, and that a default event is "nothing";
//  3. a default event carries no identity, no task, no step, no process value (NaN) and no attempt;
//  4. the package version that ships the type (0.3.0).
#include <cmath>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>
#include <rosidl_typesupport_introspection_cpp/field_types.hpp>
#include <rosidl_typesupport_introspection_cpp/message_introspection.hpp>
#include <rosidl_typesupport_introspection_cpp/message_type_support_decl.hpp>

#include "trainit_interfaces/msg/execution_event.hpp"

namespace
{
using trainit_interfaces::msg::ExecutionEvent;
using rosidl_typesupport_introspection_cpp::MessageMembers;

template <typename MsgT>
const MessageMembers* membersOf()
{
  const auto* ts = rosidl_typesupport_introspection_cpp::get_message_type_support_handle<MsgT>();
  return static_cast<const MessageMembers*>(ts->data);
}

// "<name>:<type>" per field, in order; a nested type is "<namespace>/<Name>", a sequence ends in "[]".
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
      case ft::ROS_TYPE_DOUBLE: type = "float64"; break;
      case ft::ROS_TYPE_UINT8: type = "uint8"; break;
      case ft::ROS_TYPE_UINT32: type = "uint32"; break;
      case ft::ROS_TYPE_UINT64: type = "uint64"; break;
      case ft::ROS_TYPE_MESSAGE:
      {
        const auto* sub = static_cast<const MessageMembers*>(f.members_->data);
        std::string ns = sub->message_namespace_;   // e.g. "geometry_msgs::msg"
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

TEST(ExecutionEvent, FieldsAreExactlyTheAdrsInOrder)
{
  EXPECT_EQ(fieldsOf(membersOf<ExecutionEvent>()),
            (std::vector<std::string>{
              "header:std_msgs/Header", "run_id:string", "seq:uint64", "kind:uint8", "status:uint8",
              "task_id:string", "step_id:string", "step_kind:string", "process_channel:string",
              "process_kind:string", "process_value:float64", "attempt_index:uint32", "candidate_id:string",
              "candidate_provider:string"}));
}

TEST(ExecutionEvent, TheClosedVocabulariesAreTheAdrs)
{
  EXPECT_EQ(ExecutionEvent::RUN_STARTED, 1);
  EXPECT_EQ(ExecutionEvent::RUN_ENDED, 2);
  EXPECT_EQ(ExecutionEvent::TASK_ENTERED, 3);
  EXPECT_EQ(ExecutionEvent::TASK_EXITED, 4);
  EXPECT_EQ(ExecutionEvent::STEP_ENTERED, 5);
  EXPECT_EQ(ExecutionEvent::STEP_EXITED, 6);
  EXPECT_EQ(ExecutionEvent::PROCESS_EVENT, 7);
  EXPECT_EQ(ExecutionEvent::STATUS_NONE, 0);
  EXPECT_EQ(ExecutionEvent::SUCCEEDED, 1);
  EXPECT_EQ(ExecutionEvent::FAILED, 2);
  EXPECT_EQ(ExecutionEvent::HALTED, 3);
}

TEST(ExecutionEvent, ADefaultEventSaysNothing)
{
  const ExecutionEvent e;
  EXPECT_EQ(e.kind, 0);                     // no kind: every consumer refuses it
  EXPECT_EQ(e.status, ExecutionEvent::STATUS_NONE);
  EXPECT_TRUE(e.run_id.empty());
  EXPECT_EQ(e.seq, 0u);                     // a real event counts from 1
  EXPECT_TRUE(e.task_id.empty());
  EXPECT_TRUE(e.step_id.empty());
  EXPECT_TRUE(e.step_kind.empty());
  EXPECT_TRUE(e.process_channel.empty());
  EXPECT_TRUE(e.process_kind.empty());
  EXPECT_EQ(e.attempt_index, 0u);
  EXPECT_TRUE(e.candidate_id.empty());
  EXPECT_TRUE(e.candidate_provider.empty());
}

TEST(ExecutionEvent, ThePackageThatShipsItIs0_3_0)
{
  const std::string xml = readFile(TRAINIT_INTERFACES_PACKAGE_XML);
  std::smatch m;
  ASSERT_TRUE(std::regex_search(xml, m, std::regex("<version>([^<]+)</version>")));
  EXPECT_EQ(m[1].str(), "0.3.0") << "ExecutionEvent ships with trainit_interfaces 0.3.0 (ADR-0030): TMR and "
                                    "trainit_data_ros declare that floor";
}
