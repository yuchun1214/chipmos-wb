//
// Created by YuChunLin on 2021/11/20.
//
#ifndef __TEST_MACHINE_CONSTRAINT_BASE_H__
#define __TEST_MACHINE_CONSTRAINT_BASE_H__

#include <gtest/gtest.h>
#include <map>
#include <string>
#include <vector>

#define private public
#define protected public

#include "include/machine_constraint.h"
#include "include/machine_constraint_a.h"
#include "include/machine_constraint_r.h"

#undef private
#undef protected

using namespace std;

struct testing_machine_constraint_case_t {
    // job information
    string pin_pkg;
    string customer;
    string pkg_id;
    int oper;

    // machine information
    string machine_no;
    string model_name;
    bool out;

    friend ostream &operator<<(
        ostream &out,
        const struct testing_machine_constraint_case_t &cs)
    {
        char _[200];
        sprintf(_, "(%s, %s, %s, %d, %s, %s, %s)", cs.pin_pkg.c_str(),
                cs.customer.c_str(), cs.pkg_id.c_str(), cs.oper,
                cs.machine_no.c_str(), cs.model_name.c_str(),
                cs.out ? "true" : "false");
        return out << _ << endl;
    }
};



class test_machine_constraint_base_t : public testing::Test
{
protected:
    static machine_constraint_t *mcs_a;
    static machine_constraint_t *mcs_r;

public:
    static void SetUpTestSuite()
    {
        csv_t entity_limit("test_data/ent_limit.csv", "r", true, true);
        csv_t constraint_a = entity_limit.filter("el_action", "A");
        csv_t constraint_r = entity_limit.filter("el_action", "R");
        if (mcs_a == nullptr) {
            mcs_a = new machine_constraint_a_t(constraint_a);
        }

        if (mcs_r == nullptr) {
            mcs_r = new machine_constraint_r_t(constraint_r);
        }
    }
    static void TearDownTestSuite()
    {
        if (mcs_a != nullptr) {
            delete mcs_a;
            mcs_a = nullptr;
        }

        if (mcs_r != nullptr) {
            delete mcs_r;
            mcs_r = nullptr;
        }
    }
};

class test_machine_constraint_suite_t
    : public test_machine_constraint_base_t,
      public testing::WithParamInterface<testing_machine_constraint_case_t>
{
protected:
    job_t *j;
    machine_t *m;

    void SetUp()
    {
        j = new job_t();
        m = new machine_t();
    }

    void TearDown()
    {
        delete j;
        delete m;
    }
    void init_job(testing_machine_constraint_case_t &cs)
    {
        j->pin_package = stringToInfo(cs.pin_pkg);
        j->pkg_id = stringToInfo(cs.pkg_id);
        j->customer = stringToInfo(cs.customer);
        j->oper = cs.oper;
    }
    void init_machine(testing_machine_constraint_case_t &cs)
    {
        m->base.machine_no = stringToInfo(cs.machine_no);
        m->model_name = stringToInfo(cs.model_name);
    }
};



#endif
