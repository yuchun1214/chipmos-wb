#include <gtest/gtest.h>
#include <exception>

#include "tests/test_route/test_route.h"

using namespace std;

namespace setup_cure_time
{
struct error_cure_time_test_case_t {
    int file_index;
    string characters;
    friend ostream &operator<<(ostream &os,
                               const struct error_cure_time_test_case_t &cs)
    {
        return os << "("
                  << "test_data/wrong_cure_time/cure_time" << cs.file_index
                  << ".csv->" << cs.characters << ")";
    }
};

class test_setup_cure_time_t
    : public testing::TestWithParam<struct error_cure_time_test_case_t>
{
public:
    route_t *route;
    void SetUp() override { route = new route_t(); }

    void TearDown() override { delete route; }
};

// TEST_F(test_setup_cure_time_t, rmk_header){
//     csv_t rmk("test_data/route_list.csv", "r", true, true);
//     csv_t cure_time("test_data/cure_time.csv", "r", true, true);
//     EXPECT_THROW(
//                 {
//                     try{
//                         route->setCureTime(rmk, cure_time);
//                     }catch(out_of_range & e){
//                         EXPECT_EQ(e.what(), "The rmk_file should contains
//                         these two headers, " throw;
//                     }
//                 }, out_of_range);
// }

// TEST_F(test_setup_cure_time_t, cure_time_header){

// }

TEST_P(test_setup_cure_time_t, cure_time_data)
{
    auto cs = GetParam();
    csv_t cure_time("test_data/wrong_cure_time/cure_time" +
                        to_string(cs.file_index) + ".csv",
                    "r", true, true);
    csv_t rmk("test_data/process_find_lot_size_and_entity.csv", "r", true,
              true);
    EXPECT_THROW(
        {
            try {
                route->setCureTime(rmk, cure_time);
            } catch (invalid_argument &e) {
                EXPECT_EQ(
                    e.what(),
                    "The cure time file contains invalid characters such as " +
                        cs.characters);
                throw;
            }
        },
        invalid_argument);
}

INSTANTIATE_TEST_SUITE_P(
    test_setup_cure_time,
    test_setup_cure_time_t,
    testing::Values(error_cure_time_test_case_t{0, "9v"},
                    error_cure_time_test_case_t{1, "6y,qi"},
                    error_cure_time_test_case_t{2, "6y,6O,qi"},
                    error_cure_time_test_case_t{3, "6y,6O,qi,15l"},
                    error_cure_time_test_case_t{4, "9v,o,aa,3o3"},
                    error_cure_time_test_case_t{5, ",o,1l0"},
                    error_cure_time_test_case_t{6, ",o,1l0,3a"},
                    error_cure_time_test_case_t{7, ",xi,qoo,1l0,3a,"},
                    error_cure_time_test_case_t{8, ",o,1l0,sss,3a,xyz"},
                    error_cure_time_test_case_t{9, ",6o6,00l,,3a3"}


                    ));
}  // namespace setup_cure_time
