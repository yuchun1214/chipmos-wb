#ifndef __MACHINES_H__
#define __MACHINES_H__

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "include/job_base.h"
#include "include/machine.h"
#include "include/machine_base.h"
#include "include/machine_constraint.h"
#include "include/parameters.h"

/**
 * @struct __machine_group_t : A structure used to store the machines and jobs
 * whose recipes are the same. The jobs in the structure are dispatched to each
 * machine.
 */
struct __machine_group_t {
    std::vector<machine_t *> machines;
    std::vector<job_t *> unscheduled_jobs;
    std::vector<job_t *> scheduled_jobs;
    std::vector<job_t *> sprhot_jobs;
    std::string part_no;
    std::string part_id;
    std::string recipe;

    std::vector<ares_t *> tools;
    std::vector<ares_t *> wires;

    /// index : used to determine which group had better get the new machine
    double index;
};

struct __job_group_t {
    std::string part_no;
    std::string part_id;
    int number_of_tools;
    int number_of_wires;

    int number_of_machines;
    int number_of_jobs;

    std::vector<job_t *> jobs;
    std::vector<job_t *> orphan_jobs;
};

struct __distribution_entry_t {
    std::string name;
    double ratio;
};

class machines_t
{
protected:
    // entity_name -> machine_t *
    std::map<std::string, machine_t *> _machines;

    // vector machines
    std::vector<machine_t *> _v_machines;

    std::vector<machine_t *> _grouped_machines;

    // tool_wire_name -> machine_t *
    std::map<std::string, std::vector<machine_t *>> _tool_wire_machines;
    std::map<std::string, std::vector<machine_t *>> _tool_machines;
    std::map<std::string, std::vector<machine_t *>> _wire_machines;

    // model->locations
    std::map<std::string, std::vector<std::string>> _model_locations;

    // scheduled jobs
    std::vector<job_t *> _job_on_machine;
    std::vector<job_t *> _scheduled_jobs;

    // operations
    list_operations_t *list_ops;
    machine_base_operations_t *machine_ops;
    job_base_operations_t *job_ops;

    // parameters and weights
    std::map<std::pair<std::string, std::string>, double>
        _transportation_time_table;
    setup_time_parameters_t _param;
    weights_t _weights;
    int threshold;

    // other job information such as can_run_location and process_times
    std::map<std::string, std::vector<std::string>> _job_can_run_locations;
    std::map<std::string, std::map<std::string, double>> _job_process_times;

    // store other job information such as can run machine
    std::map<std::string, std::vector<std::string>> _job_can_run_machines;

    // store the the wire and tool carried by the machines;
    std::map<std::string, std::vector<std::string>> _machines_tools;
    std::map<std::string, std::vector<std::string>> _machines_wires;

    // groups
    std::map<std::string, struct __machine_group_t *> _dispatch_groups;


    std::map<std::string, std::vector<struct __job_group_t *>>
        _wire_jobs_groups;
    std::map<std::string, std::vector<struct __job_group_t *>>
        _tool_jobs_groups;
    std::map<std::string, struct __job_group_t *> _tool_wire_jobs_groups;
    std::vector<struct __job_group_t *> _jobs_groups;

    // number of tools and wires
    std::map<std::string, int> _number_of_tools;
    std::map<std::string, int> _number_of_wires;

    // tools and wires
    std::map<std::string, std::vector<ares_t *>> _tools;
    std::map<std::string, std::vector<ares_t *>> _loaded_tools;
    std::map<std::string, std::vector<ares_t *>> _wires;
    std::map<std::string, std::vector<ares_t *>> _loaded_wires;

    // dedicate machines
    std::map<std::string, std::map<std::string, bool>> _dedicate_machines;

    // automotive lots
    std::set<std::string> _automotive_lot_numbers;

    // machine constraints
    machine_constraint_t *_mcs_a, *_mcs_r;

    bool _isMachineLocationAvailableForJob(std::string lot_number,
                                           std::string location);
    bool _isModelAvailableForJob(std::string lot_number, std::string model);

    bool _isMachineDedicatedForJob(std::string lot_number,
                                   std::string cust,
                                   std::string entity);

    bool _isMachineRestrainedForJob(job_t *job, machine_t *machine);

    bool _canJobRunOnTheMachine(job_t *jb,
                                machine_t *machine,
                                bool strict_model = true);


    void _init(setup_time_parameters_t param);
    int _scheduleAGroup(struct __machine_group_t *group);
    std::vector<machine_t *> _sortedMachines(std::vector<machine_t *> &ms);
    std::vector<job_t *> _sortedJobs(std::vector<job_t *> &jobs);

    bool _addNewResource(
        machine_t *machine,
        std::string resource_name,
        std::map<std::string, std::vector<std::string>> &container);

    std::map<std::string, int> _distributeAResource(
        int number_of_resources,
        std::map<std::string, int> groups_statistic);

    void _chooseMachinesForAGroup(struct __job_group_t *group,
                                  std::vector<machine_t *> candidate_machines);

    void _initializeNumberOfExpectedMachines();

    void _setupContainersForMachines();

    void _setupResources(
        std::map<std::string, int> &number_of_resource,
        std::map<std::string, std::vector<ares_t *>>
            &resources_instance_container,
        std::map<std::string, std::vector<machine_t *>> &resource_machines);

    resources_t _loadResource(
        std::vector<std::string> list,
        std::map<std::string, std::vector<ares_t *>> &resource_instances,
        std::map<std::string, std::vector<ares_t *>> &used_resources);

    void _loadResourcesOnTheMachine(machine_t *machine);

    void _linkMachineToAJob(job_t *job);

    bool _isThereAnyUnusedResource(
        std::map<std::string, std::vector<ares_t *>> _resources,
        std::string resource_name,
        int threshold = 1);

    /**
     * _jobsExceedDispatchingThreshold - take out the job dispatched on the
     * machine, exceeding the threshold The function will go through the linked
     * list of machine and check if the job exceeds the threshold given by
     * function parameter. The function will also take out the job and close the
     * job linked list.
     * @param machine
     * @param threshold
     * @return
     */
    std::vector<job_t *> _jobsExceedDispatchingThreshold(machine_t *machine,
                                                         int threshold);

    void _createResources(std::map<std::string, int> &number_of_resource,
                          std::map<std::string, std::vector<ares_t *>>
                              &resource_instance_container);

    void _updateAKindOfResourceAvailableTime(
        std::vector<ares_t *> &resource_instances,
        std::vector<machine_t *> &resource_machines);
    void _updateAllKindOfResourcesAvailableTime(
        std::map<std::string, std::vector<ares_t *>>
            &resource_instance_container,
        std::map<std::string, std::vector<machine_t *>> &resource_machines);

    int _collectScheduledJobs(machine_t *machine,
                              std::vector<job_t *> &scheduled_jobs);

    void _loadResource(struct __machine_group_t *group);

    bool _distributeOrphanMachines(struct __machine_group_t *group,
                                   machine_t *orphan_machine);

    /**
     * _calculateMachineGroupIndex - calculate the index of a machine group
     * @param group
     * @return
     */
    double _calculateMachineGroupIndex(struct __machine_group_t *group);

    /**
     * _averageProcessTime - calculate the average process time for a job
     * @param process_times : the process time of a job
     * @return double type number
     */
    double _averageProcessTime(std::map<std::string, double> process_times);


    ares_t *_availableResource(
        std::map<std::string, std::vector<ares_t *>> resource,
        std::string name);

public:
    machines_t();

    machines_t(setup_time_parameters_t param,
               std::map<std::pair<std::string, std::string>, double>
                   &transportation_time_table,
               weights_t weight);

    machines_t(machines_t &other);

    const std::vector<machine_t *> scheduledMachines();

    void addMachine(machine_t machine);

    void addPrescheduledJob(job_t *job);

    void addJobLocation(std::string lot_number,
                        std::vector<std::string> locations);
    void addJobProcessTimes(std::string lot_number,
                            std::map<std::string, double> uphs);

    void setNumberOfTools(std::map<std::string, int> tool_number);
    void setNumberOfWires(std::map<std::string, int> wire_number);

    void addGroupJobs(std::string recipe, std::vector<job_t *> jobs);

    void prescheduleJobs();

    std::string getModelByEntityName(std::string entity_name);

    int scheduleGroups();

    std::map<std::string, std::vector<std::string>> getModelLocations();

    const std::vector<job_t *> getScheduledJobs();

    const std::vector<job_t *> getOnMachineJobs();

    void groupJobsByToolAndWire();


    void distributeTools();

    void distributeWires();

    void chooseMachinesForGroups();

    void setupToolAndWire();

    void reconsiderJobs();

    void prepareMachines(int *number, machine_t ***machine_array);

    void prepareJobs(int *number, job_t ***job_array);


    void setDedicateMachines(
        std::map<std::string, std::map<std::string, bool>> dedicate_machines);

    void setAutomotiveLotNumber(std::set<std::string> automtv_lots);

    list_operations_t *getInitializedListOperations();
    job_base_operations_t *getInitializedJobBaseOperations();
    machine_base_operations_t *getInitilizedMachineBaseOperations();

    void distributeOrphanMachines(double probability = 1.0);

    ~machines_t();

    void setThreshold(int day_threshold);

    void setMachineConstraintA(machine_constraint_t *mcs_a);
    void setMachineConstraintR(machine_constraint_t *mcs_r);
};


inline void machines_t::setMachineConstraintA(machine_constraint_t *mcs_a)
{
    _mcs_a = mcs_a;
}

inline void machines_t::setMachineConstraintR(machine_constraint_t *mcs_r)
{
    _mcs_r = mcs_r;
}

// inline void
// machines_t::setMachineConstraints(std::vector<machine_constraint_t *> mcsts)
// {
//     _machine_constraints = mcsts;
// }

inline void machines_t::setThreshold(int day_threshold)
{
    threshold = day_threshold;
}

inline list_operations_t *machines_t::getInitializedListOperations()
{
    return list_ops;
}

inline job_base_operations_t *machines_t::getInitializedJobBaseOperations()
{
    return job_ops;
}

inline machine_base_operations_t *
machines_t::getInitilizedMachineBaseOperations()
{
    return machine_ops;
}

inline const std::vector<job_t *> machines_t::getOnMachineJobs()
{
    return _job_on_machine;
}

inline const std::vector<job_t *> machines_t::getScheduledJobs()
{
    return this->_scheduled_jobs;
}

inline std::map<std::string, std::vector<std::string>>
machines_t::getModelLocations()
{
    return _model_locations;
}

inline std::string machines_t::getModelByEntityName(std::string entity_name)
{
    try {
        return std::string(_machines.at(entity_name)->model_name.data.text);
    } catch (std::out_of_range &e) {
        std::string err_msg =
            "Error : entity name [" + entity_name + "] not found " + e.what();
        throw std::out_of_range(err_msg);
    }
}

inline const std::vector<machine_t *> machines_t::scheduledMachines()
{
    std::vector<machine_t *> machines;
    for (std::map<std::string, machine_t *>::iterator it = _machines.begin();
         it != _machines.end(); ++it) {
        if (it->second->base.size_of_jobs > 0) {
            machines.push_back(it->second);
        }
    }

    return machines;
}

inline void machines_t::addJobLocation(std::string lot_number,
                                       std::vector<std::string> locations)
{
    if (_job_can_run_locations.count(lot_number) != 0)
        std::cerr << "Warning : add job location twice, lot_number is ["
                  << lot_number << "]" << std::endl;

    _job_can_run_locations[lot_number] = locations;
}

inline void machines_t::addJobProcessTimes(
    std::string lot_number,
    std::map<std::string, double> process_times)
{
    if (_job_process_times.count(lot_number) != 0)
        std::cerr << "Warning : add job process_times twice, lot_number is ["
                  << lot_number << "]" << std::endl;


    _job_process_times[lot_number] = process_times;
}

inline void machines_t::setNumberOfTools(std::map<std::string, int> tool_number)
{
    this->_number_of_tools = tool_number;
}

inline void machines_t::setNumberOfWires(std::map<std::string, int> wire_number)
{
    this->_number_of_wires = wire_number;
}

inline void machines_t::setDedicateMachines(
    std::map<std::string, std::map<std::string, bool>> dedicate_machines)
{
    _dedicate_machines = dedicate_machines;
}

inline void machines_t::setAutomotiveLotNumber(
    std::set<std::string> automotv_lot_numbers)
{
    _automotive_lot_numbers = automotv_lot_numbers;
}

class machine_base_operations_initializer_t
{
private:
    int _number_of_setup_time_units;
    machine_base_operations_t *ops;

public:
    machine_base_operations_initializer_t(std::vector<setup_time_t> funcs,
                                          setup_time_parameters_t parameters)
    {
        _number_of_setup_time_units =
            sizeof(setup_time_parameters_t) / sizeof(double);
        ops = (machine_base_operations_t *) malloc(
            sizeof(machine_base_operations_t) +
            sizeof(setup_time_unit_t) * _number_of_setup_time_units);
        double *_parameters = (double *) &parameters;
        for (unsigned int i = 0; i < funcs.size(); ++i) {
            ops->setup_time_functions[i] = {funcs[i], _parameters[i]};
        }
        ops->sizeof_setup_time_function_array = _number_of_setup_time_units - 1;
        ops->reset = machineReset;
        ops->add_job = machineBaseAddJob;
        ops->sort_job = machineBaseSortJob;
    }

    machine_base_operations_t *getOps() { return ops; }

    ~machine_base_operations_initializer_t() { free(ops); }
};
#endif
