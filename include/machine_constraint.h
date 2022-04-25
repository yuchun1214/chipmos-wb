#ifndef __MACHINE_CONSTRAINT__
#define __MACHINE_CONSTRAINT__

#include "include/csv.h"
#include "include/job.h"
#include "include/machine.h"
#include "include/route.h"

#include <map>
#include <regex>
#include <string>
#include <vector>



struct constraint_entry_t {
    std::regex check_val;
    std::string _str_check_val;
    std::regex entity_name;
    std::string _str_entity_name;
    std::string model;
};

struct constraint_oper_t {
    std::map<std::string, std::vector<constraint_entry_t>> restrained_entity;
    std::map<std::string, std::vector<constraint_entry_t>>
        restrained_entity_pinpkg;

    constraint_oper_t() : restrained_entity(), restrained_entity_pinpkg() {}
};

class machine_constraint_t
{
private:
    void _storeStationData(csv_t csv, int oper);
    void _storeRestrainedData(
        csv_t csv,
        std::map<std::string, std::vector<constraint_entry_t>> &restraint_ent);

protected:
    std::map<int, constraint_oper_t> _table;


    /**
     * _isMachineRestrained : give the restrained information, judge the machine
     * is restrained or not Return true if machine is okay which means the
     * machine follows the limitation. The function's result depends on the
     * implementation of the virtual function.
     * @param entity_re
     * @param restrained_model
     * @param entity_name
     * @param model_name
     * @return
     */
    virtual bool _isMachineRestrained(std::regex entity_re,
                                      std::string restrained_model,
                                      std::string entity_name,
                                      std::string model_name) = 0;

    virtual bool _isMachineRestrained(constraint_oper_t &oper,
                                      job_t *job,
                                      machine_t *machine,
                                      bool *care) = 0;


    /**
     * _isMachineRestrained : give several limitation, judge the machine is
     * restrained or not
     * @param val : job value, could be pin_pkg or pkg_id
     * @param entries : limitation entries
     * @param machine : single machine to be judged
     * @return true if machine is okay for the job.
     */
    virtual bool _isMachineRestrainedForTheValue(
        std::vector<constraint_entry_t> entries,
        machine_t *machine) = 0;

    machine_constraint_t() = default;

public:
    machine_constraint_t(csv_t csv);
    static std::string transformStarToRegexString(std::string str);
    static std::string transformPkgIdToRegex(std::string pkg_id);
    static std::string transformEntityGroupToRegex(std::string entity_group);

    std::vector<constraint_entry_t> getConstraintEntries(
        std::map<std::string, std::vector<constraint_entry_t>> entries,
        const std::string &cust,
        const std::string &val);
    virtual bool isMachineRestrained(job_t *job,
                                     machine_t *machine,
                                     bool *care);

    virtual ~machine_constraint_t() = default;
};


#endif
