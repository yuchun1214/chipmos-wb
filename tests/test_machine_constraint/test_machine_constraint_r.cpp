//
// Created by YuChunLin on 2021/11/22.
//

#include <gtest/gtest.h>
#include <string>

#define private public
#define protected public

#include "include/machine_constraint.h"
#include "include/machine_constraint_r.h"
#include "test_machine_constraint_base.h"

#undef private
#undef protected

using namespace std;

class test_machine_constraint_r_t : public test_machine_constraint_suite_t
{
};

TEST_P(test_machine_constraint_r_t, r)
{
    auto cs = GetParam();
    init_job(cs);
    init_machine(cs);
    bool care = true, ret;
    ret = mcs_r->isMachineRestrained(j, m, &care);
    EXPECT_EQ(cs.out, care && ret);
}

INSTANTIATE_TEST_SUITE_P(machine_constraint_r,
                         test_machine_constraint_r_t,
                         testing::Values(
                             testing_machine_constraint_case_t{
                                 "FBGA-96UEF", "ZJP", "P3T1GF40CBF-Z{YZ}", 2200,
                                 "BB544", "UTC1000S", false},

                             testing_machine_constraint_case_t{
                                 "SSLGA16DOU", "MXIC", "PKGID", 2200, "BB544",
                                 "UTC3000", false}));
