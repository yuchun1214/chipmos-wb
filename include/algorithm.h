//
// Created by eugene on 2021/7/5.
//

#ifndef __ALGORITHM_H__
#define __ALGORITHM_H__

#include "include/chromosome.h"
#include "include/chromosome_base.h"
#include "include/lots.h"
#include "include/machines.h"
#include "include/population.h"

extern bool stop;

void prescheduling(machines_t *machines, lots_t *lots);

int stage2Scheduling(machines_t *machines, lots_t *lots, double peak_period);

void stage3Scheduling(machines_t *machines,
                      lots_t *lots,
                      population_t *pop,
                      int fd);

void prepareChromosomes(chromosome_base_t **_chromosomes,
                        int NUMBER_OF_JOBS,
                        int NUMBER_OF_R_CHROMOSOMES);

void geneticAlgorithm(population_t *pop, int fd);

#endif
