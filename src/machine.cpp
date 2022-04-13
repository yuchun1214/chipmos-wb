#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>

#include "include/job_base.h"
#include "include/linked_list.h"
#include "include/machine.h"
#include "include/machine_base.h"
#include "include/parameters.h"


bool aresPtrComp(ares_t *a1, ares_t *a2)
{
    return a1->available_time > a2->available_time;
}

bool aresComp(ares_t a1, ares_t a2)
{
    return a1.time < a2.time;
}

void machineReset(machine_base_t *base)
{
    machine_t *m = (machine_t *) base->ptr_derived_object;
    machine_base_reset(base);
    m->makespan = 0;
    m->total_completion_time = 0;
}

double setupTimeCWN(job_base_t *_prev, job_base_t *_next, double time)
{
    if (_prev) {
        job_t *prev = (job_t *) _prev->ptr_derived_object;
        job_t *next = (job_t *) _next->ptr_derived_object;
        if (isSameInfo(prev->part_no, next->part_no))
            return 0.0;
        else
            return time;
    }
    return 0;
}

double setupTimeCK(job_base_t *_prev, job_base_t *_next, double time)
{
    if (_prev) {
        job_t *prev = (job_t *) _prev->ptr_derived_object;
        job_t *next = (job_t *) _next->ptr_derived_object;
        if (prev->part_no.data.text[5] == next->part_no.data.text[5]) {
            return 0.0;
        } else {
            return time;
        }
    }
    return 0;
}

double setupTimeEU(job_base_t *_prev, job_base_t *_next, double time)
{
    if (_next) {
        job_t *next = (job_t *) _next->ptr_derived_object;
        if (next->urgent_code)
            return time;
        else
            return 0;
    }
    return -1;
}

double setupTimeMC(job_base_t *_prev, job_base_t *_next, double time)
{
    if (_prev && _next) {
        job_t *prev = (job_t *) _prev->ptr_derived_object;
        job_t *next = (job_t *) _next->ptr_derived_object;
        if (prev->pin_package.data.number[0] ==
            next->pin_package.data.number[0]) {
            return time;
        }
    }
    return 0;
}

double setupTimeSC(job_base_t *_prev, job_base_t *_next, double time)
{
    if (_prev && _next) {
        job_t *prev = (job_t *) _prev->ptr_derived_object;
        job_t *next = (job_t *) _next->ptr_derived_object;
        if (prev->pin_package.data.number[0] !=
            next->pin_package.data.number[0]) {
            return time;
        }
    }
    return 0;
}

double setupTimeCSC(job_base_t *_prev, job_base_t *_next, double time)
{
    if (_next) {
        job_t *next = (job_t *) _next->ptr_derived_object;
        if (next->part_no.data.text[5] != 'A')
            return time;
    }
    return 0;
}

double setupTimeUSC(job_base_t *_prev, job_base_t *_next, double time)
{
    if (_next) {
        job_t *next = (job_t *) _next->ptr_derived_object;
        if (next->part_no.data.text[5] != 'A' && next->urgent_code == 'Y')
            return time;
        else
            return 0;
    }
    return 0;
}

double calculateSetupTime(job_t *prev,
                          job_t *next,
                          machine_base_operations_t *ops)
{
    // for all setup time function
    double time = 0;
    if (isSameInfo(prev->bdid, next->bdid))
        return 0.0;
    for (unsigned int i = 0; i < ops->sizeof_setup_time_function_array; ++i) {
        if (prev) {
            time += ops->setup_time_functions[i].function(
                &prev->base, &next->base, ops->setup_time_functions[i].minute);
        } else {
            time += ops->setup_time_functions[i].function(
                NULL, &next->base, ops->setup_time_functions[i].minute);
        }
    }
    return time;
}

ares_t *searchResource(resources_t res, info_t name)
{
    // FIXME : should perform binary search to optimize
    if (res.areses) {
        for (int i = 0; i < res.number; ++i) {
            if (isSameInfo(res.areses[i]->name, name)) {
                return res.areses[i];
            }
        }
    }
    return nullptr;
}

void scheduling(machine_t *machine,
                machine_base_operations_t *ops,
                weights_t weights,
                std::map<std::pair<std::string, std::string>, double>
                    &transportation_time_table,
                setup_time_parameters_t scheduling_parameters)
{
    // initialize tool and wire
    for (int i = 0; i < machine->tools.number; ++i) {
        machine->tools.areses[i]->time =
            machine->tools.areses[i]->available_time;
    }

    for (int i = 0; i < machine->wires.number; ++i) {
        machine->wires.areses[i]->time =
            machine->wires.areses[i]->available_time;
    }

    list_ele_t *it;
    job_t *job;
    job_t *prev_job = &machine->current_job;
    it = machine->base.root;
    double arrival_time;
    double setup_time;
    bool hasICSI = false;
    double start_time = machine->base.available_time;
    double total_completion_time = 0;
    double total_transportation_time = 0;
    int setup_times = 0;
    int setup_times_in1440 = 0;
    double cr_sum = 0;
    machine->setup_times = 0;
    ares_t *tool, *wire;
    while (it) {
        double tr_time = 0;
        tool = NULL;
        wire = NULL;
        job = (job_t *) it->ptr_derived_object;
        if (strlen(job->location.data.text) != 0) {
            // printf("Machine location :%s ->
            // %s\n",machine->base.machine_no.data.text,
            // machine->location.data.text); fflush(stdout);
            try {
                tr_time = transportation_time_table.at(std::make_pair(
                    std::string((char *) (job->location.data.text)),
                    std::string((char *) (machine->location.data.text))));
            } catch (std::out_of_range &e) {
                printf("Machine number : %s\n",
                       machine->base.machine_no.data.text);
                printf("Unable to find transportation pair %s -> %s",
                       job->location.data.text, machine->location.data.text);
                exit(EXIT_FAILURE);
            }

            total_transportation_time += tr_time;
        }

        arrival_time = job->base.arriv_t;
        setup_time = calculateSetupTime(prev_job, job, ops);
        if (setup_time != 0.0)
            ++setup_times;
        if (start_time < 1440) {
            ++setup_times_in1440;
        }
        if (!hasICSI) {
            if (strncmp(job->customer.data.text, "ICSI", 4) == 0) {
                setup_time += scheduling_parameters.TIME_ICSI;
                hasICSI = true;
            }
        }

        arrival_time += tr_time;
        start_time = (start_time + setup_time) > arrival_time
                         ? start_time + setup_time
                         : arrival_time;

        tool = searchResource(machine->tools, job->part_no);
        wire = searchResource(machine->wires, job->part_id);

        // if the tools and wires are loaded on the machine
        if (machine->tools.number && machine->wires.number) {
            if (machine->tools.areses != nullptr &&
                machine->wires.areses != nullptr) {
                double max_resource_time =
                    (tool->time > wire->time ? tool->time : wire->time);
                start_time =
                    (start_time > max_resource_time ? start_time
                                                    : max_resource_time);
                set_start_time(&job->base, start_time);
                start_time = get_end_time(&job->base);
                tool->time = start_time;
                wire->time = start_time;
            } else {
                printf(
                    "Tools and wires have number of resources, however, they "
                    "can't find the part_id or part_no");
                exit(EXIT_FAILURE);
            }
        } else {
            set_start_time(&job->base, start_time);
            start_time = get_end_time(&job->base);
        }


        cr_sum += job->cr * job->base.end_time;
        total_completion_time += start_time;
        prev_job = job;
        it = it->next;
    }
    machine->makespan = start_time;


    machine->total_completion_time = total_completion_time;
    // machine->quality =
    //     weights.WEIGHT_SETUP_TIMES * setup_times +
    //             weights.WEIGHT_TOTAL_COMPLETION_TIME * total_completion_time
    //             + weights.WEIGHT_MAX_SETUP_TIMES * (setup_times_in1440 > 180)
    //         ? setup_times_in1440
    //         : -10;

    machine->quality =
        weights.WEIGHT_SETUP_TIMES * setup_times +
        weights.WEIGHT_TOTAL_COMPLETION_TIME * total_completion_time +
        weights.WEIGHT_CR * cr_sum +
        weights.WEIGHT_TR * total_transportation_time;

    machine->setup_times = setup_times_in1440;
}


void setLastJobInMachine(machine_t *machine)
{
    list_ele_t *it = machine->base.root;
    // Protect the job declared below not to be nullptr in loop;
    if (!it)
        return;
    job_t *job = nullptr;
    while (it) {
        job = (job_t *) it->ptr_derived_object;
        it = it->next;
    }
    if (job != nullptr) {
        machine->current_job = *job;
        machine->current_job.base.ptr_derived_object = &machine->current_job;
        machine->current_job.list.ptr_derived_object = &machine->current_job;
        machine->base.available_time = job->base.end_time;
    }
}


void _insertHeadAlgorithm(machine_t *machine,
                          machine_base_operations_t *mbops,
                          weights_t weights,
                          std::map<std::pair<std::string, std::string>, double>
                              &transportation_time_table,
                          setup_time_parameters_t scheduling_parameters)
{
    list_ele_t *it = machine->base.root;
    list_ele_t *prev;
    list_ele_t *next;
    list_ele_t *head = it;

    job_t *job;
    double size = -1;

    // search the head segment
    // check if the machine has at least two jobs
    if (it == nullptr || it->next == nullptr)
        return;  // if the machine has only one job -> return immediately

    // get the first segment size
    job = (job_t *) it->ptr_derived_object;
    size = job->base.start_time - machine->base.available_time;

    it = it->next;
    while (it) {
        job = (job_t *) it->ptr_derived_object;
        // check if ptime is smaller then size
        if (job->base.ptime < size &&
            job->base.arriv_t < machine->base.available_time) {
            prev = it->prev;
            next = it->next;
            prev->next = next;
            if (next != nullptr)
                next->prev = prev;

            it->prev = nullptr;
            it->next = head;
            head->prev = it;
            machine->base.root = it;
            break;
        }
        it = it->next;
    }
    scheduling(machine, mbops, weights, transportation_time_table,
               scheduling_parameters);
}

void insertAlgorithm(machine_t *machine,
                     machine_base_operations_t *mbops,
                     weights_t weights,
                     std::map<std::pair<std::string, std::string>, double>
                         &transportation_time_table,
                     setup_time_parameters_t scheduling_parameters)
{
    _insertHeadAlgorithm(machine, mbops, weights, transportation_time_table,
                         scheduling_parameters);

    list_ele_t *it = machine->base.root;
    list_ele_t *prev, *next, *it2;
    job_t *it_job, *next_job, *it2_job;
    double size = -1;

    while (it && it->next) {
        it_job = (job_t *) it->ptr_derived_object;
        next_job = (job_t *) it->next->ptr_derived_object;
        size = next_job->base.start_time - it_job->base.end_time;
        if (size > 0) {
            it2 = it->next->next;
            while (it2) {
                it2_job = (job_t *) it2->ptr_derived_object;
                if (it2_job->base.ptime < size &&
                    it2_job->base.arriv_t <= it_job->base.end_time) {
                    prev = it2->prev;
                    next = it2->next;
                    prev->next = next;
                    if (next != nullptr)
                        next->prev = prev;

                    prev = it;
                    next = it->next;
                    prev->next = it2;
                    next->prev = it2;
                    it2->next = next;
                    it2->prev = prev;
                    scheduling(machine, mbops, weights,
                               transportation_time_table,
                               scheduling_parameters);
                    break;
                }
                it2 = it2->next;
            }
        }
        it = it->next;
    }
}

void setJob2Scheduled(machine_t *machine)
{
    list_ele_t *it = machine->base.root;
    job_t *job;
    while (it) {
        job = (job_t *) it->ptr_derived_object;
        job->is_scheduled = true;
        it = it->next;
    }
}

int staticAddJob(machine_t *machine,
                 job_t *job,
                 machine_base_operations_t *machine_ops)
{
    job_t *last_job_of_machine;
    if (machine->base.tail != nullptr) {
        last_job_of_machine = (job_t *) machine->base.tail->ptr_derived_object;
    } else {
        last_job_of_machine = &machine->current_job;
    };
    machine_ops->add_job(&machine->base, &job->list);
    double setup_time =
        calculateSetupTime(last_job_of_machine, job, machine_ops);
    // double start_time = (job->base.arriv_t > machine->base.available_time)
    //                         ? job->base.arriv_t
    //                         : machine->base.available_time;
    double start_time =
        (job->base.arriv_t - machine->base.available_time > setup_time
             ? job->base.arriv_t
             : machine->base.available_time + setup_time);

    set_start_time(&job->base, start_time);
    machine->base.available_time = get_end_time(&job->base);
    job->base.machine_no = machine->base.machine_no;
    if (start_time < 1440 && setup_time != 0)
        return 1;
    else
        return 0;
}
