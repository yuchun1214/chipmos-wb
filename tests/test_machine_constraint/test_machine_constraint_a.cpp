//
// Created by YuChunLin on 2021/11/22.
//

#include <gtest/gtest.h>
#include <string>

#include "include/info.h"
#include "test_machine_constraint_base.h"

#define private public
#define protected public

#include "include/machine_constraint.h"
#include "include/machine_constraint_a.h"

#undef private
#undef protected

using namespace std;

class test_machine_constraint_a_t : public test_machine_constraint_suite_t
{
};


TEST_P(test_machine_constraint_a_t, a)
{
    bool care, ret;
    auto cs = GetParam();
    init_job(cs);
    init_machine(cs);
    ret = mcs_a->isMachineRestrained(j, m, &care);
    EXPECT_EQ(cs.out, care && ret);
}

// struct testing_machine_constraint_case_t {
//     // job information
//     string pin_pkg;
//     string customer;
//     int oper;
//
//     // machine information
//     string machine_no;
//     string model_name;
//     bool out;
// }
INSTANTIATE_TEST_SUITE_P(machine_constraint_a,
                         test_machine_constraint_a_t,
                         testing::Values(testing_machine_constraint_case_t{
                             "FBGA-96UEF", "ZJP", "P3T1GF40CBF-Z{YZ}", 2200,
                             "BB544", "UTC2000", true}));
