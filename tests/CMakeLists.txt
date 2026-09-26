add_executable(JevWrapperPP_tests ts_jev_client_test.cpp)

target_link_libraries(JevWrapperPP_tests PRIVATE JevWrapperPP::JevWrapperPP)

add_test(NAME JevWrapperPP_tests COMMAND JevWrapperPP_tests)
