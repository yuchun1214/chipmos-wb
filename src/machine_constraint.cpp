#include "include/machine_constraint.h"
#include <algorithm>
#include <map>
#include <regex>
#include <string>
#include <utility>
#include <vector>
#include "include/job.h"

using namespace std;

string machine_constraint_t::transformStarToRegexString(std::string str)
{
    regex star("\\*");
    return regex_replace(str, star, R"(\w*)"s);
}

string machine_constraint_t::transformPkgIdToRegex(string pkg_id)
{
    regex left_parenthese("\\{");
    regex right_parenthese("\\}");
    pkg_id = transformStarToRegexString(pkg_id);
    pkg_id = regex_replace(pkg_id, left_parenthese, R"(\{)");
    pkg_id = regex_replace(pkg_id, right_parenthese, R"(\})");
    return pkg_id;
}

string machine_constraint_t::transformEntityGroupToRegex(string entity_group)
{
    return transformStarToRegexString(entity_group);
}

void machine_constraint_t::_storeStationData(csv_t csv, int oper)
{
    constraint_oper_t station_data;

    if (csv.nrows()) {
        csv_t restrained_entity = csv.filter("el_key", "RESTRAINED-ENTITY");
        _storeRestrainedData(restrained_entity, station_data.restrained_entity);

        csv_t restrained_pinpkg = csv.filter("el_key", "RESTRAINED-ENT-PINPK");
        _storeRestrainedData(restrained_pinpkg,
                             station_data.restrained_entity_pinpkg);
        _table[oper] = station_data;
    }
}

void machine_constraint_t::_storeRestrainedData(
    csv_t csv,
    map<string, vector<constraint_entry_t>> &restrained_ent)
{
    for (int i = 0, size = csv.nrows(); i < size; ++i) {
        map<string, string> row = csv.getElements(i);
        string cust = row["el_customer"];
        string pkg_id = transformPkgIdToRegex(row["el_check_value"]);
        string entity_group =
            transformEntityGroupToRegex(row["el_entity_group"]);

        if (restrained_ent.count(cust) == 0) {
            restrained_ent[cust] = vector<constraint_entry_t>();
            // cout << cust << endl;
        }
        restrained_ent[cust].push_back(
            constraint_entry_t{regex(pkg_id), pkg_id, regex(entity_group),
                               entity_group, row["el_entity_model"]});
    }
}



machine_constraint_t::machine_constraint_t(csv_t csv)
{
    for (int i = 0; i < NUMBER_OF_WB_STATIONS; ++i) {
        csv_t tmp = csv.filter("el_entity_oper", to_string(WB_STATIONS[i]));
        _storeStationData(tmp, WB_STATIONS[i]);
    }
}

vector<constraint_entry_t> machine_constraint_t::getConstraintEntries(
    std::map<std::string, std::vector<constraint_entry_t>> entries,
    const std::string &cust,
    const std::string &val)
{
    vector<constraint_entry_t> out;
    if (entries.count(cust) == 0)
        return out;

    vector<constraint_entry_t> possible_entries = entries.at(cust);
    foreach (possible_entries, i) {
        if (regex_match(val, possible_entries[i].check_val)) {
            out.push_back(possible_entries[i]);
        }
    }
    return out;
}


bool machine_constraint_t::isMachineRestrained(job_t *job,
                                               machine_t *machine,
                                               bool *care)
{
    if (job && machine) {
        string pinpkg(job->pin_package.data.text);
        string pkg_id(job->pkg_id.data.text);
        string cust(job->customer.data.text);
        if (_table.count(job->oper) == 0) {
            return true;
        } else {
            return _isMachineRestrained(_table.at(job->oper), job, machine,
                                        care);
        }
    } else {
        return false;
    }
}
