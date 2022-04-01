#include "include/entities.h"

using namespace std;

void entities_t::_readProcessIdFile(std::string filename)
{
    csv_t csv(filename, "r", true, true);
    csv.trim(" ");
    csv.setHeaders(map<string, string>({{"prod_id", "product"},
                                        {"process_id", "process_id"},
                                        {"bom_id", "bom_id"}}));

    map<string, string> row;
    for (int i = 0, nrows = csv.nrows(); i < nrows; ++i) {
        row = csv.getElements(i);
        prod_map_to_pid[row["prod_id"]] = row["process_id"];
        prod_map_to_bom_id[row["prod_id"]] = row["bom_id"];
    }
}

void entities_t::_readPartNoFile(std::string filename)
{
    csv_t csv(filename, "r", true, true);
    csv.trim(" ");
    csv.setHeaders(map<string, string>(
        {{"process_id", "process_id"}, {"remark", "remark"}}));
    map<string, string> pid_remark;
    map<string, string> row;
    string remark;
    for (int i = 0, size = csv.nrows(); i < size; ++i) {
        row = csv.getElements(i);
        remark = row["remark"];
        if (remark[0] == 'A') {
            remark = remark.substr(0, remark.find("("));
            if (remark.find("(") != std::string::npos) {
                remark = remark.substr(0, remark.find("("));
            }
            pid_remark[row["process_id"]] = remark.substr(0, remark.find(" "));
        }
    }
    pid_map_to_part_no = pid_remark;
}

void entities_t::_readPartIdFile(std::string filename)
{
    csv_t csv(filename, "r", true, true);
    csv.trim(" ");
    csv.setHeaders(map<string, string>(
        {{"bom_id", "bom_id"}, {"oper", "oper"}, {"part_id", "part_id"}}));
    map<string, string> row;
    for (int i = 0, size = csv.nrows(); i < size; ++i) {
        row = csv.getElements(i);
        bom_id_map_to_part_id[row["bom_id"] + "_" + row["oper"]] =
            row["part_id"];
    }
}

void entities_t::_readDedicateMachines(std::string filename)
{
    csv_t csv(filename, "r", true, true);
    csv.trim(" ");
    _dedicate_machines.clear();
    map<string, string> row;
    for (int i = 0, size = csv.nrows(); i < size; ++i) {
        row = csv.getElements(i);
        // _dedicate_machines[row.at("customer") + "_" + row.at("entity")] =
        // row.at("pass").compare("Y") == 0 ? true : false;
        _dedicate_machines[row.at("customer")][row.at("entity")] =
            row.at("pass").compare("Y") == 0;
    }
    csv.close();
}

void entities_t::_setupMachineConstraints(std::string filename)
{
    csv_t csv(filename, "r", true, true);
    csv.trim(" ");
    csv_t r_csv, a_csv;
    r_csv = csv.filter("el_action", "R");
    a_csv = csv.filter("el_action", "A");
    _mcs_r = new machine_constraint_r_t(r_csv);
    _mcs_a = new machine_constraint_a_t(a_csv);
}

entities_t::entities_t()
{
    _time = 0;
}

entities_t::entities_t(std::map<std::string, std::string> arguments)
{
    _time = 0;
    setTime(arguments["std_time"]);
    _readProcessIdFile(arguments["pid_bomid"]);
    _readPartNoFile(arguments["pid_heatblock"]);
    _readPartIdFile(arguments["bom_list"]);
    _readDedicateMachines(arguments["dedicate_machines"]);
    _setupMachineConstraints(arguments["ent_limit"]);
}

void entities_t::setTime(string time)
{
    if (time.length()) {
        _time = timeConverter()(time);
    } else
        _time = 0;
}


entity_t *entities_t::addMachine(map<string, string> elements,
                                 machine_base_operations_t *ops)
{
    string model, location;
    model = elements["model"];
    location = elements["location"];

    string prod_id = elements["prod_id"];
    string part_no = pid_map_to_part_no[prod_map_to_pid[prod_id]];
    string part_id = bom_id_map_to_part_id[prod_map_to_bom_id[prod_id] + "_" +
                                           elements["oper"]];
    elements["part_no"] = part_no;
    elements["part_id"] = part_id;

    entity_t *ent = new entity_t(elements, ops, _time);
    if (ent) {
        _ents.push_back(ent);
        _entities[model][location].push_back(ent);
        _loc_ents[location].push_back(ent);

        name_entity[ent->getEntityName()] = ent;

        if (_model_locations.count(model) == 0) {
            _model_locations[model] = vector<string>();
        }

        // if it is unable to find location in _model_locations[model]
        // add location in _model_locations[model] list
        if (find(_model_locations[model].begin(), _model_locations[model].end(),
                 location) == _model_locations[model].end()) {
            _model_locations[model].push_back(location);
        }

    } else {
        perror("addMachine");
    }

    return ent;
}

void entities_t::addMachines(csv_t _machines,
                             csv_t _location,
                             machine_base_operations_t *ops)
{
    int mrows = _machines.nrows();
    int lrows = _location.nrows();
    map<string, string> locations;  // entity->location
    for (int i = 0; i < lrows; ++i) {
        map<string, string> elements = _location.getElements(i);
        string ent = elements["entity"];
        string loc = elements["location"];
        locations[ent] = loc;
    }

    for (int i = 0; i < mrows; ++i) {
        map<string, string> elements = _machines.getElements(i);
        try {
            elements["location"] = locations.at(elements["entity"]);
            addMachine(elements, ops);
        } catch (std::out_of_range &e) {
            // cout<<elements["entity"]<<endl;
            elements["log"] = "can't find the location of this entity";
            _faulty_machine.push_back(elements);
        } catch (std::invalid_argument &e) {
            elements["log"] = e.what();
            _faulty_machine.push_back(elements);
        }
    }
    return;
}
