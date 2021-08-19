#ifndef __MACHINES_H__
#define __MACHINES_H__

#include <map>
#include <string>
#include <vector>

#include "include/job_base.h"
#include "include/machine.h"
#include "include/machine_base.h"
#include "include/parameters.h"

class machines_t
{
protected:
    // entity_name -> machine_t *
    std::map<std::string, machine_t *> _machines;

    // tool_wire_name -> machine_t *
    std::map<std::string, std::vector<machine_t *> > _tool_wire_machines;

    std::map<std::string, std::vector<machine_t *> > _tool_machines;

    std::map<std::string, std::vector<machine_t *> > _wire_machines;

    list_operations_t *list_ops;
    machine_base_operations_t *machine_ops;
    job_base_operations_t *job_ops;

    void _init(scheduling_parameters_t param);

public:
    machines_t();

    machines_t(scheduling_parameters_t param);

    machines_t(machines_t &other);

    void addMachine(machine_t machine);

    void addPrescheduledJob(job_t *job);

    void prescheduleJobs();

    ~machines_t();
};


#endif
