#include <algorithm>
#include <array>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <fcntl.h>
#include <sys/stat.h>

/* Directory needed : output_X-X */
/* Files needed in same directory */
#include "include/csv.h"
#include "include/time_converter.h"

#define DIR_OUTPUT "result"

/* Files needed in same directory */
//#define PATH_MACHINE "machines.csv"
#define PATH_CONFIG "config.csv"
#define PATH_RECORD_GAP "record_gap.csv"
#define PATH_NCKU_RESULT "result.csv"

#define PATH_OUTPUT "indicator.csv"

#define BASE_TIME "07:30"
#define START_DATE "22-02-01"
#define END_DATE "22-02-02"

#define MAX(x, y) ((x) ^ (((x) ^ (y)) & -((x) < (y))))

#define MIN(x, y) ((x) ^ (((x) ^ (y)) & -((x) > (y))))

class wlth_info_t
{
public:
    wlth_info_t(std::map<std::string, std::string> const &elements, long rtime)
        : qty(std::stoi(elements.at("qty"))),
          lot(elements.at("lot")),
          entity(elements.at("entity")),
          start(static_cast<time_t>(std::stod(elements.at("start")) * 60) -
                rtime),
          end(static_cast<time_t>(std::stod(elements.at("end")) * 60) -
              rtime){};

    int qty;
    std::string lot, entity;
    time_t start, end;
};

class setup_info_t
{
public:
    setup_info_t(std::map<std::string, std::string> const &elements, long rtime)
        : entity(elements.at("entity")),
          start(static_cast<time_t>(std::stod(elements.at("start")) * 60) -
                rtime){};
    std::string entity;
    time_t start;
};

int main()
{
    // mkdir(DIR_OUTPUT, 0777);
    /* Get relative time */
    csv_t conf(PATH_CONFIG, "r", true, true);
    conf.dropNullRow();
    conf.trim(" ");
    conf.setHeaders(std::map<std::string, std::string>({
        {"base_time", "std_time"},
    }));

    std::vector<time_t> rtime;
    for (int i = 0, size = conf.nrows(); i < size; ++i) {
        std::string rtimes = conf.getElements(i)["base_time"];
        rtimes.replace(rtimes.find(" ") + 1, 5, BASE_TIME);
        rtime.push_back(
            timeConverter(conf.getElements(i)["base_time"])(rtimes));
    }

    std::ofstream outfile(PATH_OUTPUT);
    outfile << "No,Utilization,Outplan,Setup" << std::endl;
    std::cout << "No,Utilization,Outplan,Setup" << std::endl;
    /* Read result.csv */
    for (int dir = 0, size = conf.nrows(); dir < size; ++dir) {
        csv_t data(
            "output_" + conf.getElements(dir)["no"] + "/" + PATH_NCKU_RESULT,
            "r", true, true);
        data.dropNullRow();
        data.trim(" ");
        data.setHeaders(
            std::map<std::string, std::string>({{"lot", "lot_number"},
                                                {"start", "start_time"},
                                                {"end", "end_time"}}));

        /* Assume start_time, end_time currently in minutes */
        std::vector<wlth_info_t> wlths;
        for (int i = 0, size = data.nrows(); i < size; ++i) {
            wlths.push_back(wlth_info_t(data.getElements(i), rtime.at(dir)));
        }

        /* entity - wlth pair */
        std::map<std::string, std::vector<wlth_info_t>> info;
        std::vector<wlth_info_t> temp;
        for (auto &cur : wlths) {
            if (info.find(cur.entity) == info.end())
                info.insert(std::pair<std::string, std::vector<wlth_info_t>>(
                    cur.entity, temp));
            info[cur.entity].push_back(cur);
        }

        /* output per simulation */
        /*
        std::ofstream outfile_utilization("result/utilization_" +
        conf.getElements(dir)["no"] + ".csv"); std::ofstream
        outfile_quantity("result/quantity_" + conf.getElements(dir)["no"] +
        ".csv");
        */
        time_t base_start = 0, base_end = timeConverter(START_DATE)(END_DATE);
        int gqty = 0;
        time_t gtotal = 0;
        for (auto &cur : info) {
            int lqty = 0;
            time_t total = 0;

            for (auto &it : cur.second) {
                if (it.end < base_start || it.start > base_end)
                    continue;
                total += MIN(it.end, base_end) - MAX(it.start, base_start);
                if (it.end <= base_end)
                    lqty += it.qty;
            }
            /*
            outfile_utilization << cur.first << ","
                << (long double)(total)/(long double)(base_end) << std::endl;

            outfile_quantity << cur.first << ","
                << lqty << std::endl;
            */
            gtotal += total;
            gqty += lqty;
        }

        int nmachine = 0;
        std::string line;
        std::string PATH_MACHINE = conf.getElements(dir)["machines"];
        std::ifstream infile(PATH_MACHINE);
        while (std::getline(infile, line)) {
            if (line.compare(0, 2, "BB") == 0)
                ++nmachine;
        }

        int gitotal =
            (static_cast<int>((long double) (gtotal) /
                              ((long double) (nmachine * base_end)) * 100000));
        /* output no, utility, quantity */
        outfile << conf.getElements(dir)["no"] << "," << gitotal / 1000 << "."
                << (((gitotal + 5) % 1000)) / 10 << "%,"
                << ((gqty + 50) / 100) / 10 << "." << ((gqty + 50) / 100) % 10
                << "K";

        std::cout << conf.getElements(dir)["no"] << "," << gitotal / 1000 << "."
                  << (((gitotal + 5) % 1000)) / 10 << "%,"
                  << ((gqty + 50) / 100) / 10 << "." << ((gqty + 50) / 100) % 10
                  << "K";

        // TODO : repeated code
        /* Read record_gap.csv */
        csv_t setup(
            "output_" + conf.getElements(dir)["no"] + "/" + PATH_RECORD_GAP,
            "r", true, true);
        setup.dropNullRow();
        setup.trim(" ");
        setup.setHeaders(
            std::map<std::string, std::string>({{"start", "start_time"}}));

        std::vector<setup_info_t> setupd;
        for (int i = 0, size = setup.nrows(); i < size; ++i) {
            setupd.push_back(setup_info_t(setup.getElements(i), rtime.at(dir)));
        }
        /* entity - setupd pair */
        std::map<std::string, std::vector<setup_info_t>> sinfo;
        std::vector<setup_info_t> stemp;
        for (auto &cur : setupd) {
            if (sinfo.find(cur.entity) == sinfo.end())
                sinfo.insert(std::pair<std::string, std::vector<setup_info_t>>(
                    cur.entity, stemp));
            sinfo[cur.entity].push_back(cur);
        }

        /*
        std::ofstream outfile_setup("result/setup_" +
        conf.getElements(dir)["no"] + ".csv");
        */

        /* output setup */
        int gsetup = 0;
        for (auto &cur : sinfo) {
            int lsetup = 0;
            for (auto &it : cur.second) {
                if (it.start < base_start || it.start > base_end)
                    continue;
                ++lsetup;
            }
            /*
            outfile_setup << cur.first << ","
                << lsetup << std::endl;
            */
            gsetup += lsetup;
        }

        outfile << "," << gsetup << std::endl;
        std::cout << "," << gsetup << std::endl;
    }

    return 0;
}
