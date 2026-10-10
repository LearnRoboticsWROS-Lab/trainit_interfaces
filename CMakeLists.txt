cmake_minimum_required(VERSION 3.8)
project(trainit_interfaces)

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(std_msgs REQUIRED)

# ADR-0022 D3: two messages, and nothing else. A new type needs an ADR.
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/EndEffectorState.msg"
  "msg/EndEffectorActuatorState.msg"
  DEPENDENCIES builtin_interfaces std_msgs
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
endif()

ament_package()
