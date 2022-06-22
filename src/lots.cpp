#include <cstdlib>
#include <ctime>
#include <exception>
#include <ios>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <stdexcept>
#include <string>

#include "include/condition_card.h"
#include "include/csv.h"
#include "include/da.h"
#include "include/infra.h"
#include "include/lot.h"
#include "include/lots.h"
#include "include/route.h"

#ifdef WIN32
#include <direct.h>
#endif

using namespace std;

lots_t::lots_t()
{
    // the _traversing_functions' order is fixed which means that
    // you can't randomly change the order of content
    // the order is deremined by the enum TRAVERSE_STATUS

    // the concept is each bit of the return value of calculateQueueTime
    // maps to a unique function.
    _traversing_functions = {
        &lots_t::_traversingError, &lots_t::_traversingFinished,
        &lots_t::_traversingDAArrived, &lots_t::_traversingDAUnarrived,
        &lots_t::_traversingDADecrement};
    _total_number_of_wip = 0;
}


void lots_t::pushBackNotPrescheduledLot(lot_t *lot)
{
    if (lot->isPrescheduled()) {
        lot->setNotPrescheduled();
    }

    this->lots.push_back(lot);
    std::string part_id, part_no;
    part_id = lot->part_id();
    part_no = lot->part_no();
    this->tool_lots[part_no].push_back(lot);
    this->wire_lots[part_id].push_back(lot);
    this->tool_wire_lots[part_no + "_" + part_id].push_back(lot);

    if (amount_of_tools.empty() || amount_of_wires.empty()) {
        amount_of_tools[part_no] = lot->getAmountOfTools();
        amount_of_wires[part_id] = lot->getAmountOfWires();
    }
}

void lots_t::addLots(std::vector<lot_t *> lots)
{
    std::string part_id, part_no;
    bool reset_tool_and_wire =
        amount_of_tools.empty() || amount_of_wires.empty();
    foreach (lots, i) {
        if (lots[i]->isPrescheduled()) {
            this->prescheduled_lots.push_back(lots[i]);
        } else {
            this->lots.push_back(lots[i]);
        }
        part_id = lots[i]->part_id();
        part_no = lots[i]->part_no();
        if (reset_tool_and_wire) {
            amount_of_tools[part_no] = lots[i]->getAmountOfTools();
            amount_of_wires[part_id] = lots[i]->getAmountOfWires();
        }
    }

    foreach (this->lots, i) {
        part_id = this->lots[i]->part_id();
        part_no = this->lots[i]->part_no();
        this->tool_lots[part_no].push_back(this->lots[i]);
        this->wire_lots[part_id].push_back(this->lots[i]);
        this->tool_wire_lots[part_no + "_" + part_id].push_back(this->lots[i]);
    }
}

void lots_t::readLocation(string filename,
                          vector<lot_t> &lots,
                          vector<lot_t> &faulty_lots)
{
    csv_t location(filename, "r", true, true);
    location.dropNullRow();
    location.trim(" ");
    map<std::string, std::string> loc;

    for (int i = 0; i < location.nrows(); ++i)
        if (loc.count(location.getElements(i)["Entity"]) == 0)
            loc.insert(pair<std::string, std::string>(
                location.getElements(i)["Entity"],
                location.getElements(i)["Location"]));

    foreach (lots, i) {
        if ("DA" == lots[i].getLastEntity())
            lots[i].setLastLocation("TA-4F");
        else if (!lots[i].getLastEntity().empty())
            // TODO exception last entity not found
            lots[i].setLastLocation(loc.at(lots[i].getLastEntity()));
    }

    return;
}

void lots_t::readWip(string filename,
                     vector<lot_t> &lots,
                     vector<lot_t> &faulty_lots)
{
    // setup wip
    // Step 1 : read wip.csv
    csv_t wip(filename, "r", true, true);
    wip.dropNullRow();
    wip.trim(" ");
    wip.setHeaders(map<string, string>({{"lot_number", "wlot_lot_number"},
                                        {"qty", "wlot_qty_1"},
                                        {"hold", "wlot_hold"},
                                        {"oper", "wlot_oper"},
                                        {"mvin", "wlot_mvin_perfmd"},
                                        {"recipe", "bd_id"},
                                        {"prod_id", "wlot_prod"},
                                        {"urgent_code", "urgent_code"},
                                        {"customer", "wlot_crt_dat_al_1"}}));
    lot_t lot_tmp;
    for (unsigned int i = 0, size = wip.nrows(); i < size; ++i) {
        lot_tmp = lot_t(wip.getElements(i));
        if (lot_tmp.checkFormation()) {
            lots.push_back(lot_tmp);
        } else {
            faulty_lots.push_back(lot_tmp);
        }

        if (lot_tmp.isAutomotive()) {
            _automotive_lot_numbers.insert(lot_tmp.lotNumber());
        }
    }
    _total_number_of_wip = wip.nrows();

    foreach (lots, i) {
        lots[i].setDAQueueTime(_base_time);
        lots[i].addQueueTime((double) lots[i].da_queue_time(), "DA Queue Time");
    }
}

void lots_t::setPidBomId(string filename,
                         vector<lot_t> &lots,
                         vector<lot_t> &faulty_lots)
{
    // Step 2 : read product_find_process_id
    // csv_t prod_pid_mapping("product_find_process_id.csv", "r", true, true);
    csv_t prod_pid_mapping(filename, "r", true, true);
    prod_pid_mapping.trim(" ");
    prod_pid_mapping.setHeaders(
        map<string, string>({{"prod_id", "product"},
                             {"process_id", "process_id"},
                             {"bom_id", "bom_id"}}));
    map<string, string> prod_pid;
    map<string, string> prod_bom;
    for (unsigned int i = 0; i < prod_pid_mapping.nrows(); ++i) {
        map<string, string> tmp = prod_pid_mapping.getElements(i);
        prod_pid[tmp["prod_id"]] = tmp["process_id"];
        prod_bom[tmp["prod_id"]] = tmp["bom_id"];
    }

    vector<lot_t> result;

    string err_msg;

    for (unsigned int i = 0; i < lots.size(); i++) {
        try {
            string process_id = prod_pid.at(lots[i].prodId());
            lots[i].setProcessId(process_id);
        } catch (std::out_of_range &e) {
            err_msg = "Lot Entry " + to_string(i + 2) + ": " +
                      lots[i].lotNumber() +
                      " has no mapping relationship between its product id(" +
                      lots[i].prodId() + ") and its process_id";
            lots[i].addLog(err_msg, ERROR_PROCESS_ID);
            faulty_lots.push_back(lots[i]);
            continue;
        }

        try {
            string bom_id = prod_bom.at(lots[i].prodId());
            lots[i].setBomId(bom_id);
        } catch (std::out_of_range &e) {
            err_msg = "Lot Entry" + to_string(i + 2) + ": " +
                      lots[i].lotNumber() +
                      "has no mapping relation between its product id(" +
                      lots[i].prodId() + ") and its bom_id";
            lots[i].addLog(err_msg, ERROR_BOM_ID);
            faulty_lots.push_back(lots[i]);
            continue;
        }

        result.push_back(lots[i]);
    }

    lots = result;
}

void lots_t::setLotSize(string filename,
                        vector<lot_t> &lots,
                        vector<lot_t> &faulty_lots)
{
    // Step 3 : sublot_size;
    // TODO : read sublot size
    // csv_t eim("process_find_lot_size_and_entity.csv", "r", true, true);
    csv_t eim(filename, "r", true, true);
    eim.trim(" ");
    eim.setHeaders(map<string, string>({{"process_id", "process_id"},
                                        {"desc", "master_desc"},
                                        {"lot_size", "remark"}}));
    csv_t pid_lotsize_df;
    pid_lotsize_df = eim.filter("desc", "Lot Size");
    map<string, int> pid_lotsize;
    for (unsigned int i = 0; i < pid_lotsize_df.nrows(); ++i) {
        map<string, string> tmp = pid_lotsize_df.getElements(i);
        pid_lotsize[tmp["process_id"]] = stoi(tmp["lot_size"]);
    }

    vector<lot_t> result;
    int lot_size;
    string err_msg;
    foreach (lots, i) {
        try {
            lot_size = pid_lotsize.at(lots[i].processId());
            lots[i].setLotSize(lot_size);
        } catch (std::out_of_range &e) {  // catch map::at
            err_msg =
                ("Lot Entry" + to_string(i + 2) + ": " + lots[i].lotNumber() +
                 "has no mapping relation between its process id(" +
                 lots[i].processId() + ") and its lot_size");
            lots[i].addLog(err_msg, ERROR_LOT_SIZE);
            faulty_lots.push_back(lots[i]);
            continue;
        } catch (std::invalid_argument &e) {
            err_msg = "Lot Entry" + to_string(i + 2) + ": " +
                      lots[i].lotNumber() + "the lot_size of process id(" +
                      lots[i].processId() + ") is" + to_string(lot_size) +
                      " which is less than 0";
            lots[i].addLog(err_msg, ERROR_INVALID_LOT_SIZE);
            faulty_lots.push_back(lots[i]);
            continue;
        }
        result.push_back(lots[i]);
    }
    lots = result;
}

void lots_t::setupRoute(std::string routelist,
                        std::string queuetime,
                        std::string eim_lot_size,
                        std::string cure_time,
                        route_t &routes)
{
    // setup routes
    csv_t routelist_df(routelist, "r", true, true);
    csv_t queue_time(queuetime, "r", true, true);
    csv_t eim_ptn(eim_lot_size, "r", true, true);
    csv_t cure_time_df(cure_time, "r", true, true);

    routelist_df.trim(" ");
    routelist_df.setHeaders(
        map<string, string>({{"route", "wrto_route"},
                             {"oper", "wrto_oper"},
                             {"seq", "wrto_seq_num"},
                             {"desc", "wrto_opr_shrt_desc"}}));
    routes.setQueueTime(queue_time);
    routes.setCureTime(eim_ptn, cure_time_df);

    vector<vector<string> > data = routelist_df.getData();
    vector<string> routenames = routelist_df.getColumn("route");
    set<string> routelist_set(routenames.begin(), routenames.end());
    routenames = vector<string>(routelist_set.begin(), routelist_set.end());

    // setRoute -> get wb - 7
    csv_t df;
    foreach (routenames, i) {
        df = routelist_df.filter("route", routenames[i]);
        routes.setRoute(routenames[i], df);
    }
}

vector<lot_t> lots_t::wb7Filter(vector<lot_t> alllots,
                                vector<lot_t> &dontcare,
                                route_t &routes)
{
    vector<lot_t> lots;
    foreach (alllots, i) {
        if (alllots[i].hold()) {
            alllots[i].addLog("Lot is hold", ERROR_HOLD);
            dontcare.push_back(alllots[i]);
        } else if (!routes.isLotInStations(alllots[i])) {
            alllots[i].addLog("Lot is not in WB - 7", ERROR_WB7);
            dontcare.push_back(alllots[i]);
        } else {
            lots.push_back(alllots[i]);
        }
    }
    return lots;
}

void lots_t::_traversingError(lot_t &lot,
                              std::vector<lot_t> &unfinished,
                              std::vector<lot_t> &finished,
                              vector<lot_t> &faulty_lot,
                              da_stations_t &das)
{
    string err_msg(
        "Error occures on routes.calculateQueueTime, the "
        "reason is the lot can't reach W/B satation.");

    lot.addLog(err_msg, ERROR_WB7);
}

void lots_t::_traversingFinished(lot_t &lot,
                                 std::vector<lot_t> &unfinished,
                                 std::vector<lot_t> &finished,
                                 vector<lot_t> &faulty_lot,
                                 da_stations_t &das)
{
    lot.addLog("Lot finishes traversing the route", SUCCESS);
    finished.push_back(lot);
}

void lots_t::_traversingDAArrived(lot_t &lot,
                                  std::vector<lot_t> &unfinished,
                                  std::vector<lot_t> &finished,
                                  vector<lot_t> &faulty_lot,
                                  da_stations_t &das)
{
    lot.addLog(
        "Lot is waiting on DA station, it is cataloged to "
        "arrived",
        SUCCESS);
    das.addArrivedLotToDA(lot);
}

void lots_t::_traversingDAUnarrived(lot_t &lot,
                                    std::vector<lot_t> &unfinished,
                                    std::vector<lot_t> &finished,
                                    vector<lot_t> &faulty_lot,
                                    da_stations_t &das)
{
    lot.addLog(
        "Lot traverses to DA station, it is cataloged to "
        "unarrived",
        SUCCESS);
    das.addUnarrivedLotToDA(lot);
}

void lots_t::_traversingDADecrement(lot_t &lot,
                                    std::vector<lot_t> &unfinished,
                                    std::vector<lot_t> &finished,
                                    vector<lot_t> &faulty_lot,
                                    da_stations_t &das)
{
    das.decrementProductionCapacity(lot);
}


vector<lot_t> lots_t::queueTimeAndQueue(vector<lot_t> lots,
                                        vector<lot_t> &faulty_lots,
                                        vector<lot_t> &dontcare,
                                        da_stations_t &das,
                                        route_t &routes)
{
    int retval = 0;
    string err_msg;
    std::vector<lot_t> unfinished = lots;
    std::vector<lot_t> finished;
    int enum_size = TRAVERSE_STATUS_SIZE;
    while (unfinished.size()) {
        foreach (unfinished, i) {
            try {
                retval = routes.calculateQueueTime(unfinished[i]);
                for (int j = 0; j < enum_size; ++j) {
                    if (check_bit(retval, j)) {
                        (this->*_traversing_functions[j])(unfinished[i],
                                                          unfinished, finished,
                                                          faulty_lots, das);
                    }
                }
            } catch (std::out_of_range
                         &e) {  // for da_stations_t function member,
                                // addArrivedLotToDA, addUnarrivedLotToDA,
                                // decrementProductionCapacity
                unfinished[i].addLog(e.what(), ERROR_DA_FCST_VALUE);
                faulty_lots.push_back(unfinished[i]);
            } catch (std::logic_error &e) {  // for calculateQueueTime
                char *text = strdup(e.what());
                vector<string> what = split(text, '|');
                unfinished[i].addLog(what[0],
                                     static_cast<ERROR_T>(stoi(what[1])));
                faulty_lots.push_back(unfinished[i]);
            } 
        }
        unfinished = das.distributeProductionCapacity();
        das.removeAllLots();
    }

    dontcare += das.getParentLots();
    dontcare += das.get_all_remaining_lots();
    _parent_lots_and_sublots = das.getParentLotAndSubLots();

    return finished;
}

void lots_t::setCanRunModels(string bdid_model_mapping_models_filename,
                             vector<lot_t> &lots,
                             vector<lot_t> &faulty_lots)
{
    condition_cards_h cards(12, "UTC1000", "UTC1000S", "UTC2000", "UTC2000S",
                            "UTC3000", "UTC5000S", "Maxum Base", "Maxum Plus",
                            "Maxum Ultra", "Iconn", "Iconn Plus", "RAPID");
    cards.addMapping("Maxum (Ultra)", 2, "Maxum", "Maxum-Ultra");
    cards.readBdIdModelsMappingFile(bdid_model_mapping_models_filename);
    // vector<lot_t> result;
    foreach (lots, i) {
        try {
            lots[i].setCanRunModels(
                cards.getModels(lots[i].recipe(), lots[i].tmp_oper).models);
            // result.push_back(lots[i]);
        } catch (out_of_range &e) {
            lots[i].addLog("Lot has no matched condition card",
                           ERROR_CONDITION_CARD);
            // faulty_lots.push_back(lots[i]);
        }
    }
    // lots = result;
}

void lots_t::setPartId(string filename,
                       vector<lot_t> &lots,
                       vector<lot_t> &faulty_lots)
{
    // filename = "BomList"
    csv_t bomlist(filename, "r", true, true);
    bomlist.trim(" ");
    bomlist.setHeaders(map<string, string>(
        {{"bom_id", "bom_id"}, {"oper", "oper"}, {"part_id", "part_id"}}));
    map<int, map<string, string> > bom_oper_part;
    for (unsigned int i = 0; i < bomlist.nrows(); ++i) {
        map<string, string> tmp = bomlist.getElements(i);
        // if "oper" is WB, then get its part_id.
        std::set<int> opers;
        for (int i = 0, size = NUMBER_OF_WB_STATIONS; i < size; ++i) {
            opers.insert(WB_STATIONS[i]);
        }
        int oper_int = stoi(tmp["oper"]);
        if (opers.count(oper_int) != 0) {
            bom_oper_part[oper_int][tmp["bom_id"]] = tmp["part_id"];
        }
    }

    string err_msg;
    foreach (lots, i) {
        try {
            string part_id =
                bom_oper_part.at(lots[i].tmp_oper).at(lots[i].bomId());
            lots[i].setPartId(part_id);
        } catch (std::out_of_range &e) {
            err_msg = "Lot Entry " + to_string(i + 2) + ": " +
                      lots[i].lotNumber() +
                      " has no mapping relationship between its oper :" +
                      to_string(lots[i].tmp_oper) + " bom id(" +
                      lots[i].bomId() + ") and its part_id";
            lots[i].addLog(err_msg, ERROR_PART_ID);
            // faulty_lots.push_back(lots[i]);
            continue;
        }

        // result.push_back(lots[i]);
    }

    // lots = result;
}

void lots_t::setAmountOfWire(string gw_filename,
                             string wire_stock_filename,
                             vector<lot_t> &lots,
                             vector<lot_t> &faulty_lots)
{
    // filename = "GW Inventory.csv"
    csv_t gw(gw_filename, "r", true, true);
    gw.trim(" ");
    gw.setHeaders(map<string, string>(
        {{"gw_part_no", "gw_part_no"}, {"roll_length", "roll_length"}}));
    map<string, int> &part_roll = amount_of_wires;
    for (unsigned int i = 0; i < gw.nrows(); ++i) {
        map<string, string> tmp = gw.getElements(i);
        if (tmp["code_flag"].compare("A") == 0 ||
            tmp["code_flag"].compare("N") == 0 ||
            tmp["code_flag"].compare("O") == 0 ||
            tmp["code_flag"].compare("R") == 0) {
            if (part_roll.count(tmp["gw_part_no"]) == 0) {
                part_roll[tmp["gw_part_no"]] = 0;
            }

            if (tmp["gw_part_no"][4] ==
                    'A' &&  // gw_part_no[4] is 'A' --> which is golden wire
                stod(tmp["roll_length"]) >= 500.0) {
                part_roll[tmp["gw_part_no"]] += 1;
            } else if (tmp["gw_part_no"][4] != 'A' &&
                       stod(tmp["roll_length"]) >= 200.0) {
                part_roll[tmp["gw_part_no"]] += 1;
            }
        }
    }

    csv_t wire_stock(wire_stock_filename, "r", true, true);
    wire_stock.trim(" ");
    for (unsigned int i = 0; i < wire_stock.nrows(); ++i) {
        map<string, string> elements = wire_stock.getElements(i);
        if (part_roll.count(elements["MATNR"]) == 0) {
            part_roll[elements["MATNR"]] = 0;
        }

        if (elements["MATNR"][4] == 'A' && stod(elements["CLABS"]) >= 500) {
            part_roll[elements["MATNR"]] += 1;
        } else if (elements["MATNR"][4] != 'A' &&
                   stod(elements["CLABS"]) >= 200) {
            part_roll[elements["MATNR"]] += 1;
        }
    }

    // vector<lot_t> result;

    string err_msg;

    foreach (lots, i) {
        if (lots[i].part_id().length() == 0) {
            lots[i].addLog(
                "Cannot get the number of wires due to empty part_id",
                ERROR_NO_WIRE);
            continue;
        }

        try {
            int amountOfWires = part_roll.at(lots[i].part_id());
            if (amountOfWires > 0) {
                lots[i].setAmountOfWires(amountOfWires);
                // result.push_back(lots[i]);
            } else {
                lots[i].addLog("There is no wire.", ERROR_NO_WIRE);
                // faulty_lots.push_back(lots[i]);
            }
        } catch (std::out_of_range &e) {
            err_msg = "Lot Entry " + to_string(i + 2) + ": " +
                      lots[i].lotNumber() +
                      " has no mapping relationship between its part id(" +
                      lots[i].part_id() + ") and its roll_length";
            lots[i].addLog(err_msg, ERROR_NO_WIRE);
            // faulty_lots.push_back(lots[i]);
        }
    }

    // lots = result;
}

void lots_t::setPartNo(string filename,
                       vector<lot_t> &lots,
                       vector<lot_t> &faulty_lots)
{
    // filename = "Process find heatblock.csv"
    csv_t heatblock(filename, "r", true, true);
    heatblock.trim(" ");
    heatblock.setHeaders(map<string, string>(
        {{"process_id", "process_id"}, {"remark", "remark"}}));
    map<string, vector<string> > pid_part_numbers;
    for (unsigned int i = 0; i < heatblock.nrows(); ++i) {
        map<string, string> row = heatblock.getElements(i);
        // get substring of remark from first to "("
        string part_no = row["remark"];
        string process_id = row["process_id"];
        if (part_no[0] == 'A') {  // if remark == "Compression Mold" or "O/S
            // Xray檢驗___/20ea", then it shouldn't be used.
            part_no = part_no.substr(0, part_no.find(" "));
            if (part_no.find("(") != std::string::npos) {
                part_no = part_no.substr(0, part_no.find("("));
            }
            if (pid_part_numbers.count(process_id) == 0) {
                pid_part_numbers[process_id] = vector<string>();
            }
            pid_part_numbers[process_id].push_back(
                part_no.substr(0, part_no.find(" ")));
        }
    }

    // vector<lot_t> result;

    string err_msg;

    foreach (lots, i) {
        try {
            vector<string> part_numbers =
                pid_part_numbers.at(lots[i].processId());
            foreach (part_numbers, j)
                lots[i].setPartNo(part_numbers[j]);

        } catch (std::out_of_range &e) {
            err_msg = "Lot Entry " + to_string(i + 2) + ": " +
                      lots[i].lotNumber() +
                      " has no mapping relationship between its process id(" +
                      lots[i].processId() + ") and its remark";
            lots[i].addLog(err_msg, ERROR_PART_NO);
        }
    }
}

void lots_t::setAmountOfTools(string filename,
                              vector<lot_t> &lots,
                              vector<lot_t> &faulty_lots)
{
    // filename = "EMS Heatblock data.csv"
    csv_t ems(filename, "r", true, true);
    ems.trim(" ");
    ems.setHeaders(map<string, string>(
        {{"part_no", "part_no"}, {"qty1", "qty1"}, {"qty3", "qty3"}}));
    map<string, int> &pno_qty = amount_of_tools;
    for (unsigned int i = 0; i < ems.nrows(); ++i) {
        map<string, string> tmp = ems.getElements(i);
        string part_no = tmp["part_no"];
        int qty1_int = stoi(tmp["qty1"]);
        int qty3_int = stoi(tmp["qty3"]);
        pno_qty[part_no] = (qty1_int <= qty3_int) ? qty1_int : qty3_int;
    }

    string err_msg;
    int amount_of_tools;
    foreach (lots, i) {
        if (lots[i].part_no().length() == 0) {
            lots[i].addLog(
                "Cannot get the number of tools due to empty part_no",
                ERROR_NO_TOOL);
            continue;
        }
        amount_of_tools = lots[i].setAmountOfTools(pno_qty);
        if (amount_of_tools == 0)
            lots[i].addLog("There is no tool for this lot", ERROR_NO_TOOL);
    }

    return;
}

void lots_t::setUph(string uph_file_name,
                    vector<lot_t> &lots,
                    vector<lot_t> &faulty_lots)
{
    csv_t uph_csv(uph_file_name, "r", true, true);
    uph_csv.trim(" ");
    uph_csv.setHeaders(map<string, string>({{"oper", "OPER"},
                                            {"cust", "CUST"},
                                            {"recipe", "B/D#"},
                                            {"model", "MODEL"},
                                            {"uph", "G.UPH"}}));
    bool retval = 0;
    vector<lot_t> temp;
    vector<lot_t> maybe_faulty;
    // vector<lot_t> result;
    foreach (lots, i) {
        retval = lots[i].setUph(uph_csv);
    }
}

void lots_t::createLots(map<string, string> files)
{
    vector<lot_t *> lts;
    _base_time = files["std_time"];
    lts = createLots(
        files["wip"], files["pid_bomid"], files["lot_size"], files["fcst"],
        files["routelist"], files["queue_time"], files["bom_list"],
        files["pid_heatblock"], files["ems_heatblock"], files["gw_inventory"],
        files["wire_stock"], files["bdid_model_mapping"], files["uph"],
        files["cure_time"], files["locations"], files["no"]);
    string direcory_name = "output_" + files["no"];
    csv_t cfg(direcory_name + "/config.csv", "w");
    cfg.addData(files);
    cfg.write();
    addLots(lts);
}

std::vector<lot_t *> lots_t::createLots(
    std::string wip_file_name,
    std::string prod_pid_bomid_filename,
    std::string eim_lot_size_filename,
    std::string fcst_filename,
    std::string routelist_filename,
    std::string queue_time_filename,
    std::string bomlist_filename,
    std::string pid_heatblock_filename,
    std::string ems_heatblock_filename,
    std::string gw_filename,
    std::string wire_stock_filename,
    std::string bdid_mapping_models_filename,
    std::string uph_filename,
    std::string cure_time_filename,
    std::string location_filename,
    std::string dir_suffix)
{
    string err_msg;

    std::vector<lot_t> alllots;
    std::vector<lot_t> faulty_lots;
    std::vector<lot_t> lots;
    vector<lot_t> dontcare;

    // setup da_stations_t
    csv_t fcst(fcst_filename, "r", true, true);
    fcst.trim(" ");
    da_stations_t das(fcst);

    route_t routes;
    setupRoute(routelist_filename, queue_time_filename, eim_lot_size_filename,
               cure_time_filename, routes);

    // start creating lots
    readWip(wip_file_name, alllots, faulty_lots);

    lots = wb7Filter(
        alllots, dontcare,
        routes);  // check each lot if it is in scheduling plan (WB - 7)

    setPidBomId(prod_pid_bomid_filename, lots, faulty_lots);
    setLotSize(eim_lot_size_filename, lots, faulty_lots);

    // route traversal
    // sum the queue time and distribute the production capacity
    lots = queueTimeAndQueue(lots, faulty_lots, dontcare, das, routes);
    /*************************************************************************************/

    // setPartId
    setPartId(bomlist_filename, lots, faulty_lots);
    setAmountOfWire(gw_filename, wire_stock_filename, lots, faulty_lots);
    // setPartNo
    setPartNo(pid_heatblock_filename, lots, faulty_lots);
    setAmountOfTools(ems_heatblock_filename, lots, faulty_lots);

    setCanRunModels(bdid_mapping_models_filename, lots, faulty_lots);
    setUph(uph_filename, lots, faulty_lots);

    readLocation(location_filename, lots, faulty_lots);

    string directory_name = "output_" + dir_suffix;
    vector<lot_t> in_scheduling_plan_lots;
    foreach (lots, i) {
        bool is_in_scheduling_plan = lots[i].isInSchedulingPlan();
        bool isOkay = lots[i].isLotOkay();
        if (is_in_scheduling_plan && isOkay) {
            in_scheduling_plan_lots.push_back(lots[i]);
        } else if (!is_in_scheduling_plan) {
            lots[i].addLog("This lot is not in scheduling plan",
                           ERROR_NOT_IN_SCHEDULING_PLAN);
            dontcare.push_back(lots[i]);
        } else {
            faulty_lots.push_back(lots[i]);
        }
    }
    lots = in_scheduling_plan_lots;
#if defined(_WIN32)
    mkdir(directory_name.c_str());
#else
    mkdir(directory_name.c_str(),
          0777);  // notice that 777 is different than 0777
#endif

    // output faulty lots
    csv_t faulty_lots_csv(directory_name + "/faulty_lots.csv", "w");
    foreach (faulty_lots, i) {
        faulty_lots_csv.addData(faulty_lots[i].data());
    }
    faulty_lots_csv.write();
    // output dontcare lots
    csv_t dontcare_lots_csv(directory_name + "/dontcare.csv", "w");
    foreach (dontcare, i) {
        dontcare_lots_csv.addData(dontcare[i].data());
    }
    dontcare_lots_csv.write();

    _total_number_of_unscheduled_jobs = dontcare.size() + faulty_lots.size();

    // output lots
    csv_t lots_csv(directory_name + "/lots.csv", "w");
    foreach (lots, i) {
        lots_csv.addData(lots[i].data());
    }
    lots_csv.write();

    // ouput wip
    csv_t wip_csv(directory_name + "/out.csv", "w");
    foreach (faulty_lots, i) {
        wip_csv.addData(faulty_lots[i].data());
    }
    foreach (dontcare, i) {
        wip_csv.addData(dontcare[i].data());
    }
    foreach (lots, i) {
        wip_csv.addData(lots[i].data());
    }
    wip_csv.write();

    vector<lot_t *> lot_ptrs;
    foreach (lots, i) {
        lot_ptrs.push_back(new lot_t(lots[i]));
    }

    return lot_ptrs;
}

map<string, vector<lot_t *> > lots_t::getLotsRecipeGroups()
{
    map<string, vector<lot_t *> > groups;


    foreach (this->lots, i) {
        string recipe = this->lots[i]->recipe();
        string lot_number = this->lots[i]->lotNumber();
        if (groups.count(recipe) == 0)
            groups[recipe] = vector<lot_t *>();

        groups[recipe].push_back(this->lots[i]);
    }

    return groups;
}
