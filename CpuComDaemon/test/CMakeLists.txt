project(CPUCD_TESTS CXX)

file(GLOB_RECURSE CPUCD_GTEST_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

enable_testing()

add_executable(cpucd_gtest "${CPUCD_GTEST_SRC}")

SET_TARGET_PROPERTIES(cpucd_gtest PROPERTIES LINKER_LANGUAGE CXX)


target_include_directories(cpucd_gtest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/vcpu/protocol/
)

target_link_libraries(cpucd_gtest CPUCD gtest gtest_main gmock pthread)
