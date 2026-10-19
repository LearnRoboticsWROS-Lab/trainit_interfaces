// ADR-0026: the grasp candidate is exactly the ADR's. Pinned here, through rosidl's own
// introspection (compilation alone would not notice a reordered, renamed or retyped field):
//  1. GraspCandidate's fields, in order, with their types (Humble has no type hashes: a change ships
//     in lockstep and starts here);
//  2. GraspCandidateArray: a header and the candidates, best first; an EMPTY array is a valid "no
//     candidate" answer;
//  3. a default-constructed candidate carries nothing a consumer could execute: no frame, no id, no
//     provider, a zero approach (refused as "not a unit vector") and a zero quaternion;
//  4. no constants: the candidate has no closed vocabulary, so the ADR-0022 token file is untouched;
//  5. the package version that ships the type (0.2.0).
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

#include "trainit_interfaces/msg/grasp_candidate.hpp"
#include "trainit_interfaces/msg/grasp_candidate_array.hpp"

namespace
{
using trainit_interfaces::msg::GraspCandidate;
using trainit_interfaces::msg::GraspCandidateArray;
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

TEST(GraspCandidate, FieldsAreExactlyTheAdrsInOrder)
{
  EXPECT_EQ(fieldsOf(membersOf<GraspCandidate>()),
            (std::vector<std::string>{
              "header:std_msgs/Header", "id:string", "provider:string", "grasp_pose:geometry_msgs/Pose",
              "approach:geometry_msgs/Vector3", "standoff_m:float64", "eef:string", "site:string",
              "score:float64", "confidence:float64", "object_class:string", "object_id:string",
              "surface_normal:geometry_msgs/Vector3", "metadata:diagnostic_msgs/KeyValue[]"}));
}

TEST(GraspCandidate, TheArrayIsAHeaderAndTheCandidatesBestFirst)
{
  EXPECT_EQ(fieldsOf(membersOf<GraspCandidateArray>()),
            (std::vector<std::string>{"header:std_msgs/Header", "candidates:trainit_interfaces/GraspCandidate[]"}));
  const GraspCandidateArray empty;
  EXPECT_TRUE(empty.candidates.empty());   // "no candidate": a valid answer, never malformed data

  GraspCandidateArray a;
  for (const char* id : {"best", "second", "third"})
  {
    GraspCandidate c;
    c.id = id;
    a.candidates.push_back(c);
  }
  ASSERT_EQ(a.candidates.size(), 3u);   // the order is the rank: kept as published
  EXPECT_EQ(a.candidates[0].id, "best");
  EXPECT_EQ(a.candidates[2].id, "third");
}

TEST(GraspCandidate, ADefaultCandidateCarriesNothingExecutable)
{
  const GraspCandidate c;
  EXPECT_TRUE(c.header.frame_id.empty());   // no frame: unresolvable
  EXPECT_EQ(c.header.stamp.sec, 0);
  EXPECT_TRUE(c.id.empty());
  EXPECT_TRUE(c.provider.empty());
  // a zero approach is not a unit vector (refused), and the default quaternion is (0, 0, 0, 1)
  EXPECT_EQ(c.approach.x, 0.0);
  EXPECT_EQ(c.approach.y, 0.0);
  EXPECT_EQ(c.approach.z, 0.0);
  EXPECT_EQ(c.grasp_pose.orientation.w, 1.0);
  EXPECT_EQ(c.standoff_m, 0.0);             // "none suggested"
  EXPECT_TRUE(c.eef.empty());               // the application's default device
  EXPECT_TRUE(c.site.empty());              // the device's only site
  EXPECT_EQ(c.score, 0.0);
  EXPECT_EQ(c.confidence, 0.0);
  EXPECT_TRUE(c.object_class.empty());
  EXPECT_TRUE(c.object_id.empty());
  EXPECT_EQ(c.surface_normal.x, 0.0);       // zero = no normal
  EXPECT_EQ(c.surface_normal.y, 0.0);
  EXPECT_EQ(c.surface_normal.z, 0.0);
  EXPECT_TRUE(c.metadata.empty());
}

TEST(GraspCandidate, HasNoConstants)
{
  // a constant would be a closed vocabulary: it would need its token file and its ADR row
  const std::string dir = std::string(TRAINIT_INTERFACES_PACKAGE_XML).substr(
    0, std::string(TRAINIT_INTERFACES_PACKAGE_XML).rfind('/'));
  for (const char* file : {"/msg/GraspCandidate.msg", "/msg/GraspCandidateArray.msg"})
  {
    std::istringstream in(readFile((dir + file).c_str()));
    std::string line;
    while (std::getline(in, line))
    {
      const auto code = line.substr(0, line.find('#'));
      EXPECT_EQ(code.find('='), std::string::npos) << file << ": a constant: " << line;
    }
  }
}

TEST(GraspCandidate, ThePackageThatShipsItIs0_2_0)
{
  const std::string xml = readFile(TRAINIT_INTERFACES_PACKAGE_XML);
  std::smatch m;
  ASSERT_TRUE(std::regex_search(xml, m, std::regex("<version>([^<]+)</version>")));
  EXPECT_EQ(m[1].str(), "0.2.0") << "GraspCandidate / GraspCandidateArray ship with trainit_interfaces 0.2.0 "
                                    "(ADR-0026): TMR 2.5.0 declares that floor";
}
