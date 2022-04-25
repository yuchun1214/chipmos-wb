#include "include/machine_constraint_r.h"
#include "include/machine_constraint.h"

#include <regex>
#include <string>
#include <vector>

using namespace std;

machine_constraint_r_t::machine_constraint_r_t(csv_t csv)
    : machine_constraint_t(csv)
{
}

bool machine_constraint_r_t::_isMachineRestrained(std::regex entity_re,
                                                  std::string restrained_model,
                                                  std::string entity_name,
                                                  std::string machine_model)
{
    // R means reject if entity_re matches and two given models are the same
    // if one of the condition isn't matched, the machine is okay
    return !regex_match(entity_name, entity_re) ||
           restrained_model != machine_model;
}
bool machine_constraint_r_t::_isMachineRestrained(
    constraint_oper_t &station_data,
    job_t *job,
    machine_t *machine,
    bool *care)
{
    if (job && machine) {
        string pin_pkg(job->pin_package.data.text);
        string pkg_id(job->pkg_id.data.text);
        string cust(job->customer.data.text);

        vector<constraint_entry_t> restrained_entity_entries =
            getConstraintEntries(station_data.restrained_entity, cust, pkg_id);
        vector<constraint_entry_t> restrained_entity_pinpkg_entries =
            getConstraintEntries(station_data.restrained_entity_pinpkg, cust,
                                 pin_pkg);

        bool retval = true;
        *care = false;
        if (restrained_entity_pinpkg_entries.size()) {
            retval = _isMachineRestrainedForTheValue(
                restrained_entity_pinpkg_entries, machine);
            *care = true;
        } else {
            *care = false;
        }

        if (restrained_entity_entries.size()) {
            retval &= _isMachineRestrainedForTheValue(restrained_entity_entries,
                                                      machine);
            *care = true;
        } else {
            *care |= false;
        };

        return retval;
    } else {
        return false;
    }
}
bool machine_constraint_r_t::_isMachineRestrainedForTheValue(
    std::vector<constraint_entry_t> entries,
    machine_t *machine)
{
    string entity_name(machine->base.machine_no.data.text);
    string model_name(machine->model_name.data.text);

    foreach (entries, i) {
        if (!_isMachineRestrained(entries[i].entity_name, entries[i].model,
                                  entity_name, model_name)) {
            return false;
        }
    }

    return true;
}

machine_constraint_r_t::~machine_constraint_r_t() {}
