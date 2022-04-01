#include <gtest/gtest.h>

#include "include/csv.h"
#include "tests/test_route/test_route.h"

#define private public
#define protected public
#include "include/lot.h"
#include "include/lot_base.h"
#include "include/route.h"
#undef private
#undef protected

#include <iostream>
#include <string>

using namespace std;


namespace route_index
{
struct route_index_test_case_t {
    string test_route_name;
    int test_oper;
    int rs_idx;

    friend ostream &operator<<(ostream &out,
                               const struct route_index_test_case_t &cs)
    {
        return out << "(" << cs.test_route_name << ", " << cs.test_oper << ", "
                   << cs.rs_idx << ")";
    }
};

class test_route_idx_t
    : public testing::WithParamInterface<struct route_index_test_case_t>,
      public ::test_route::test_route_base_t
{
};


TEST_P(test_route_idx_t, test_index)
{
    auto cs = GetParam();
    EXPECT_EQ(route->findStationIdx(cs.test_route_name, cs.test_oper),
              cs.rs_idx);
}

INSTANTIATE_TEST_SUITE_P(
    oper_index,
    test_route_idx_t,
    testing::Values(route_index_test_case_t{"BGA140", 2050, 0},
                    route_index_test_case_t{"BGA140", 2000, -1},
                    route_index_test_case_t{"MBGA222", 2070, 0},
                    route_index_test_case_t{"MBGA222", 2130, 1},
                    route_index_test_case_t{"MBGA222", 2080, 2},
                    route_index_test_case_t{"MBGA222", 3130, 8},
                    route_index_test_case_t{"MBGA222", 4330, 17},
                    route_index_test_case_t{"MBGA222", 3350, 19},
                    route_index_test_case_t{"MBGA222", 3600, 27},
                    route_index_test_case_t{"MQFNS042", 2070, 0},
                    route_index_test_case_t{"MQFNS042", 2209, 4},
                    route_index_test_case_t{"MQFNS042", 3150, 8},
                    route_index_test_case_t{"MBGA452", 3130, 2},
                    route_index_test_case_t{"MBGA452", 2205, 7},
                    route_index_test_case_t{"MBGA452", 3140, 10},
                    route_index_test_case_t{"MBGA970", 2080, 2},
                    route_index_test_case_t{"MBGA970", 2200, 5},
                    route_index_test_case_t{"MBGA970", 3150, 11},
                    route_index_test_case_t{"MBGA970", 4130, 15},
                    route_index_test_case_t{"QFNS205", 2040, 0},
                    route_index_test_case_t{"QFNS205", 2090, 6},
                    route_index_test_case_t{"QFNS482", 2060, 2},
                    route_index_test_case_t{"QFNS482", 2200, 7},
                    route_index_test_case_t{"BGA404", 2061, 2},
                    route_index_test_case_t{"BGA404", 2090, 6},
                    route_index_test_case_t{"MBGA553", 2080, 1},
                    route_index_test_case_t{"MBGA553", 2205, 5},
                    route_index_test_case_t{"MBGA263", 2070, -1},
                    route_index_test_case_t{"BGA199", 2030, -1},
                    route_index_test_case_t{"BGA319", 2030, -1},
                    route_index_test_case_t{"BGA369", 2050, -1},
                    route_index_test_case_t{"BGA369", 2050, -1},
                    route_index_test_case_t{"FAKE", 2050, -1},
                    route_index_test_case_t{"FAKE", 2050, -1},
                    route_index_test_case_t{"MBGA263", 2130, 0},
                    route_index_test_case_t{"BGA199", 2040, 0},
                    route_index_test_case_t{"BGA319", 2030, -1},
                    route_index_test_case_t{"BGA369", 2050, -1}));

}  // namespace route_index
