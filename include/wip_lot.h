#ifndef __LOT_WIP_H__
#define __LOT_WIP_H__

#include <ctime>
#include <string>
#include "include/lot_base.h"
#include "include/time_converter.h"

class lot_wip_t : public lot_base_t
{
protected:
    std::string _route;
    std::string _urgent_code;

    std::string _last_wb_entity;
    std::string _wb_location;
    std::string _wb_last_trans_str;

    time_t _wb_last_trans;
    int _sublot_size;

    virtual inline std::map<std::string, std::vector<std::string> >
    getRowDefaultValues() const override;

public:
    lot_wip_t();
    lot_wip_t(std::map<std::string, std::string> &row);

    inline std::string getRoute() { return _route; }
    inline std::string getUrgentCode() { return _urgent_code; }
    inline std::string getLastWbEntity() { return _last_wb_entity; }
    inline std::string getWBLocation() { return _wb_location; }
    inline std::string getWBLastTransTimeStr() { return _wb_last_trans_str; }

    inline time_t getWBLastTransTime() { return _wb_last_trans; }
    inline int getSubLotSize() { return _sublot_size; }
};

std::map<std::string, std::vector<std::string> >
lot_wip_t::getRowDefaultValues() const
{
    std::map<std::string, std::vector<std::string> > default_values =
        lot_base_t::getRowDefaultValues();

    default_values[""].push_back("route");
    default_values[""].push_back("urgent_code");
    default_values[""].push_back("last_wb_entity");
    default_values[""].push_back("wb_last_trans");
    default_values["1"].push_back("sub_lot_size");
    default_values["N"].push_back("mvin");
    default_values["N"].push_back("hold");
    return default_values;
}

#endif
