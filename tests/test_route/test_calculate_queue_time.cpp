#include <gtest/gtest.h>

#include "tests/test_route/test_route.h"

#include "include/lot.h"
#include "include/wip_lot.h"

#include <string>

namespace route_queue_time
{
struct queue_time_test_case_t {
    string route_name;
    string process_id;
    int oper;
    bool mvin;
    bool is_sub_lot;
    int sum_of_queue_time;

    int expected_status;

    friend ostream &operator<<(ostream &out,
                               const struct queue_time_test_case_t &cs)
    {
        return out << "(" + cs.route_name + "," + cs.process_id << ", "
                   << to_string(cs.oper) + ", " + (cs.mvin ? "true" : "false") +
                          ", " + to_string(cs.sum_of_queue_time) + ")";
    }
};

class test_calculate_queue_time_t
    : public testing::WithParamInterface<struct queue_time_test_case_t>,
      public ::test_route::test_route_base_t
{
public:
    lot_t prepareTestObject(struct queue_time_test_case_t cs)
    {
        lot_t lot;
        lot._mvin = lot.tmp_mvin = cs.mvin;
        lot._is_sub_lot = cs.is_sub_lot;
        lot.tmp_oper = lot._oper = cs.oper;
        lot._route = cs.route_name;
        lot._queue_time = 0;
        return lot;
    }
};

TEST_P(test_calculate_queue_time_t, lot_status)
{
    auto cs = GetParam();
    lot_t lot = prepareTestObject(cs);
    EXPECT_EQ(route->calculateQueueTime(lot), cs.expected_status);
}

TEST_P(test_calculate_queue_time_t, lot_queue_time)
{
    auto cs = GetParam();
    lot_t lot = prepareTestObject(cs);
    route->calculateQueueTime(lot);
    EXPECT_NEAR(lot._queue_time, cs.sum_of_queue_time, 0.001);
}



// TEST_P(test_queue_time_t, wip_lot_queue_time){

// }

INSTANTIATE_TEST_SUITE_P(
    test_queue_time,
    test_calculate_queue_time_t,
    testing::Values(queue_time_test_case_t{"BGA140"s, "", 2050, true, false, 0,
                                           TRAVERSE_DA_UNARRIVED},
                    queue_time_test_case_t{
                        "BGA140"s, "", 2070, true, true, 395,
                        TRAVERSE_FINISHED | TRAVERSE_DA_MVIN}));
}  // namespace route_queue_time
