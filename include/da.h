#ifndef __DA_H__
#define __DA_H__

#include <include/csv.h>
#include <include/infra.h>
#include <include/lot.h>
#include <string>
#include <vector>

/**
 * @struct da_station_t
 * @brief store the information about a D/A station
 *
 * da_station_t is used to store the information such as how much die will be
 * processed in D/A station and the lot which is arrived or unarrived in D/A
 * station. These information is used to predict that if lot will be processed
 * in D/A station in 24 hours or not.
 *
 * @var fcst : the amount of die expect to be processed in D/A station in 24
 * hours
 * @var act : the actual amount of die which is processed in D/A station.
 * @var remain : the remain amount of die which is going to be processed.
 * @var upm : unit per minute. How many die are processed in a minute.
 * @var finished : does da_station_t finish arranging the production capacity.
 * @var arrived : store the lot which is waiting in D/A station
 * @var unarrived : store the lot which is unarrived to D/A station.
 * @var remaining : store the lot which will not be processed in D/A in 24
 * hours.
 */
typedef struct da_station_t {
    double fcst;
    double act;
    double remain;
    double upm;
    double time;
    bool finished;
    std::vector<lot_t> arrived;
    std::vector<lot_t> unarrived;
    std::vector<lot_t> remaining;
} da_station_t;

/**
 * @class da_stations_t
 * @brief used to manage all da_station_t objects
 *
 * da_stations_t object manages all da_station_t objects. The program use the
 * production capacity to predict whether the lot will pass D/A station in 24
 * hours or not. The classification lot in D/A station is by its recipe(bd_id).
 * If lot is in D/A station, lots will be added in arrived queue and if lot is
 * not in D/A station but the lot has traversed to D/A station, the lot will be
 * added into unarrived queue.
 */
class da_stations_t
{
private:
    /// _da_stations_container is a container which classify the da_station_t
    /// instances by bd_id
    std::map<std::string, da_station_t> _da_stations_container;


    std::vector<lot_t> _parent_lots;
    std::map<std::string, std::vector<std::string> > _parent_lots_and_sublots;

    /**
     * daDistributeCapacity () - distribute the production capacity of a station
     * @param da : for a single station
     * @return the lots which successfully passed D/A station after distributing
     * production capacity.
     */
    std::vector<lot_t> daDistributeCapacity(
        da_station_t &da);  // for single da_station

    /**
     * splitSubLots () - split parent lots to several sub-lots
     * @param lots
     * @return the sub-lots
     */
    std::vector<lot_t> splitSubLots(std::vector<lot_t> lots);


public:
    da_stations_t(csv_t fcst);

    /**
     * setFcst () setup the forecast production capacity of D/A station
     * In the function, if the bd_id is duplicated, sum the value.
     */
    void setFcst(csv_t _fcst);

    /**
     *
     * @param lot
     * @return
     */
    bool addArrivedLotToDA(lot_t &lot);
    bool addUnarrivedLotToDA(lot_t &lot);

    std::vector<lot_t> distributeProductionCapacity();

    void removeAllLots();

    std::vector<lot_t> getParentLots();

    void decrementProductionCapacity(lot_t &lot);

    std::map<std::string, std::vector<std::string> > getParentLotAndSubLots();

    std::vector<lot_t> get_all_remaining_lots();
};

inline std::vector<lot_t> da_stations_t::getParentLots()
{
    return _parent_lots;
}

inline std::map<std::string, std::vector<std::string> >
da_stations_t::getParentLotAndSubLots()
{
    return _parent_lots_and_sublots;
}

#endif
