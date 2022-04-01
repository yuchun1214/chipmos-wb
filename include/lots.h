//
// Created by Eugene on 2021/7/5.
//
#ifndef __LOTS_H__
#define __LOTS_H__

#include <sys/stat.h>
#include <map>
#include <string>
#include <vector>

#include "include/da.h"
#include "include/entities.h"
#include "include/entity.h"
#include "include/infra.h"
#include "include/lot.h"
#include "include/route.h"

class lots_t;

typedef void (lots_t::*traversing_function)(lot_t &lot,
                                            std::vector<lot_t> &unfinished,
                                            std::vector<lot_t> &finished,
                                            std::vector<lot_t> &faulty_lot,
                                            da_stations_t &das);

/**
 * class lots_t, a container of lot
 * lots in lots_t are classified to several group by their tool name and wire
 * name.
 */
class lots_t
{
protected:
    // for sys log
    int _total_number_of_wip;
    int _total_number_of_unscheduled_jobs;
    std::string _base_time;
    std::map<std::string, std::vector<std::string> > _parent_lots_and_sublots;

    std::vector<lot_t *> lots;
    std::vector<lot_t *> prescheduled_lots;
    // std::vector<lot_t> lots;
    // std::vector<lot_t> prescheduled_lots;
    std::map<std::string, std::vector<lot_t *> > wire_lots;
    std::map<std::string, std::vector<lot_t *> > tool_lots;
    std::map<std::string, std::vector<lot_t *> > tool_wire_lots;
    std::map<std::string, int> amount_of_wires;
    std::map<std::string, int> amount_of_tools;
    std::set<std::string> _automotive_lot_numbers;

    std::vector<traversing_function> _traversing_functions;
    void _traversingError(lot_t &lot,
                          std::vector<lot_t> &unfinished,
                          std::vector<lot_t> &finished,
                          std::vector<lot_t> &faulty_lot,
                          da_stations_t &das);
    void _traversingFinished(lot_t &lot,
                             std::vector<lot_t> &unfinished,
                             std::vector<lot_t> &finished,
                             std::vector<lot_t> &faulty_lot,
                             da_stations_t &das);
    void _traversingDAArrived(lot_t &lot,
                              std::vector<lot_t> &unfinished,
                              std::vector<lot_t> &finished,
                              std::vector<lot_t> &faulty_lot,
                              da_stations_t &das);
    void _traversingDAUnarrived(lot_t &lot,
                                std::vector<lot_t> &unfinished,
                                std::vector<lot_t> &finished,
                                std::vector<lot_t> &faulty_lot,
                                da_stations_t &das);
    void _traversingDADecrement(lot_t &lot,
                                std::vector<lot_t> &unfinished,
                                std::vector<lot_t> &finished,
                                std::vector<lot_t> &faulty_lot,
                                da_stations_t &das);

    /**
     * createLots () - create a vector of lots by reading and mapping the
     * information from files
     * @param wip_file_name : for wip
     * @param prod_pid_bomid_filename : product_id maps to process_id and bom_id
     * @param eim_lot_size_filename : get the lot size from eim
     * @param fcst_filename : D/A forecast
     * @param routelist_filename : route list
     * @param queue_time_filename : queue time
     * @param bomlist_filename : bom list
     * @param pid_heatblock_filename : process_id maps to heatblock partno
     * @param ems_heatblock_filename : get the amount of heatblock
     * @param gw_filename : get the number of rolls of wire
     * @param bdid_mapping_models_filename : bdid maps to its available models
     * @param uph_filename : uph
     * @return
     */
    std::vector<lot_t *> createLots(std::string wip_file_name,
                                    std::string prod_pid_bomid_filename,
                                    std::string eim_lot_size_filename,
                                    std::string fcst_filename,
                                    std::string routelist_filename,
                                    std::string queue_time_filename,
                                    std::string bomlist_filename,
                                    std::string pid_heatblock_filename,
                                    std::string ems_heatblock_filename,
                                    std::string gw_filename,
                                    std::string wire_stock_filename,
                                    std::string bdid_mapping_models_filename,
                                    std::string uph_filename,
                                    std::string cure_time_filename,
                                    std::string location_filename,
                                    std::string dir_suffix);

    void readLocation(std::string filename,
                      std::vector<lot_t> &lots,
                      std::vector<lot_t> &faulty_lots);

    /**
     * readWip () - read wip filename
     * read the wip data from the function and check if wip data has
     * completeness. If wip data doesn't have completeness, the lot will be
     * pushed to faulty_lots
     *
     * @param filename : wip filename
     * @param lots
     * @param faulty_lots
     */
    void readWip(std::string filename,
                 std::vector<lot_t> &lots,
                 std::vector<lot_t> &faulty_lots);

    /**
     * setPidBomId () - get lot's process_id and bom_id from a file
     *
     * Use lot's product id to map the process_id and bom_id. If lot doesn't
     * have completeness, lot will be pushed into faulty_lots.
     *
     * @param filename :
     * @param lots
     * @param faulty_lots
     */
    void setPidBomId(std::string filename,
                     std::vector<lot_t> &lots,
                     std::vector<lot_t> &faulty_lots);

    /**
     * setLotSize () - set lot's lot size for splitting the parent lots
     *
     * If lot is parent lot, lot will be split on D/A station by its lot size.
     * @param filename
     * @param lots
     * @param faulty_lots
     */
    void setLotSize(std::string filename,
                    std::vector<lot_t> &lots,
                    std::vector<lot_t> &faulty_lots);
    /**
     * setupRoute () - read the route list file and the queue time file to setup
     * the route station and queue time in each station
     *
     * @param routelist
     * @param queuetime
     * @param routes
     */
    void setupRoute(std::string routelist,
                    std::string queuetime,
                    std::string eim_lot_size,
                    std::string cure_time,
                    route_t &routes);

    /**
     * wb7Filter () - check if lot is in scheduling plan (WB - 7)
     * If lot is in scheduling plan, the lot will be returned, otherwise, the
     * lot is pushed into dontcare vector.
     *
     * @param alllots
     * @param dontcare
     * @param routes
     * @return
     */
    std::vector<lot_t> wb7Filter(std::vector<lot_t> alllots,
                                 std::vector<lot_t> &dontcare,
                                 route_t &routes);

    /**
     * queueTimeAndQueue () - sum the queue time for each lot.
     * In the function, each lot will traverse its route until arriving W/B
     * station. In the traversal, queue time is summed. The lot pauses its
     * traversal when it arrives D/A station. If lot raverses to D/A station,
     * lot is split to several sub-lot by its lot size. Every lots being in D/A
     * station lined up and the D/A station distributes the production capacity
     * to determined which lot will pass station in 24 hours.
     *
     * @param lots
     * @param faulty_lots
     * @param dontcare
     * @param das
     * @param routes
     * @return
     */
    std::vector<lot_t> queueTimeAndQueue(std::vector<lot_t> lots,
                                         std::vector<lot_t> &faulty_lots,
                                         std::vector<lot_t> &dontcare,
                                         da_stations_t &das,
                                         route_t &routes);

    /**
     * setCanRunModels () - setup can run models
     * If lots has no can run models, lot will be collected into faulty_lots
     * @param bdid_model_mapping_models_filename
     * @param lots
     * @param faulty_lots
     */
    void setCanRunModels(std::string bdid_model_mapping_models_filename,
                         std::vector<lot_t> &lots,
                         std::vector<lot_t> &faulty_lots);

    /**
     * setPartId () - setup partId
     * If lot has no mapping relationship between bom_id and part_id, the lot
     * will be collected into faulty_lots.
     * @param filename
     * @param lots
     * @param faulty_lots
     */
    void setPartId(std::string filename,
                   std::vector<lot_t> &lots,
                   std::vector<lot_t> &faulty_lots);

    /**
     * setAmountOfWire () - read the file and set the number of rolls of wire
     * @param filename
     * @param wire_stock_filename
     * @param lots
     * @param faulty_lots
     */
    void setAmountOfWire(std::string gw_filename,
                         std::string wire_stock_filename,
                         std::vector<lot_t> &lots,
                         std::vector<lot_t> &faulty_lots);

    /**
     * setPartNo () - setup PartNo
     * If lot has no mapping relationship between process_id to its part_no, the
     * lot will be collected into faulty_lots.
     * @param filename
     * @param lots
     * @param faulty_lots
     */
    void setPartNo(std::string filename,
                   std::vector<lot_t> &lots,
                   std::vector<lot_t> &faulty_lots);

    /**
     * setAmountOfTools () - read the file and set the number of heat block.
     * @param filename
     * @param lots
     * @param faulty_lots
     */
    void setAmountOfTools(std::string filename,
                          std::vector<lot_t> &lots,
                          std::vector<lot_t> &faulty_lots);

    /**
     * setUph () - read uph file and set the model's uph to each lot
     * The function will call lot_t::setUph to set the uph for each kind of
     * model
     * @param uph_file_name
     * @param lots
     * @param faulty_lots
     */
    void setUph(std::string uph_file_name,
                std::vector<lot_t> &lots,
                std::vector<lot_t> &faulty_lots);


public:
    lots_t();

    /**
     * addLots () - add the lots
     * If lot's creation isn't from lots_t::createLots, addLots can accept a
     * vector of lots having complete information from external function.
     * @param lots
     */
    void addLots(std::vector<lot_t *> lots);

    void pushBackNotPrescheduledLot(lot_t *lot);

    virtual void createLots(std::map<std::string, std::string> files);


    /**
     * amountOfWires () - return the amount of each sorts of wires
     *
     * @return a std::map container which maps the part_id to the number of this
     * kinds of wires
     */
    std::map<std::string, int> amountOfWires();

    /**
     * amuntOfTools () - return the amount of each sorts of tools
     *
     * @return a std::map container which maps the part_no to the number of this
     * kinds of tools
     */
    std::map<std::string, int> amountOfTools();

    std::vector<lot_t *> prescheduledLots();

    std::map<std::string, std::vector<lot_t *> > getLotsRecipeGroups();

    inline std::set<std::string> getAutomotiveLots();

    inline void setProcessTimeRatio(double ratio);

    inline std::map<std::string, std::vector<std::string> >
    getParentLotAndSubLots();

    std::vector<lot_t *> getAllLots();

    inline int totalNumberOfWip() { return _total_number_of_wip; }
    inline int totalNumberOfUnscheduledJobs()
    {
        return _total_number_of_unscheduled_jobs;
    }
};

inline std::vector<lot_t *> lots_t::getAllLots()
{
    return lots;
}

inline std::map<std::string, std::vector<std::string> >
lots_t::getParentLotAndSubLots()
{
    return _parent_lots_and_sublots;
}

inline std::set<std::string> lots_t::getAutomotiveLots()
{
    return _automotive_lot_numbers;
}

inline void lots_t::setProcessTimeRatio(double ratio)
{
    foreach (lots, i) {
        lots[i]->setProcessTimeRatio(ratio);
    }

    foreach (prescheduled_lots, i) {
        prescheduled_lots[i]->setProcessTimeRatio(ratio);
    }
}

inline std::vector<lot_t *> lots_t::prescheduledLots()
{
    return prescheduled_lots;
}

inline std::map<std::string, int> lots_t::amountOfWires()
{
    return amount_of_wires;
}

inline std::map<std::string, int> lots_t::amountOfTools()
{
    return amount_of_tools;
}


#endif
