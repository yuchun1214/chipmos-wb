#include <cmath>
#include <exception>
#include <regex>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "include/entity.h"
#include "include/info.h"
#include "include/infra.h"
#include "include/job.h"
#include "include/job_base.h"
#include "include/lot.h"

#define X(item, value, name) name,
const char *ERROR_NAMES[] = {ERROR_TABLE};
#undef X

using namespace std;

static regex TIME_REGEX4(
    "\\d{4}-\\d{2}-\\d{2}\\ ([0-1][0-9]|2[0-3]):[0-5][0-9]");
static regex TIME_REGEX2(
    "\\d{2}-\\d{2}-\\d{2}\\ ([0-1][0-9]|2[0-3]):[0-5][0-9]");

lot_t::lot_t()
{
    _status = SUCCESS;
}

std::map<std::string, std::string> lot_t::rearrangeData(
    std::map<std::string, std::string> elements)
{
    elements["recipe"] =
        elements.count("recipe") == 1 ? elements["recipe"] : elements["bd_id"];

    if (elements.count("bd_id") == 1) {
        elements["recipe"] = elements.at("bd_id");
    } else if (elements.count("recipe") == 0) {
        elements["recipe"] = std::string("");
    }

    if (elements.count("queue_time") == 0) {
        elements["queue_time"] = std::string("0");
    }

    if (elements.count("fcst_time") == 0) {
        elements["fcst_time"] = std::string("0");
    }

    if (elements.count("dest_oper") == 0) {
        elements["dest_oper"] = elements["oper"];
    }

    if (elements.count("amount_of_tools") == 0) {
        elements["amount_of_tools"] = std::string("0");
    }

    if (elements.count("amount_of_wires") == 0) {
        elements["amount_of_wires"] = std::string("0");
    }

    if (elements.count("part_no") == 0) {
        elements["part_no"] = std::string("");
    }

    if (elements.count("part_id") == 0) {
        elements["part_id"] = std::string("");
    }

    if (elements.count("sub_lot") == 0) {
        elements["sub_lot"] = std::string("-1");
    }

    if (elements.count("qty") == 0) {
        elements["qty"] = std::string("0");
    }

    if (elements.count("oper") == 0 || elements["oper"].length() == 0) {
        elements["oper"] = std::string("0");
    }

    if (elements.count("last_WB_entity") == 0 ||
        elements["last_WB_entity"].length() == 0) {
        elements["last_WB_entity"] = "";
    }

    if (elements.count("package_id") == 0) {
        elements["package_id"] = "";
    }

    if (elements.count("CR") == 0 || elements["CR"].compare("0") == 0) {
        elements["CR"] = std::string("10000");
    }

    if (elements.count("super_hot_run_code") == 0) {
        elements["super_hot_run_code"] = "N";
    }

    if (elements.count("wlot_last_trans") == 0 ||
        elements["wlot_last_trans"].length() < 3) {
        elements["wlot_last_trans"] = "";
    } else {
        if (regex_search(elements["wlot_last_trans"], TIME_REGEX4)) {
            elements["wlot_last_trans"] = elements["wlot_last_trans"].substr(2);
        }
    }

    return elements;
}

bool lot_t::checkDataFormat(std::map<std::string, std::string> &elements,
                            std::string &log)
{
    // numeric data : queue_time, fcst_time,  dest_oper, amount_of_tools,
    // amount_of_wires, sub_lot, qty
    std::vector<std::string> numeric_keys = {
        "queue_time",      "fcst_time", "dest_oper", "amount_of_tools",
        "amount_of_wires", "sub_lot",   "qty"};

    std::vector<std::string> wrong_keys;
    for (auto key : numeric_keys) {
        if (elements.count(key) == 0) {
            continue;
        } else if (!isNumeric()(elements[key])) {
            wrong_keys.push_back(key);
            elements[key] = std::string("0");  // give the default value
        }
    }

    log += "wrong keys : ";
    log += join(wrong_keys, ",");

    if (wrong_keys.size())
        return false;
    return true;
}


lot_t::lot_t(std::map<std::string, std::string> elements) : _last_location("")
{
    _status = SUCCESS;

    elements = rearrangeData(elements);

    std::string lg;
    if (!checkDataFormat(elements, lg)) {
        addLog(lg, ERROR_BAD_DATA_FORMAT);
    }

    _route = elements["route"];
    _lot_number = elements["lot_number"];
    _pin_package = elements["pin_package"];
    _recipe = elements["recipe"];
    _prod_id = elements["prod_id"];
    _urgent = elements["urgent_code"];
    _customer = elements["customer"];
    _wb_location = elements["wb_location"];
    _pkg_id = elements["package_id"];
    _last_entity = tmp_oper == 2070 ? "DA" : elements["last_WB_entity"];

    if (_last_entity.compare("DA") == 0 &&
        elements["WB_move_in_entity"].length() != 0) {
        _last_entity = elements["WB_move_in_entity"];
    }

    _wlot_last_trans = elements["wlot_last_trans"];

    setHold(elements["hold"]);
    setMvin(elements["mvin"]);
    setAutomotive(elements["automotive"]);
    setSprHot(elements["super_hot_run_code"]);

    _cr = std::stod(elements["CR"]);
    _queue_time = std::stod(elements["queue_time"]);
    _cure_time = 0;
    _fcst_time = std::stod(elements["fcst_time"]);
    tmp_oper = stoi(elements["dest_oper"]);
    _sub_lots = std::stoi(elements["sub_lot"]);

    _amount_of_wires = stoi(elements["amount_of_wires"]);

    int _number_of_tools = stoi(elements["amount_of_tools"]);
    std::string _part_no = elements["part_no"];
    if (_part_no.length()) {
        setAmountOfTools(_part_no, _number_of_tools);
    }

    _qty = std::stoi(elements["qty"]);
    _oper = std::stoi(elements["oper"]);

    _outplan_time = 0;
    _finish_traversal = false;
    tmp_mvin = _mvin;

    _is_sub_lot = _lot_number.length() > 8 ? true : false;
    _part_id = elements["part_id"];


    if (elements.count("CAN_RUN_MODELS") != 0 &&
        elements.count("PROCESS_TIME") != 0 && elements.count("uphs")) {
        char *text = strdup(elements["CAN_RUN_MODELS"].c_str());
        std::vector<std::string> models = split(text, ',');
        free(text);
        text = strdup(elements["PROCESS_TIME"].c_str());
        std::vector<std::string> ptimes = split(text, ',');
        free(text);
        text = strdup(elements["uphs"].c_str());
        std::vector<std::string> uphs = split(text, ',');
        free(text);
        _can_run_models = models;
        if (models.size() != ptimes.size() || models.size() != uphs.size()) {
            throw std::invalid_argument(
                "vector size is not the same, lot_number : " + _lot_number);
        } else {
            foreach (models, i) {
                _model_process_times[models[i]] = std::stod(ptimes[i]);
                _uphs[models[i]] = std::stod(uphs[i]);
            }
        }
    }


    _prescheduled_order = -1;
    if (_wb_location.length() && _wb_location[0] != 'N') {
        char *text = strdup(_wb_location.c_str());
        std::vector<std::string> machine_order = split(text, '-');
        _prescheduled_machine = machine_order[0];

        if (machine_order.size() >= 2) {
            try {
                _prescheduled_order = stoi(machine_order[1]);
            } catch (std::invalid_argument &e) {
                std::cout << e.what() << std::endl;
            }
        } else {
            _prescheduled_order = 0;
        }
        free(text);
    }
}

bool lot_t::checkFormation()
{
    std::string error_msg;
    std::vector<std::string> data_members;

    if (_route.length() == 0)
        data_members.push_back("route");

    if (_lot_number.length() == 0)
        data_members.push_back("lot_number");

    if (_pin_package.length() == 0)
        data_members.push_back("pin_package");

    if (_recipe.length() == 0)
        data_members.push_back("recipe");

    if (_qty <= 0) {
        data_members.push_back("qty");
    }

    if (_pkg_id.length() == 0)
        data_members.push_back("pkg_id");

    if (data_members.size()) {
        error_msg = data_members.size() > 1 ? "These information, "
                                            : "This"
                                              " information, ";
        for (unsigned int i = 0; i < data_members.size(); ++i) {
            error_msg += data_members[i] + " ,";
        }
        error_msg += data_members.size() > 1 ? " are incorrect."
                                             : " is"
                                               " incorrect.";
        addLog(error_msg, ERROR_WIP_INFORMATION_LOSS);
        return false;
    }
    return true;
}

std::vector<lot_t> lot_t::createSublots()
{
    std::vector<lot_t> lots;
    if (_is_sub_lot) {
        return lots;
    }
    char str_number[100];
    int count = std::ceil((double) _qty / _lot_size);
    int remain = _qty;
    for (int i = 0; i < count; ++i) {
        sprintf(str_number, "%02d", i + 1);
        lot_t tmp(*this);
        tmp._lot_number += str_number;
        tmp._is_sub_lot = true;
        tmp.addLog("This lot is split from the parent lot " + _lot_number,
                   SUCCESS);
        if (remain - _lot_size > 0) {
            tmp._qty = _lot_size;
            remain -= tmp._qty;
        } else {
            tmp._qty = remain;
            remain = 0;
        }
        lots.push_back(tmp);
    }
    _qty = remain;

    return lots;
}

std::map<std::string, std::string> lot_t::data()
{
    std::map<std::string, std::string> d;
    d["route"] = _route;
    d["lot_number"] = _lot_number;
    d["pin_package"] = _pin_package;
    d["recipe"] = _recipe;
    d["prod_id"] = _prod_id;
    d["process_id"] = _process_id;
    d["bom_id"] = _bom_id;
    d["part_id"] = _part_id;
    d["all_part_no"] = join(__tools, ",");
    // d["part_no"] = join(__tools, ",");
    try {
        d["part_no"] = part_no();
    } catch (std::logic_error &e) {
        d["part_no"] = "";
    }

    d["qty"] = std::to_string(_qty);
    d["oper"] = std::to_string(_oper);
    d["dest_oper"] = std::to_string(tmp_oper);
    d["lot_size"] = std::to_string(_lot_size);
    d["amount_of_wires"] = std::to_string(_amount_of_wires);
    try {
        d["amount_of_tools"] = std::to_string(getAmountOfTools());
    } catch (std::logic_error &e) {
        d["amount_of_tools"] = "";
    }
    d["package_id"] = _pkg_id;
    d["hold"] = _hold ? "Y" : "N";
    d["mvin"] = _mvin ? "Y" : "N";
    d["is_sub_lot"] = _is_sub_lot ? "Y" : "N";
    d["automotive"] = _is_automotive ? "Y" : "N";
    d["queue_time"] = std::to_string(_queue_time);
    d["fcst_time"] = std::to_string(_fcst_time);
    d["log"] = join(_log, "||");
    d["urgent_code"] = _urgent;
    d["customer"] = _customer;
    d["wb_location"] = _wb_location;
    d["prescheduled_machine"] = _prescheduled_machine;
    d["prescheduled_order"] = std::to_string(_prescheduled_order);
    // d["code"] = std::string(ERROR_NAMES[_status]);
    std::vector<std::string> code;
    foreach (_statuses, i) {
        code.push_back(std::string(ERROR_NAMES[_statuses[i]]));
    }


    std::vector<std::string> process_times;
    std::vector<std::string> _can_run_models;
    for (std::map<std::string, double>::iterator it =
             _model_process_times.begin();
         it != _model_process_times.end(); it++) {
        process_times.push_back(std::to_string(it->second));
        _can_run_models.push_back(it->first);
    }

    std::vector<std::string> uphs;
    for (std::map<std::string, double>::iterator it = _uphs.begin();
         it != _uphs.end(); it++) {
        uphs.push_back(std::to_string(it->second));
    }

    d["ALL_CAN_RUN_MODELS"] = join(_all_models, ",");
    d["CAN_RUN_MODELS"] = join(_can_run_models, ",");
    d["PROCESS_TIME"] = join(process_times, ",");
    d["uphs"] = join(uphs, ",");
    d["code"] = join(code, ",");

    return d;
}

bool lot_t::setUph(csv_t &original_uph_csv)
{
    csv_t _uph_csv;
    _uph_csv = original_uph_csv.filter("recipe", _recipe);
    _uph_csv = _uph_csv.filter("oper", std::to_string(this->tmp_oper));
    _uph_csv = _uph_csv.filter("cust", _customer);
    if (_uph_csv.nrows() == 0) {
        this->addLog("(" + std::to_string(this->tmp_oper) + ", " + _recipe +
                         ") is not in uph file",
                     ERROR_UPH_FILE_ERROR);
        return false;
    } else {
        int nrows = _uph_csv.nrows();
        std::map<std::string, double> uphs;

        for (int i = 0; i < nrows; ++i) {
            std::map<std::string, std::string> elements =
                _uph_csv.getElements(i);
            double value = std::stof(elements["uph"]);
            if (value - 0.0 > 0.00000001) {
                uphs[elements["model"]] = value;
            }
        }

        for (std::map<std::string, double>::iterator it = uphs.begin();
             it != uphs.end(); it++) {
            if (_uphs.count(it->first) != 0) {
                setUph(it->first, it->second);
            }
        }
    }
    std::vector<std::string> invalid_models;
    std::vector<std::string> valid_models;
    for (std::map<std::string, double>::iterator it = _uphs.begin();
         it != _uphs.end(); it++) {
        if (it->second == 0) {
            invalid_models.push_back(it->first);
        } else {
            valid_models.push_back(it->first);
        }
    }

    foreach (invalid_models, i) {
        _uphs.erase(invalid_models[i]);
        _model_process_times.erase(invalid_models[i]);
    }

    _can_run_models = valid_models;

    if (_uphs.size() == 0) {
        addLog("All of uph are 0", ERROR_UPH_0);
        return false;
    } else
        return true;
}


void lot_t::setCanRunLocation(
    std::map<std::string, std::vector<std::string> > model_locations)
{
    foreach (_can_run_models, i) {
        std::vector<std::string> locations =
            model_locations[_can_run_models[i]];
        if (_pin_package.find("DOU") != std::string::npos) {
            foreach (locations, j) {
                if (locations[j].compare("TA-P") == 0 ||
                    locations[j].compare("TA-U") == 0) {
                    if (_pin_package.find("DFN") != std::string::npos ||
                        _pin_package.find("QFN") != std::string::npos ||
                        _part_id[4] != 'A') {
                        continue;
                    } else {
                        _can_run_locations.push_back(locations[j]);
                    }
                } else if (locations[j].compare("TA-R") == 0) {
                    if (_pin_package.find("DFN") != std::string::npos ||
                        _pin_package.find("QFN") != std::string::npos ||
                        (_pin_package.find("FPS") != std::string::npos &&
                         _pin_package.back() == 'V') ||
                        _part_id[4] != 'A') {
                        continue;
                    } else {
                        _can_run_locations.push_back(locations[j]);
                    }
                } else if (locations[j].find("TA") != std::string::npos)
                    _can_run_locations.push_back(locations[j]);
            }
        } else if (_pin_package.find("uSD") != std::string::npos) {
            foreach (locations, j) {
                if (locations[j].compare("TB-5P") == 0) {
                    if (_pin_package.find("FBGA") != std::string::npos &&
                        _part_id[4] == 'A') {
                        _can_run_locations.push_back(locations[j]);
                    } else {
                        continue;
                    }
                } else if (locations[j].compare("TB-P") == 0 ||
                           (locations[j].compare("TB-R") == 0) ||
                           (locations[j].compare("TB-U") == 0)) {
                    if ((_pin_package.find("TSOP1") != std::string::npos ||
                         _pin_package.find("TSOP2") != std::string::npos) &&
                        _part_id[4] == 'A') {
                        continue;
                    } else {
                        _can_run_locations.push_back(locations[j]);
                    }
                } else if (locations[j].find("TB") != std::string::npos) {
                    _can_run_locations.push_back(locations[j]);
                }
            }
        } else {
            foreach (locations, j) {
                if (locations[j].compare("TA-P") == 0 ||
                    locations[j].compare("TA-U") == 0) {
                    if (_pin_package.find("DFN") != std::string::npos ||
                        _pin_package.find("QFN") != std::string::npos ||
                        _part_id[4] != 'A') {
                        continue;
                    } else {
                        _can_run_locations.push_back(locations[j]);
                    }
                } else if (locations[j].compare("TA-R") == 0) {
                    if (_pin_package.find("DFN") != std::string::npos ||
                        _pin_package.find("QFN") != std::string::npos ||
                        (_pin_package.find("FPS") != std::string::npos &&
                         _pin_package.back() == 'V') ||
                        _part_id[4] != 'A') {
                        continue;
                    } else {
                        _can_run_locations.push_back(locations[j]);
                    }
                } else if (locations[j].compare("TB-5P") == 0) {
                    if (_pin_package.find("FBGA") != std::string::npos &&
                        _part_id[4] == 'A') {
                        _can_run_locations.push_back(locations[j]);
                    } else {
                        continue;
                    }
                } else if (locations[j].compare("TB-P") == 0 ||
                           (locations[j].compare("TB-R") == 0) ||
                           (locations[j].compare("TB-U") == 0)) {
                    if ((_pin_package.find("TSOP1") != std::string::npos ||
                         _pin_package.find("TSOP2") != std::string::npos) &&
                        _part_id[4] == 'A') {
                        continue;
                    } else {
                        _can_run_locations.push_back(locations[j]);
                    }
                } else {
                    _can_run_locations.push_back(locations[j]);
                }
            }
        }
    }
}

bool lot_t::isEntitySuitable(std::string location)
{
    // Edit in 2021/8/21
    // In previous version, the code check for model first and than check the
    // location Actually, Model spreads in several locations but there is only
    // one kind of model in a location. So, we just need to check the entity's
    // location to determine if the entity can run or not;
    if (find(_can_run_locations.begin(), _can_run_locations.end(), location) !=
        _can_run_locations.end()) {
        return true;
    }
    return false;
}


job_t lot_t::job()
{
    job_t j;
    job_base_init(&j.base);
    _list_init(&j.list);
    j.is_scheduled = false;
    j.spr_hot = _spr_hot;

    j.base.job_info = stringToInfo(_lot_number);

    j.part_no = stringToInfo(part_no());
    j.part_id = stringToInfo(_part_id);
    j.prod_id = stringToInfo(_prod_id);
    j.pkg_id = stringToInfo(_pkg_id);
    j.location = stringToInfo(_last_location);

    j.pin_package = stringToInfo(_pin_package);
    j.customer = stringToInfo(_customer);
    j.bdid = stringToInfo(_recipe);

    j.oper = tmp_oper;
    float lot_order;
    try {
        std::string seq = _lot_number.substr(_lot_number.length() - 2);
        int n1, n2;
        std::sscanf(seq.c_str(), "%1x%d", &n1, &n2);
        lot_order = n1 * 10 + n2;
        lot_order /= (float) _sub_lots;
    } catch (std::invalid_argument &e) {
        std::cout << e.what() << std::endl;
        lot_order = 1;
    } catch (std::out_of_range &e) {
        lot_order = 1;
    }


    if (_urgent.length()) {
        j.urgent_code = _urgent[0];
    } else
        j.urgent_code = '\0';

    j.cr = _cr;
    j.base.qty = _qty;
    j.base.start_time = j.base.end_time = 0;
    j.base.arriv_t = _queue_time;


    if (isPrescheduled()) {
        j.list.get_value = prescheduledJobGetValue;
        j.base.machine_no = stringToInfo(_prescheduled_machine);
        j.weight = _prescheduled_order;
        try {
            j.base.ptime = _model_process_times.at(_prescheduled_model);
        } catch (std::out_of_range &e) {
            // #ifdef LOG_ERROR
            //             std::cerr << "Warning : Attempt to create job without
            //             setting "
            //                          "prescheduled model. The ptime will be
            //                          set as -1"
            //                       << std::endl;
            // #endif
            j.base.ptime = _model_process_times.begin()->second;
        }
    } else {
        j.list.get_value = jobGetValue;
        j.base.machine_no = emptyInfo();
        j.weight = lot_order;
        j.base.ptime = 0.0;
    }

    if (_model_process_times.size() == 1)
        j.base.ptime = _model_process_times.begin()->second;

    return j;
}

bool lot_t::isLotOkay()
{
    bool ret = true;

    // check data from wip
    //  1. check lot number is sub lot
    ret &= isSubLot();

    // check hold
    ret ^= _hold;

    // check statuses
    ret &= (_statuses.size() == 0);

    return ret;
}
