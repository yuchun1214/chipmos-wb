#ifndef __LOT_BASE_H__
#define __LOT_BASE_H__

#include <map>
#include <string>
#include <vector>
#include "include/job_base.h"
#include "include/linked_list.h"


class lot_base_t : protected job_base_t, public list_ele_t
{
protected:
    std::string _lot_number;
    std::string _pin_package;
    std::string _recipe;
    std::string _prod_id;
    std::string _part_id;
    std::string _part_no;
    std::string _pkg_id;
    std::string _customer;

    int _qty;
    int _oper;
    int _number_of_wires;
    int _number_of_tools;

    bool _hold;
    bool _mvin;
    bool _is_sub_lot;
    bool _is_automotive;
    bool _spr_hot;

    double _cr;

    std::vector<std::string> _log;
    std::vector<std::string> _can_run_models;
    std::vector<std::string> _can_run_locations;

    std::map<std::string, double> _uphs;
    std::map<std::string, double> _model_process_times;

    std::map<std::string, int> _tools;

    virtual std::map<std::string, std::vector<std::string> >
    getRowDefaultValues() const;
    void _setupDefaultValueOfRow(std::map<std::string, std::string> &row);

public:
    lot_base_t();
    lot_base_t(std::map<std::string, std::string> &row);

    // getter
    inline std::string getLotNumber() { return _lot_number; }
    inline std::string getPinPackage() { return _pin_package; }
    inline std::string getRecipe() { return _recipe; }
    inline std::string getProdId() { return _prod_id; }
    inline std::string getPartId() { return _part_id; }
    inline std::string getPartNo() { return _part_no; }
    inline std::string getPkgId() { return _pkg_id; }
    inline std::string getCustomer() { return _customer; }

    inline int getQty() { return _qty; }
    inline int getOper() { return _oper; }
    inline int getNumberOfWires() { return _number_of_wires; }
    inline int getNumberOfTools() { return _number_of_tools; }

    inline bool isHold() { return _hold; }
    inline bool isMoveIn() { return _mvin; }
    inline bool isSubLot() { return _is_sub_lot; }
    inline bool isAutomotive() { return _is_automotive; }
    inline bool isSuperHot() { return _spr_hot; }
    inline double getCRValue() { return _cr; }
};
#endif
