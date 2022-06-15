#ifndef __LOT_H__
#define __LOT_H__

#include <algorithm>
#include <ctime>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "include/csv.h"
#include "include/infra.h"
#include "include/job.h"
#include "include/time_converter.h"

#define ERROR_TABLE                                                         \
    X(SUCCESS, = 0x00, "SUCCESS")                                           \
    X(ERROR_WIP_INFORMATION_LOSS, = 0x01, "ERROR_WIP_INFORMATION_LOSS")     \
    X(ERROR_PROCESS_ID, = 0x02, "ERROR_PROCESS_ID")                         \
    X(ERROR_BOM_ID, = 0x03, "ERROR_BOM_ID")                                 \
    X(ERROR_LOT_SIZE, = 0x04, "ERROR_LOT_SIZE")                             \
    X(ERROR_INVALID_LOT_SIZE, = 0x05, "ERROR_INVALID_LOT_SIZE")             \
    X(ERROR_DA_FCST_VALUE, = 0x06, "ERROR_DA_FCST_VALUE")                   \
    X(ERROR_INVALID_OPER_IN_ROUTE, = 0x07, "ERROR_INVALID_OPER_IN_ROUTE")   \
    X(ERROR_INVALID_QUEUE_TIME_COMBINATION, = 0x08,                         \
      "ERROR_INVALID_QUEUE_TIME_COMBINATION")                               \
    X(ERROR_HOLD, = 0x09, "ERROR_HOLD")                                     \
    X(ERROR_WB7, = 0x0A, "ERROR_WB7")                                       \
    X(ERROR_CONDITION_CARD, = 0x0B, "ERROR_CONDITION_CARD")                 \
    X(ERROR_PART_ID, = 0x0C, "ERROR_PART_ID")                               \
    X(ERROR_NO_WIRE, = 0x0D, "ERROR_NO_WIRE")                               \
    X(ERROR_WIRE_MAPPING_ERROR, = 0x0E, "ERROR_WIRE_MAPPING_ERROR")         \
    X(ERROR_PART_NO, = 0x0F, "ERROR_PART_NO")                               \
    X(ERROR_NO_TOOL, = 0x10, "ERROR_NO_TOOL")                               \
    X(ERROR_TOOL_MAPPING_ERROR, = 0x11, "ERROR_TOOL_MAPPING_ERROR")         \
    X(ERROR_UPH_FILE_ERROR, = 0x12, "ERROR_UPH_FILE")                       \
    X(ERROR_UPH_0, = 0x13, "ERROR_UPH_0")                                   \
    X(ERROR_NOT_IN_SCHEDULING_PLAN, = 0x14, "ERROR_NOT_IN_SCHEDULING_PLAN") \
    X(ERROR_BAD_DATA_FORMAT, = 0x15, "ERROR_BAD_DATA_FORMAT")


#define X(item, value, name) item value,
enum ERROR_T { ERROR_TABLE };
#undef X

extern const char *ERROR_NAMES[];

extern const int DA_STATIONS[];
extern const int NUMBER_OF_DA_STATIONS;

class lot_t
{
protected:
    std::string _route;
    std::string _lot_number;
    std::string _pin_package;
    std::string _recipe;
    std::string _prod_id;
    std::string _process_id;
    std::string _bom_id;
    std::string _part_id;
    std::string _pkg_id;
    std::string _part_no;


    std::string _urgent;
    std::string _customer;
    std::string _wb_location;
    std::string _prescheduled_machine;
    std::string _prescheduled_model;
    std::string _last_entity;
    std::string _last_location;
    std::string _wlot_last_trans;

    int _qty;
    int _oper;
    int _lot_size;
    int _sub_lots;
    int _amount_of_wires;
    int _number_of_tools;
    int _prescheduled_order;

    bool _hold;
    bool _mvin;
    bool _is_sub_lot;
    bool _is_automotive;
    bool _spr_hot;

    double _cr;
    double _queue_time;  // for all queue time;
    double _fcst_time;   // for DA fcst time
    double _outplan_time;
    double _cure_time;
    double _weight;

    time_t _da_queue_time;

    bool _finish_traversal;

    std::vector<std::string> __tools;
    std::vector<std::string> _log;
    std::vector<std::string> _all_models;
    std::vector<std::string> _can_run_models;
    std::vector<std::string> _can_run_locations;

    std::map<std::string, double> _uphs;
    std::map<std::string, double> _model_process_times;

    std::map<std::string, int> _tools;

    // TODO : should be removed
    enum ERROR_T _status;

    std::vector<ERROR_T> _statuses;

    /**
     * setAmountOfTools () - setup the number of designated tools
     *
     * The function takes two parameters one is @b part_number and the other
     * is @b number_of_tools. The part_number will be set to available directly.
     *
     * @param part_number : the designated tool's name
     * @param number_of_tools : the number of designated tool
     */
    int setAmountOfTools(std::string part_number, int number_of_tools);

    std::map<std::string, std::string> rearrangeData(
        std::map<std::string, std::string> elements);

    bool checkDataFormat(std::map<std::string, std::string> &elements,
                         std::string &log);

    void setAutomotive(std::string);
    void setAutomotive(bool);

    void setHold(std::string);
    void setHold(bool);

    void setMvin(std::string);
    void setMvin(bool);

    void setSprHot(std::string);
    void setSprHot(bool);

    void _setToolType(std::string tool_type);

public:
    int tmp_oper;
    bool tmp_mvin;

    lot_t();

    /**
     * constructor for lot_t
     *
     * This constructor for lot_t has only one parameter whose type is
     * std::map<std::string, std::string>. The parameter is used to store the
     * relationship between key and data from dataframe. For example,
     * elements["route"] == "BGA321", elements["lot_number"] == "AASJSDKA01".
     * The mapping relationship is from WIP(work in process) sheet. The
     * constructor initializes lots of data members. After initalizing the
     * data members, the constructor will checke if data member has incorrect
     * data. If data has incorrect data, the constructor will throw exception
     * and the error message will point out which information is not provided or
     * incorrect.
     *
     * @exception : std::invalid_argument
     * @param elements : mapping relationship between key and data for
     * datamember.
     */
    lot_t(std::map<std::string, std::string> elements) noexcept(false);

    /**
     * createSublots() - create sub lots
     *
     * The function is used to creat sub-lot, if lot isn't sub-lot. The number
     * of sublots is ⌈(qty / lot_size)⌉.
     *
     * @return a vector of lot_t object splited from the lot.
     */
    std::vector<lot_t> createSublots();

    /**
     * checkFormation() - check the basic formation of object is correct or not.
     * checkFormation is used to check the basic fomation of data member is
     * correct or not. Basic fomation of data member is from WIP(work in
     * process) sheet. If basic formation is incorrect or not provided, the lot
     * will not be involved in scheduling plan.
     *
     * @exception : std::invalid_argumaent
     */
    bool checkFormation();

    /**
     * addLog () - add log for this lot
     */
    void addLog(std::string _text, enum ERROR_T code);

    /**
     * addQueueTime () - add queue time
     *
     * The unit of queue time is minute. Lot initialize _queue_time as 0 in the
     * constructor. addQueueTime is used to add time to _queue_time. The
     * function is used in route_t::calculateQueueTime to add queue time station
     * by station.
     *
     * @param time : queue time in a station
     */
    void addQueueTime(double time, int prev_oper, int current_oper);
    /**
     * addQueueTime () - add queue time with reason
     */
    void addQueueTime(double time, std::string reason);

    /**
     * addCureTime () - add cure time with oper
     */
    void addCureTime(double time, int oper);

    /**
     * setTraverseFinished () - set the traversing flag to be finished
     *
     * If the lot finish traversing the route, the traversing flag need to be
     * set by using this function. The lot will no longer traverse the route
     * again.
     */
    void setTraverseFinished();

    /**
     * setProcessId () - setup the process id
     *
     * The process id is mapped by product_id.
     *
     * @param a std::string type parameter
     */
    void setProcessId(std::string process_id);

    /**
     * setBomId () - setup Bom Id
     *
     * Bom id is mapped by process id
     *
     * @param a std::string type parameter
     */
    void setBomId(std::string bom_id);

    /**
     * setLotSize () - setup lot size
     *
     * The lot can be split out to several sub-lots. The number of qty of each
     * sublot is determined by lot size.
     *
     * @exception : std::invalid_argument if parameter less than 0
     * @param lotsize : a integer type of parameter
     */
    void setLotSize(int lotsize) noexcept(false);

    /**
     * setFcstTime () - setup forecast time
     *
     * forecast time is used to predict the queue time in D/A station
     *
     * @param time : a double type of variable which is in minute unit
     */
    void setFcstTime(double time);

    /**
     * setPartId () - setup part_id
     *
     * part id is used to determine what kind of wire should be used for this
     * lot
     *
     * @param partid : a std::string type of variable.
     */
    void setPartId(std::string partid);

    /**
     * setPartNo () - setup part_no
     *
     * part_no is used to determine what kind of tool should be used for this
     * lot
     *
     * @param part_no : a std::string type of variable
     */
    void setPartNo(std::string part_no);

    /**
     * setAmountOfTools () - setup the number of tools
     *
     * The function gets a parameter which includes all tools' name
     * and their amount. The function only takes the information of the
     * tools owned by the lot.
     *
     * @param number_of_tools : a mapping relation between tools' name and
     * their amount
     * @return : the total number of the tools can be used
     */
    int setAmountOfTools(std::map<std::string, int> number_of_tools);

    /**
     * setAmountOfWires () - setup amount of wires which can be used for this
     * lot
     *
     * @param amount : a integer type of variable
     */
    void setAmountOfWires(int amount);

    /**
     * setCanRunModel () - setup a single model
     */
    void setCanRunModel(std::string model);

    /**
     * setCanRunModels () - setup multiple models
     */
    void setCanRunModels(std::vector<std::string> models);

    void setDAQueueTime(std::string);

    std::string getLastEntity();

    /**
     * getAmountOfTools () - return the number of available tools for this lot
     * @return  integer
     */
    int getAmountOfTools();

    /**
     * getAmountOfWires () - return the number of available wires for this lot
     * @return
     */
    int getAmountOfWires();

    /**
     * getCanRunModels () - return a vector of string which represents can run
     * models
     * @return
     */
    std::vector<std::string> getCanRunModels();

    /**
     * oper () - current oper of this lot
     */
    int oper();

    /**
     * qty () - quantity of this lot
     */
    int qty();

    /**
     * hold () - Is this lot held or not
     *
     * @return true if lot is held
     */
    bool hold();

    /**
     * mvin () - Is this lot move into the machine or not
     *
     * @return true if lot is moving into the machine
     */
    bool mvin();

    /**
     * isSubLot () - Is this lot a sub-lot
     *
     * if length of lot number greater then 8 which means that the lot isn't a
     * sub-lot
     *
     * @return true if lot is a sub-lot
     */
    bool isSubLot();

    /**
     * isTraversalFinished () - Is the traversal finished or not?
     *

     * Lot will traverse each station in the route to sum the queue time and
     determine the arrival time to W/B station. isTraversalFinish is going to
     describe that if traversal is finished or not.
     *
     * @return true if traversal is finished.
     */
    bool isTraversalFinish();

    bool isAutomotive();

    bool sprHot();

    /**
     * route () - get the route of this lot
     *
     * @return std::string type data
     */
    std::string route();

    /**
     * log () - get the log of lot
     *
     * @return std::string type data
     */
    std::string log();

    /**
     * lotNumber () - get the lot_number
     *
     * lotNumber in WIP(work in process) is unique which represent an identifier
     * of a lot.
     *
     * @return std::string type data
     */
    std::string lotNumber();

    /**
     * recipe () - get the recipe of the lot
     *
     * recipe is bd_id.
     *
     * @return std::string type data
     */
    std::string recipe();

    /**
     * processId () - get the process_id of the lot
     *
     * process id is mapped by prod_id. Initially, process_id is empty because
     * this information is not in WIP. process_id is set by setProcessId.
     *
     * @return std::string type data
     */
    std::string processId();

    /**
     * prodId () - get the product_id of the lot
     *
     * product id is in WIP so that the return data will not be an empty string.
     *
     * @return std::string type data
     */
    std::string prodId();

    /**
     * bomId () - get the bom_id of the lot
     *
     * bom id, mapped by process_id, is used to determine what kind of wire
     * should be used on this lot.  Initially, bom_id is an empty string. Call
     * setBomId to setup the bom_id
     *
     * @return std::string type data
     */
    std::string bomId();

    /**
     * pin_package () - get the pin_package of the lot
     *
     * pin_package, an information of lot, is used to
     */
    std::string pin_package();

    /**
     * info () - get the infomation of the lot
     *
     * The infomation includes lot_number, route and recipe. Note that info is a
     * virtual function.
     *
     * @return std::string type data which include lot_number, route and recipe
     */
    virtual std::string info();

    /**
     * part_id () - get the part id of the lot
     *
     * @return std::string type of data.
     */
    std::string part_id();

    /**
     * part_no () - get the part number of the lot
     *
     * @return std::string type of data.
     */
    std::string part_no();

    /**
     * fcst () - get the forecast time of the lot
     *
     * @return forecast time in DA station, double data type, in minutes.
     */
    double fcst();

    /**
     * queueTime () - get the queue time of the lot
     *
     * if lot is finished traversing the route, queue time is also the arrival
     * time to W/B station.
     *
     * @return  time, double data type, in minutes
     */
    double queueTime();

    time_t da_queue_time();

    /**
     * data () - return all attribute of this lot
     * The function can be used to output the information of this lot to csv
     * file. The return value is a map container mapping string, attribute name,
     * to another string, attribute value.
     * @return map<string, string> type data
     */
    std::map<std::string, std::string> data();

    /**
     * setUph () - set the uph for specific model
     *  The function is used to set the uph for specific model. If the uph is 0,
     *  the model will be removed because uph==0 is illegal.
     * @param name :  the model name
     * @param uph : double type value
     * @return true if uph isn't 0, otherwise, return false.
     */
    bool setUph(std::string name, double uph);

    /**
     * setUph () - set the uph by dataframe
     * In this function, correct uph is filtered by recipe, oper, and cust. The
     * function will call setUph(std::string, double) to setup uph for each
     * model.
     * @param uph : csv_t type dataframe
     * @return true if setup uph successfully, return false if there is no
     * model's uph is set.
     */
    bool setUph(csv_t &uph);

    /**
     * setCanRunLocation () - set the can run location
     * This function will setup the lot's can run location. The can run location
     * is determined by its pin package.
     * @param model_locations : model_locations is a map container mapping
     * model's name to it's locations.
     */
    void setCanRunLocation(
        std::map<std::string, std::vector<std::string> > model_locations);

    void setLastLocation(std::string _last_location_str);

    /**
     * getCanRunLocation () - get the can run locations of lot
     * @return a vector of string which represents a location
     */
    std::vector<std::string> getCanRunLocations();

    /**
     * isEntitySuitable () - check if the lot can run on this model and location
     * @param location : location of this entity.
     * @return
     */
    bool isEntitySuitable(std::string location);

    /**
     * job () - generate job_t instance by lot
     * In this function, job_t fields are initialized to its variable. The field
     * includes list.getValue, part_no, pin_package, job_info, customer,
     * part_id, bdid, urgent_code qty, start_time, end_time, arriv_t
     * @return job_t instance
     */
    job_t job();

    /**
     * clearCanRunLocation () clear all can run locations
     */
    void clearCanRunLocation();

    /**
     * getUphs () - get all models' uph
     * @return map<string, double> data mapping model's name to its uph
     */
    std::map<std::string, double> getUphs();

    bool isModelValid(std::string model);

    std::string preScheduledEntity();

    bool isPrescheduled();

    int prescheduledOrder();

    void setPrescheduledModel(std::string model);

    void setNotPrescheduled();

    std::map<std::string, double> getModelProcessTimes();

    virtual bool isInSchedulingPlan();

    /**
     * isLotOkay () - check if lot is faulty
     * The lot is faulty means that lot lacks of some important information
     * or the lot's state is not allowed to schedule. If the lot lacks of some
     * information in data process, the ERROR code will be recorded.
     */
    virtual bool isLotOkay();

    /**
     *
     */
    inline void setProcessTimeRatio(double ratio);

    inline double cureTime();
};

inline double lot_t::cureTime()
{
    return _cure_time;
}

inline void lot_t::setProcessTimeRatio(double ratio)
{
    for (auto it = _model_process_times.begin();
         it != _model_process_times.end(); it++) {
        it->second *= ratio;
    }
}

inline bool lot_t::isInSchedulingPlan()
{
    // if (_lot_number.find("XX") != (std::string::npos)) {
    if (_lot_number.compare(1, 2, "XX") == 0) {
        return false;
    }
    return true;
}

inline std::map<std::string, double> lot_t::getModelProcessTimes()
{
    return _model_process_times;
}

inline void lot_t::setNotPrescheduled()
{
    _prescheduled_order = -1;
}

inline void lot_t::setPrescheduledModel(std::string model)
{
    // FIXME : Is it necessary to check whether the lot is preshceduled or not?
    _prescheduled_model = model;
}

inline void lot_t::clearCanRunLocation()
{
    _can_run_locations.clear();
}

inline std::map<std::string, double> lot_t::getUphs()
{
    return _uphs;
}

inline std::vector<std::string> lot_t::getCanRunLocations()
{
    return _can_run_locations;
}

inline void lot_t::setBomId(std::string bom_id)
{
    _bom_id = bom_id;
}

inline std::string lot_t::part_no()
{
    if (_tools.size() > 0) {
        if (_part_no.length())
            return _part_no;
        else
            return _tools.begin()->first;
    } else {
        return "";
    }
}

inline bool lot_t::isTraversalFinish()
{
    return _finish_traversal;
}

inline bool lot_t::isAutomotive()
{
    return _is_automotive;
}

inline bool lot_t::sprHot()
{
    return _spr_hot;
}

inline int lot_t::oper()
{
    return _oper;
}

inline int lot_t::getAmountOfTools()
{
    return _tools[part_no()];
}

inline int lot_t::getAmountOfWires()
{
    return _amount_of_wires;
}

inline std::string lot_t::route()
{
    return _route;
}


inline bool lot_t::hold()
{
    return _hold;
}

inline bool lot_t::mvin()
{
    return _mvin;
}

inline void lot_t::addLog(std::string _text, enum ERROR_T code)
{
    _log.push_back(_text);
    if (code != SUCCESS) {
        _statuses.push_back(code);
    }
}

inline std::string lot_t::log()
{
    std::string text;
    foreach (_log, i) {
        text += _log[i] + " ";
    }
    return text;
}

inline std::string lot_t::lotNumber()
{
    return _lot_number;
}


inline void lot_t::setTraverseFinished()
{
    _finish_traversal = true;
}


inline std::string lot_t::recipe()
{
    return _recipe;
}

inline void lot_t::setProcessId(std::string process_id)
{
    _process_id = process_id;
}

inline std::string lot_t::processId()
{
    return _process_id;
}

inline std::string lot_t::prodId()
{
    return _prod_id;
}

inline std::string lot_t::bomId()
{
    return _bom_id;
}


inline void lot_t::setLotSize(int lotsize) noexcept(false)
{
    if (lotsize < 0)
        throw std::invalid_argument("lot size is less then 0");
    _lot_size = lotsize;
}

inline int lot_t::qty()
{
    return _qty;
}

inline bool lot_t::isSubLot()
{
    return _is_sub_lot;
}

inline void lot_t::setFcstTime(double time)
{
    _fcst_time += time;
    _queue_time += time;
}


inline void lot_t::addQueueTime(double time, int prev_oper, int current_oper)
{
    addLog("Add queue time + " + std::to_string(time) + " for(" +
               std::to_string(prev_oper) + "->" + std::to_string(current_oper) +
               ")",
           SUCCESS);
    _queue_time += time;
}

inline void lot_t::addQueueTime(double time, std::string reason)
{
    addLog("Add queue time + " + std::to_string(time) +
               " for the reason : " + reason,
           SUCCESS);
    _queue_time += time;
}

inline void lot_t::addCureTime(double time, int oper)
{
    addLog("Add cure time + " + std::to_string(time) +
               " for cure station : " + std::to_string(oper),
           SUCCESS);

    _queue_time += time;
    _cure_time += time;
}


inline double lot_t::fcst()
{
    return _fcst_time;
}


inline double lot_t::queueTime()
{
    return _queue_time;
}

inline time_t lot_t::da_queue_time()
{
    return _da_queue_time;
}

inline std::string lot_t::info()
{
    return "{Lot Number : " + _lot_number + ", route : " + _route +
           ", recipe : " + _recipe + "}";
}


inline void lot_t::setPartId(std::string partid)
{
    _part_id = partid;
}


inline void lot_t::setPartNo(std::string part_no)
{
    if (part_no.length()) {
        if (_tools.count(part_no) == 0) {
            _tools[part_no] = 0;
            __tools.push_back(part_no);
        }
        _part_no = part_no;
    }
}

inline void lot_t::_setToolType(std::string part_no_type)
{
    std::string part_no("");
    for (auto it = _tools.begin(); it != _tools.end(); ++it) {
        if (it->second != 0 &&
            it->first.find(part_no_type) != std::string::npos) {
            part_no = it->first;
            break;
        }
    }
    setPartNo(part_no);
}


inline std::string lot_t::part_id()
{
    return _part_id;
}

inline int lot_t::setAmountOfTools(std::string part_number, int number_of_tools)
{
    setPartNo(part_number);
    _tools[part_number] = number_of_tools;
    return number_of_tools;
}

inline int lot_t::setAmountOfTools(std::map<std::string, int> number_of_tools)
{
    int sum = 0;
    std::map<std::string, int> tls;
    for (auto it = _tools.begin(); it != _tools.end(); ++it) {
        if (number_of_tools.count(it->first) != 0) {
            it->second = number_of_tools.at(it->first);  // safe, check it above
            sum += it->second;
            tls[it->first] = it->second;
        }
    }
    _tools = tls;  // remove the tool whose amount is 0
    return sum;
}


inline void lot_t::setAmountOfWires(int amount)
{
    _amount_of_wires = amount;
}

inline bool lot_t::isModelValid(std::string model)
{
    /* _setToolType() would determine value of _part_no */
    std::string _part_no = part_no();
    if (_part_no.find("A0801") !=
        std::string::npos) {  // if part_no contains A0801
        return (model.compare("UTC1000") == 0 || model.compare("UTC1000S") == 0 ||
                model.compare("UTC2000") == 0 || model.compare("UTC2000S") == 0 ||
                model.compare("UTC3000") == 0);
    } else if (_part_no.find("A0803") !=
               std::string::npos) {  // if part_no contains A0803
        return (model.compare("UTC1000") != 0 && model.compare("UTC1000S") != 0 &&
                model.compare("UTC2000") != 0 && model.compare("UTC2000S") != 0 &&
                model.compare("UTC3000") != 0);
    }
    return true;
}

inline void lot_t::setCanRunModel(std::string model)
{
    if (isModelValid(model)) {
        _uphs[model] = 0;
        _model_process_times[model] = 0;
        if (find(_can_run_models.begin(), _can_run_models.end(), model) !=
            _can_run_models.end())
            _can_run_models.push_back(model);
    }
}

inline void lot_t::setCanRunModels(std::vector<std::string> models)
{
    // use models to decide the part_no
    int a0801 = 0, a0803 = 0;
    foreach (models, i) {
        a0801 +=  (models[i].compare("UTC1000") == 0 ||
            models[i].compare("UTC1000S") == 0 ||
            models[i].compare("UTC2000") == 0 ||
            models[i].compare("UTC2000S") == 0 ||
            models[i].compare("UTC3000") == 0);
        a0803 +=  !(models[i].compare("UTC1000") == 0 ||
            models[i].compare("UTC1000S") == 0 ||
            models[i].compare("UTC2000") == 0 ||
            models[i].compare("UTC2000S") == 0 ||
            models[i].compare("UTC3000") == 0);
    }
    _setToolType(a0801 > a0803? "A0801" : "A0803");

    foreach (models, i) {
        if (isModelValid(models[i])) {
            _uphs[models[i]] = 0;
            _model_process_times[models[i]] = 0;
        }
    }

    _all_models = models;
}

inline bool lot_t::setUph(std::string model, double uph)
{
    if (uph == 0) {
        _uphs.erase(model);
        _model_process_times.erase(model);
        return false;
    } else {
        _uphs.at(model) = uph;
        _model_process_times[model] = _qty * 60 / uph;
    }
    return true;
}

inline std::vector<std::string> lot_t::getCanRunModels()
{
    return _can_run_models;
}

inline std::string lot_t::pin_package()
{
    return _pin_package;
}

inline std::string lot_t::preScheduledEntity()
{
    return _prescheduled_machine;
}

inline std::string lot_t::getLastEntity()
{
    return _last_entity;
}

inline bool lot_t::isPrescheduled()
{
    return _prescheduled_order >= 0;
}

inline int lot_t::prescheduledOrder()
{
    return _prescheduled_order;
}

inline void lot_t::setAutomotive(std::string _automotive_str)
{
    _is_automotive = _automotive_str.compare("Y") == 0;
}

inline void lot_t::setAutomotive(bool _automotive_val)
{
    _is_automotive = _automotive_val;
}

inline void lot_t::setHold(std::string _hold_str)
{
    _hold = _hold_str.compare("Y") == 0;
}

inline void lot_t::setLastLocation(std::string _last_location_str)
{
    _last_location = _last_location_str;
}

inline void lot_t::setHold(bool _hold_val)
{
    _hold = _hold_val;
}

inline void lot_t::setMvin(std::string _mvin_str)
{
    _mvin = _mvin_str.compare("Y") == 0;
}

inline void lot_t::setMvin(bool _mvin_val)
{
    _mvin = _mvin_val;
}

inline void lot_t::setSprHot(std::string _sprHot_str)
{
    _spr_hot = _sprHot_str.compare("Y") == 0;
}

inline void lot_t::setSprHot(bool _sprHot_val)
{
    _spr_hot = _sprHot_val;
}

inline void lot_t::setDAQueueTime(std::string base_time)
{
    int i = 0;
    for (i = 0; i < NUMBER_OF_DA_STATIONS; ++i)
        if (DA_STATIONS[i] == oper())
            break;
    // _da_queue_time =
    //     (isSubLot() && mvin() && (i != NUMBER_OF_DA_STATIONS))
    //         ? (timeConverter(_wlot_last_trans) - timeConverter(base_time)) /
    //               60.0
    //         : 0;

    _da_queue_time = (isSubLot() && mvin() && i != NUMBER_OF_DA_STATIONS) *
                     (timeConverter(base_time)(_wlot_last_trans)) / 60.0;
}
#endif
