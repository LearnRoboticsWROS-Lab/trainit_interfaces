cmake_minimum_required(VERSION 3.8)
project(trainit_interfaces)

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(diagnostic_msgs REQUIRED)

# Every type has its ADR: the end-effector state (ADR-0022 D3), the grasp candidate (ADR-0026), the
# execution event (ADR-0030), the Robot Task Interface images (ADR-0032) and the machine-service images (ADR-0024
# Amendment P0 as replaced by the P0.b pre-registration, P0B_DESIGN.md §3).
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/EndEffectorState.msg"
  "msg/EndEffectorActuatorState.msg"
  "msg/GraspCandidate.msg"
  "msg/GraspCandidateArray.msg"
  "msg/ExecutionEvent.msg"
  "msg/RobotTaskCommand.msg"
  "msg/RobotTaskStatus.msg"
  "msg/ChannelRequest.msg"
  "msg/ChannelStatus.msg"
  DEPENDENCIES builtin_interfaces std_msgs geometry_msgs diagnostic_msgs
)

ament_export_dependencies(rosidl_default_runtime)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  # The messages are exactly ADR-0022's: the field list, the constants (checked against the
  # pinned canonical token file, a byte copy of the one TMR, the hardware layer and TSA check
  # against), and "a default-constructed message means cannot tell".
  ament_add_gtest(test_end_effector_state test/test_end_effector_state.cpp)
  rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME} rosidl_typesupport_cpp)
  rosidl_get_typesupport_target(introspection_target ${PROJECT_NAME} rosidl_typesupport_introspection_cpp)
  target_link_libraries(test_end_effector_state "${cpp_typesupport_target}" "${introspection_target}")
  set(_tokens ${CMAKE_CURRENT_SOURCE_DIR}/test/data/eef_contract_tokens.txt)
  file(SHA256 ${_tokens} _tokens_sha256)
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${_tokens})  # re-hash on edit
  target_compile_definitions(test_end_effector_state PRIVATE
    TRAINIT_EEF_TOKENS_FILE="${_tokens}"
    TRAINIT_EEF_TOKENS_SHA256="${_tokens_sha256}")
  # ADR-0026: the grasp candidate is exactly the ADR's -- the field list in order, the field types,
  # the defaults ("no candidate" is an empty array; a default candidate is refused by every consumer)
  # and the package version that ships it.
  ament_add_gtest(test_grasp_candidate test/test_grasp_candidate.cpp)
  target_link_libraries(test_grasp_candidate "${cpp_typesupport_target}" "${introspection_target}")
  target_compile_definitions(test_grasp_candidate PRIVATE
    TRAINIT_INTERFACES_PACKAGE_XML="${CMAKE_CURRENT_SOURCE_DIR}/package.xml")
  # ADR-0030: the execution event is exactly the ADR's -- fields in order with their types, the closed
  # kind / status vocabularies, "a default event says nothing", and the package version that ships it.
  ament_add_gtest(test_execution_event test/test_execution_event.cpp)
  target_link_libraries(test_execution_event "${cpp_typesupport_target}" "${introspection_target}")
  target_compile_definitions(test_execution_event PRIVATE
    TRAINIT_INTERFACES_PACKAGE_XML="${CMAKE_CURRENT_SOURCE_DIR}/package.xml")
  # ADR-0032: the Robot Task Interface images are exactly the ADR's -- both field lists in order (a PLC adapter copies
  # them 1:1: fields may only be appended), the closed vocabularies, every result / rejection code (stable public
  # values), "a default image says nothing", and the package version that ships them.
  ament_add_gtest(test_robot_task_interface test/test_robot_task_interface.cpp)
  target_link_libraries(test_robot_task_interface "${cpp_typesupport_target}" "${introspection_target}")
  target_compile_definitions(test_robot_task_interface PRIVATE
    TRAINIT_INTERFACES_PACKAGE_XML="${CMAKE_CURRENT_SOURCE_DIR}/package.xml")
  # P0.b: the machine-service images are exactly the pre-registered contract -- both field lists in order (a PLC-side
  # library copies them 1:1: fields may only be appended), the closed vocabularies, "a default image says nothing",
  # STATION_BUSY 313 appended, and the package version that ships them.
  ament_add_gtest(test_machine_channel test/test_machine_channel.cpp)
  target_link_libraries(test_machine_channel "${cpp_typesupport_target}" "${introspection_target}")
  target_compile_definitions(test_machine_channel PRIVATE
    TRAINIT_INTERFACES_PACKAGE_XML="${CMAKE_CURRENT_SOURCE_DIR}/package.xml")
endif()

ament_package()
