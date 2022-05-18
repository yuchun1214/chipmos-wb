#ifndef __ENTITY_H__
#define __ENTITY_H__

#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "include/csv.h"
#include "include/lot.h"
#include "include/machine.h"

enum machine_status {
    SETUP,
    QC,
    WAIT_SETUP,
    ENG,
    STOP,
    WAIT_REPAIR,
    IN_REPAIR,
    IDLE,
    PM,
    RUNNING,
};

class entity_t
{
private:
    double _recover_time;
    double _outplan_time;
    double _intime;
    double _setup_time;
    double _ptime;
    double _ent_weight;
    enum machine_status _status;
    std::string _entity_name;
    std::string _model_name;
    std::string _location;

    lot_t *_current_lot;

    std::vector<lot_t *> _prescheduled_lots;

public:
    entity_t();

    entity_t(std::map<std::string, std::string> elements,
             machine_base_operations_t *ops,
             time_t base_time = 0);

    double getRecoverTime() const;
    double getOutplanTime() const;
    std::string getEntityName();
    std::string getModelName();
    std::string getLocation();
    std::string getRecipe();

    void setBaseTime(time_t time);

    virtual machine_t machine();

    bool hold;
};

inline double entity_t::getRecoverTime() const
{
    return _recover_time;
}

inline double entity_t::getOutplanTime() const
{
    return _outplan_time;
}

inline std::string entity_t::getEntityName()
{
    return _entity_name;
}

inline std::string entity_t::getModelName()
{
    return _model_name;
}

inline std::string entity_t::getLocation()
{
    return _location;
}

inline std::string entity_t::getRecipe()
{
    return _current_lot->recipe();
}


#endif
