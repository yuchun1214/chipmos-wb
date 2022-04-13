
set(CMAKE_CXX_STANDARD 20)
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(TEST_SRC
        #         test_entities/test_entities_ctor.cpp
        #         test_entities/test_entities_read_dedicate_machines_file.cpp
        #         test_entity/test_entity.cpp
        test_lot/test_lot.cpp
        test_lot/test_lot_check.cpp
        test_lot/test_lot_addlog.cpp
        test_lot/test_automotive.cpp
        test_lots/test_lots.cpp
        #         test_machines/test_machines_addMachine.cpp
        #         test_machines/test_machines_addPrescheduledJob.cpp 
        #         test_machines/test_machines_prescheduleJobs.cpp
        #         test_machines/test_machines_scheduleAGroup.cpp
        #         test_machines/test_machines_addGroupJobs.cpp
        #         test_machines/test_machines_isDedicatedMachine.cpp
        #         test_machines/test_machines_groupBy.cpp
        #         test_machines/test_machines_distributeAResource.cpp
        #         test_machines/test_machines_canJobRunOnTheMachine.cpp
        #         test_machines/test_machines_addNewResource.cpp
        test_algorithm/test_prescheduling.cpp
        test_csv/test_csv_read.cpp
        test_csv/test_csv_parse.cpp
        #         test_machine_constraint/test_machine_constraint_base.cpp
        #         test_machine_constraint/test_transform_attr_to_regex_string.cpp
        #         test_machine_constraint/test_machine_constraint_r.cpp
        #         test_machine_constraint/test_machine_constraint_a.cpp
        test_lot/test_lot_base_ctor.cpp
        test_lot/test_lot_base_setupDefaultValueOfRow.cpp
        test_lot/test_wip_lot_ctor.cpp
        test_machine_constraint/test_machine_constraint_a.cpp
        test_infrastructure/test_string_to_info.cpp
        test_infrastructure/test_time_converter.cpp
        test_infrastructure/test_is_numeric.cpp
        test_route/test_setup_stations_before_wb.cpp
        test_route/test_oper_index.cpp
        test_route/test_route.cpp
        test_route/test_is_in_stations.cpp
        test_route/test_route_queue_time.cpp
        test_route/test_setup_cure_time.cpp
        test_route/test_get_cure_time.cpp
        test_route/test_calculate_queue_time.cpp
)

add_executable(
    test
    main.cpp
    ${TEST_SRC}
)

if (WIN32)
    target_link_libraries(
        test
        src
        pgalib
        -static-libstdc++
        m
        gtest_main
    )
else()
    target_link_libraries(
        test
        src
        pgalib
        stdc++
        m
        gtest_main
    )
endif()

include(GoogleTest)
gtest_discover_tests(test)
