#include <ctime>
#include <exception>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <system_error>

#include <include/arrival.h>
#include <include/common.h>
#include <include/condition_card.h>
#include <include/csv.h>
#include <include/da.h>
#include <include/job.h>
#include <include/route.h>


using namespace std;



int main(int argc, const char *argv[])
{
    vector<lot_t> lots =
        createLots("WipOutPlanTime_.csv", "product_find_process_id.csv",
                   "process_find_lot_size_and_entity.csv", "fcst.csv",
                   "routelist.csv", "newqueue_time.csv");
    csv_t out("out.csv", "w");
    iter(lots, i){
        out.addData(lots[i].data());
    } 
    out.write();

    return 0;
}
