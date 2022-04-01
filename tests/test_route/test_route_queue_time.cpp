#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>

using namespace std;

#include "tests/test_route/test_route.h"

namespace route_queue_time
{
struct route_queue_time_test_case_t {
    int test_oper1, test_oper2;
    double queue_time;
    friend ostream &operator<<(ostream &os,
                               const struct route_queue_time_test_case_t &cs)
    {
        return os << "(" << cs.test_oper1 << "," << cs.test_oper2 << ")->"
                  << cs.queue_time;
    }
};

class test_route_queue_time_t
    : public test_route::test_route_base_t,
      public testing::WithParamInterface<struct route_queue_time_test_case_t>
{
};

TEST_P(test_route_queue_time_t, get_queue_time)
{
    auto cs = GetParam();
    EXPECT_NEAR(route->getQueueTime(cs.test_oper1, cs.test_oper2),
                cs.queue_time, 0.0001);
}

INSTANTIATE_TEST_SUITE_P(
    queue_time_array,
    test_route_queue_time_t,
    testing::Values(route_queue_time_test_case_t{2070, 2080, 120},
                    route_queue_time_test_case_t{2090, 2200, 15},
                    route_queue_time_test_case_t{2059, 2070, 600},
                    route_queue_time_test_case_t{5530, 5530, -1},
                    route_queue_time_test_case_t{2080, 2130, 120},
                    route_queue_time_test_case_t{2064, 3200, 30},
                    route_queue_time_test_case_t{6330, 3550, 180}));

struct error_queue_time_csv_file_test_case_t {
    int file_index;
    string characters;
    friend ostream &operator<<(
        ostream &os,
        const struct error_queue_time_csv_file_test_case_t &cs)
    {
        return os << "("
                  << "test_data/wrong_queue_time/queuetime" +
                         to_string(cs.file_index) + ".csv"
                  << "->" << cs.characters << ")";
    }
};

// Test the exception string of the function
class test_route_queue_time_setup_t
    : public testing::Test,
      public testing::WithParamInterface<
          struct error_queue_time_csv_file_test_case_t>
{
public:
    route_t *route;
    void SetUp() override { route = new route_t(); }

    void TearDown() override { delete route; }
};

// if column's name doesn't have station
// the function should output message and tell the user
// what happen
TEST_F(test_route_queue_time_setup_t, invalid_csv_format)
{
    csv_t fake_file("test_data/route_list.csv"s, "r", true, true);
    EXPECT_THROW(
        try { route->setQueueTime(fake_file); } catch (out_of_range &e) {
            EXPECT_STREQ("queue_time file doesn't contain header station",
                         e.what());
            throw;
        },
        out_of_range);
}

TEST_P(test_route_queue_time_setup_t, invalid_data_format)
{
    auto cs = GetParam();
    csv_t wrong_file("test_data/wrong_queue_time/queuetime" +
                         to_string(cs.file_index) + ".csv",
                     "r", true, true);
    EXPECT_THROW(
        try { route->setQueueTime(wrong_file); } catch (invalid_argument &e) {
            EXPECT_EQ(
                "queue_time file has invalid data format, queue time shoud be "
                "a number, but the file contains : " +
                    cs.characters,
                e.what());
            throw;
        },
        invalid_argument);
}

INSTANTIATE_TEST_SUITE_P(
    invalid_queue_time_data,
    test_route_queue_time_setup_t,
    testing::Values(error_queue_time_csv_file_test_case_t{0, "a,b,c"},
                    error_queue_time_csv_file_test_case_t{1, "asdf,we,te"},
                    error_queue_time_csv_file_test_case_t{2, "asdf,we,kj30,te"},
                    error_queue_time_csv_file_test_case_t{
                        3, "asdf,we,kj30,te,abcd,ef,-j"},
                    error_queue_time_csv_file_test_case_t{
                        4, "ab30,asdf,we,kj30,te,abcd,ef,-j"},
                    error_queue_time_csv_file_test_case_t{
                        5, "ab30,asdf,we,kj30,te,abcd,ef,-j,ab,cd,"},
                    error_queue_time_csv_file_test_case_t{
                        6, "ab30,asdf,we,kj30,te,abcd,ef,-j,ab,cd,,0.25o"}));

}  // namespace route_queue_time
