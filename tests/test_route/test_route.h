#pragma once
#include <gtest/gtest.h>

#define private public
#define protected public
#include "include/route.h"
#undef private
#undef protected

using namespace std;

namespace test_route
{
class test_route_base_t : public testing::Test
{
protected:
    static csv_t *__routelist;
    static csv_t *__queeu_time;
    static csv_t *__process_find_lot_size_and_entity;
    static csv_t *__cure_time;

public:
    static route_t *route;

    static void SetUpTestSuite()
    {
        if (__routelist == nullptr) {
            test_route_base_t::__routelist =
                new csv_t("test_data/route_list.csv", "r", true, true);
            test_route_base_t::__routelist->setHeaders(
                {{"route", "wrto_route"},
                 {"oper", "wrto_oper"},
                 {"seq", "wrto_seq_num"},
                 {"desc", "wrto_opr_shrt_desc"}});
        }

        if (__queeu_time == nullptr) {
            __queeu_time =
                new csv_t("test_data/queuetime.csv", "r", true, true);
        }

        if (__process_find_lot_size_and_entity == nullptr) {
            __process_find_lot_size_and_entity =
                new csv_t("test_data/process_find_lot_size_and_entity.csv", "r",
                          true, true);
        }

        if (__cure_time == nullptr) {
            __cure_time = new csv_t("test_data/cure_time.csv", "r", true, true);
        }

        if (route == nullptr) {
            route = new route_t();
            route->setRoute(*__routelist);
            route->setCureTime(*__process_find_lot_size_and_entity,
                               *__cure_time);
            route->setQueueTime(*__queeu_time);
        }
    }

    static void TearDownTestSuite()
    {
        if (__routelist != nullptr) {
            delete __routelist;
            __routelist = nullptr;
        }

        if (__queeu_time != nullptr) {
            delete __queeu_time;
            __queeu_time = nullptr;
        }

        if (__process_find_lot_size_and_entity != nullptr) {
            delete __process_find_lot_size_and_entity;
            __process_find_lot_size_and_entity = nullptr;
        }

        if (__cure_time != nullptr) {
            delete __cure_time;
            __cure_time = nullptr;
        }

        if (route != nullptr) {
            delete __routelist;
            __routelist = nullptr;
        }
    }
};



}  // namespace test_route
