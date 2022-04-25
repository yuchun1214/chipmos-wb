//
// Created by YuChunLin on 2021/11/22.
//


#include "include/machine_constraint_a.h"
#include "include/machine_constraint.h"

#include <regex>
#include <string>
#include <vector>

using namespace std;

machine_constraint_a_t::machine_constraint_a_t(csv_t csv)
    : machine_constraint_t(csv)
{
}

bool machine_constraint_a_t::_isMachineRestrained(std::regex entity_re,
                                                  std::string restrained_model,
                                                  std::string entity_name,
                                                  std::string machine_model)
{
    // a means acceptance if entity_re matches and two given models are the same
    return regex_match(entity_name, entity_re) &&
           restrained_model == machine_model;
}
bool machine_constraint_a_t::_isMachineRestrained(
    constraint_oper_t &station_data,
    job_t *job,
    machine_t *machine,
    bool *care)
{
    if (job && machine) {
        string pin_pkg(job->pin_package.data.text);
        string pkg_id(job->pkg_id.data.text);
        string cust(job->customer.data.text);

        // first get the constraint entries,
        // if constraint entries is empty, return true, which means that
        // there is no any constraint for this job

        // if constraint entries isn't empty, check if machine is restrained

        // generally, If there is no any constraint for the job, return true.
        // otherwise, check the machine follow the constraint, if the machine
        // follow the constraint, return true.

        vector<constraint_entry_t> restrained_entity_entries =
            getConstraintEntries(station_data.restrained_entity, cust, pkg_id);
        vector<constraint_entry_t> restrained_entity_pinpkg_entries =
            getConstraintEntries(station_data.restrained_entity_pinpkg, cust,
                                 pin_pkg);

        int retval = true;
        *care = false;
        if (restrained_entity_entries.size()) {
            retval = _isMachineRestrainedForTheValue(restrained_entity_entries,
                                                     machine);
            *care = true;
        } else {
            *care |= false;
        }

        if (restrained_entity_pinpkg_entries.size()) {
            retval |= _isMachineRestrainedForTheValue(
                restrained_entity_pinpkg_entries, machine);
            *care = true;
        } else {
            *care |= false;
        }

        return retval;
    } else {
        return false;
    }
}
bool machine_constraint_a_t::_isMachineRestrainedForTheValue(
    std::vector<constraint_entry_t> entries,
    machine_t *machine)
{
    // check if the machine follow the constraint given by the job.
    // In this class, the override version is for "A" which  means that
    // the machine is accepted iff the job follow the constraint.

    // so, if the job follow the constraints, the function return true
    // in the following foreach loop
    // otherwise, return false
    string entity_name(machine->base.machine_no.data.text);
    string model_name(machine->model_name.data.text);
    foreach (entries, i) {
        if (_isMachineRestrained(entries[i].entity_name, entries[i].model,
                                 entity_name, model_name)) {
            return true;
        }
    }
    return false;
}

machine_constraint_a_t::~machine_constraint_a_t() {}
