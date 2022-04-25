#include <gtest/gtest.h>
#include <gtest/internal/gtest-internal.h>

#include "include/csv.h"
#include "tests/test_route/test_route.h"

#define private public
#define protected public
#include "include/route.h"
#undef private
#undef protected

#include <exception>
#include <map>
#include <string>
#include <vector>

using namespace std;


namespace route
{
struct route_test_case_t {
    string test_route_name;
    vector<int> test_opers;
    friend ostream &operator<<(ostream &out, struct route_test_case_t cs)
    {
        out << "route name :" << cs.test_route_name << "->";
        out << ::testing::PrintToString(cs.test_opers);
        return out;
    }
};


class test_route_t : public testing::WithParamInterface<route_test_case_t>,
                     public test_route::test_route_base_t
{
};


TEST_P(test_route_t, test_if_oper_in_WB_7)
{
    route_test_case_t cs = GetParam();
    set<int> route_list = route->_beforeWB[cs.test_route_name];
    vector<int> _v_route_list(route_list.begin(), route_list.end());
    vector<int> &test_oper = cs.test_opers;

    ASSERT_EQ(route_list.size(), test_oper.size())
        << cs << ", route list : " << ::testing::PrintToString(route_list)
        << endl;
    for (int i = 0; i < test_oper.size(); ++i) {
        ASSERT_EQ(route_list.count(test_oper[i]), 1)
            << cs << ", route list : " << ::testing::PrintToString(route_list)
            << endl;
    }
}

INSTANTIATE_TEST_SUITE_P(
    route_list,
    test_route_t,
    testing::Values(
        route_test_case_t{"BGA140",
                          {2050, 2060, 2061, 2070, 2080, 2150, 2090, 2200}},
        route_test_case_t{"BGA145",
                          {2040, 2050, 2060, 2070, 2080, 2150, 2090, 2200}},
        route_test_case_t{"BGA154",
                          {2050, 2060, 2061, 2070, 2080, 2150, 2090, 2200}},
        route_test_case_t{"BGA161",
                          {2040, 2050, 2060, 2070, 2080, 2150, 2090, 2200}},
        route_test_case_t{
            "MBGA138",
            {2070, 2080, 2150, 2090, 2200, 2205, 2209, 2130, 2140, 3150, 3200}},
        route_test_case_t{"MBGA176",
                          {2070, 2080, 2150, 2090, 2200, 2205, 2210, 2130, 3130,
                           3140, 3150, 3200}},
        route_test_case_t{"MBGA222",
                          {2070, 2130, 2080, 2150, 2090, 2200, 2205, 2210, 3130,
                           2140, 3330, 3340, 3150, 3200, 3205, 3210, 4130, 4330,
                           4340, 3350, 3400, 3405, 3410, 4530, 4730, 4740, 3550,
                           3600, 3605, 3610, 5130, 5330, 5340, 4150, 4200}},
        route_test_case_t{"BGA206",
                          {2058, 2050, 2060, 2070, 2080, 2150, 2090, 2200}},
        route_test_case_t{
            "MCON74",
            {2070, 2080, 2150, 2090, 2200, 2205, 2210, 2130, 2140, 3150, 3200}},
        route_test_case_t{"CON220",
                          {2050, 2060, 2061, 2070, 2080, 2150, 2090, 2200}},
        route_test_case_t{"MBGA393",
                          {2070, 2080, 2150, 2090, 2200, 2205, 2209, 2210, 2130,
                           2140, 3150, 3200, 3205, 3209, 3210, 3130, 3140, 3350,
                           3400, 3405, 3409, 3410, 3330, 3340, 3550, 3600}},
        route_test_case_t{
            "MQFNS042",
            {2070, 2150, 2090, 2200, 2209, 2210, 2130, 2140, 3150, 3200}},
        route_test_case_t{"MBGA578",
                          {2070, 2080, 2130, 3130, 3140, 2150, 2090, 2200}},
        route_test_case_t{"MCON57", {2070, 2130, 3130, 3140, 2150, 2090, 2200}},
        route_test_case_t{"MBGA452",
                          {2070, 2130, 3130, 2140, 2150, 2090, 2200, 2205, 2210,
                           3330, 3140, 3150, 3200}},
        route_test_case_t{
            "MBGA970",
            {2070, 2130, 2080, 2150, 2090, 2200, 2205, 2210, 3130, 2140, 3330,
             3150, 3200, 3205, 3210, 4130, 3350, 3400}},
        route_test_case_t{"MQFNS020",
                          {2070, 2080, 2130, 2140, 2150, 2090, 2200}},
        route_test_case_t{"QFNS205",
                          {2040, 2050, 2060, 2070, 2080, 2150, 2090, 2200}},
        route_test_case_t{"QFNS288",
                          {2040, 2050, 2060, 2070, 2080, 2150, 2090, 2200}},
        route_test_case_t{"QFNS169",
                          {2040, 2050, 2060, 2070, 2080, 2150, 2090, 2200}},
        route_test_case_t{"QFNS329",
                          {2050, 2060, 2061, 2070, 2080, 2150, 2090, 2200}},
        route_test_case_t{"QFNS482",
                          {2040, 2050, 2060, 2070, 2080, 2150, 2090, 2200}},
        route_test_case_t{"QFNS605",
                          {2040, 2050, 2060, 2070, 2080, 2150, 2090, 2200}},
        route_test_case_t{"BGA387",
                          {2040, 2050, 2060, 2070, 2080, 2150, 2090, 2200}},
        route_test_case_t{"BGA404",
                          {2059, 2060, 2061, 2070, 2080, 2150, 2090, 2200}},
        route_test_case_t{"BGA326",
                          {2056, 2059, 2060, 2070, 2080, 2150, 2090, 2200}},
        route_test_case_t{"BGA626",
                          {2059, 2060, 2061, 2070, 2080, 2150, 2090, 2200}},
        route_test_case_t{"BGA154",
                          {2050, 2060, 2061, 2070, 2080, 2150, 2090, 2200}},
        route_test_case_t{"BGA185",
                          {2050, 2060, 2061, 2070, 2080, 2150, 2090, 2200}},
        route_test_case_t{"BGA40",
                          {2050, 2060, 2061, 2070, 2080, 2150, 2090, 2200}},
        route_test_case_t{
            "MBGA553",
            {2070, 2080, 2150, 2090, 2200, 2205, 2130, 2140, 3150, 3200}},
        route_test_case_t{"MBGA874", {2070, 2080, 2150, 2090, 2200}},
        route_test_case_t{"BGA748",
                          {2050, 2060, 2061, 2070, 2080, 2150, 2090, 2200}}));
}  // namespace route
