//
// Created by YuChunLin on 2021/11/22.
//

#ifndef __MACHINE_CONSTRAINT_A__
#define __MACHINE_CONSTRAINT_A__

#include "include/machine_constraint.h"

class machine_constraint_a_t : public machine_constraint_t
{
private:
    machine_constraint_a_t(){};

protected:
    bool _isMachineRestrained(std::regex entity_re,
                              std::string restrained_model,
                              std::string entity_name,
                              std::string machine_model) override;
    bool _isMachineRestrained(constraint_oper_t &oper,
                              job_t *job,
                              machine_t *machine,
                              bool *care) override;
    bool _isMachineRestrainedForTheValue(
        std::vector<constraint_entry_t> entries,
        machine_t *machine) override;

public:
    explicit machine_constraint_a_t(csv_t csv);
    virtual ~machine_constraint_a_t();
};

#endif
