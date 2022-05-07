#include <chrono>
#include <csignal>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <map>
#include <string>
#include <thread>

#include "include/algorithm.h"
#include "include/arg_parser.h"
#include "include/csv.h"
#include "include/entity.h"
#include "include/infra.h"
#include "include/lot.h"
#include "include/lots.h"
#include "include/machine_base.h"
#include "include/machines.h"
#include "include/population.h"
#include "include/record_gap.h"
#include "include/system_log.h"

using namespace std;
using namespace std::chrono;

map<string, string> outputJob(job_t job);
map<string, string> outputJobInMachine(machine_t *machine);

void outputJobInMachine(map<string, machine_t *>, csv_t *csv);

lots_t createLots(map<string, string>);
entities_t createEntities(map<string, string>);

std::map<std::pair<std::string, std::string>, double>
read_transportation_time_table(std::string file_name);

typedef struct __thread_data_t {
    map<string, string> arguments;
    int argc;
    const char **argv;
    int fd;
    int id;
} thread_data_t;
void run(thread_data_t *data);

char MESSAGE[] =
    "version 0.0.6\n"
    "Author : NCKU Smart Production Lab";

argument_parser_t *parser = new argument_parser_t();

bool stop = false;

int main(int argc, const char **argv)
{
    parser->add_args({"", "setup input file", ARG_STRING, NULL},
                     {"-f"s, "--file"s});
    parser->add_args({"", "only preprocess the files", ARG_NONE, NULL},
                     {"-p"s, "--preprocessing"s});
    parser->add_args({"", "list the available arguments\n", ARG_NONE, NULL},
                     {"-h"s, "--help"s});
    parser->add_args(
        {"", "rerun and skip the file preprocessing process", ARG_NONE, NULL},
        {"-r"s, "--rerun"s});

    parser->parse_argument_list(argc, argv);

    if (parser->is_set("-h")) {
        parser->print_arg_description();
        exit(0);
    }

    string file_name = parser->get_argument_value("--file");
    if (file_name.length() == 0) {
        printf("%s\n", MESSAGE);
        printf("Please give the --file argument");
        return 0;
    }

#ifdef WIN32
    signal(SIGTERM, [](int sig_num) { stop = true; });
#else
    signal(SIGTSTP, [](int sig_num) { stop = true; });
#endif

    csv_t cfg(file_name, "r", true, true);
    // get the cfg size
    int nthreads = cfg.nrows();
    vector<thread> threads;

    thread_data_t **thread_data_array =
        (thread_data_t **) malloc(sizeof(thread_data_t *) * nthreads);


    srand(200);
    for (unsigned int i = 0; i < cfg.nrows(); ++i) {
        thread_data_array[i] = new thread_data_t();
        *thread_data_array[i] = thread_data_t{.arguments = cfg.getElements(i),
                                              .argc = argc,
                                              .argv = argv,
                                              .fd = 1,
                                              .id = (int) i};
        printf("Thread[%d] starts\n", i);
        threads.push_back(thread(run, thread_data_array[i]));
    }

    for (unsigned int i = 0, size = threads.size(); i < size; ++i) {
        threads[i].join();
        printf("Thread[%d] is finished\n", i);
    }

    return 0;
}

void run(thread_data_t *data)
{
    map<string, string> arguments = data->arguments;
    const string DIRECTORY_NAME = "output_" + arguments["no"];
    sys_log_t *syslog = new sys_log_t(DIRECTORY_NAME + "/log.txt");
    // record the time;
    high_resolution_clock::time_point start_time, end_time;
    start_time = high_resolution_clock::now();

    lots_t lots = createLots(arguments);
    entities_t entities = createEntities(arguments);

    // record the sublots
    syslog->setSublot(lots.getParentLotAndSubLots());
    // record the number of available machine
    syslog->setNumberOfAvailableMachines(entities.allEntities().size());
    // record the number of unavailable machines
    syslog->setNumberOfUnavailableMachines(entities.getFaultyEntities().size());
    // record the cure time
    auto recordCureTimeAndSuperHotRunAndPrescheduledMachines = [&]() {
        vector<lot_t *> alllots = lots.getAllLots();
        set<string> prescheduled_entities;
        int number_of_super_hot_run = 0;
        foreach (alllots, i) {
            syslog->setCureTimeForSingleLot(
                {alllots[i]->lotNumber(), alllots[i]->cureTime()});
            if (alllots[i]->sprHot())
                ++number_of_super_hot_run;
        }

        vector<lot_t *> prescheduled_lots = lots.prescheduledLots();
        foreach (prescheduled_lots, i) {
            string prescheduled_entity =
                prescheduled_lots[i]->preScheduledEntity();
            prescheduled_entities.insert(prescheduled_entity);
            if (prescheduled_lots[i]->sprHot())
                ++number_of_super_hot_run;
        }
        syslog->setNumberOfSuperHotRunLots(number_of_super_hot_run);
        syslog->setPrescheduledMachines(vector<string>(
            prescheduled_entities.begin(), prescheduled_entities.end()));
    };
    recordCureTimeAndSuperHotRunAndPrescheduledMachines();
    syslog->setNumberOfTools(lots.amountOfTools());
    syslog->setWipTotalNumber(lots.totalNumberOfWip());
    syslog->setNumberOfUnscheduledJobs(lots.totalNumberOfUnscheduledJobs());

    auto timeElapse = [&]() {
        end_time = high_resolution_clock::now();
        duration<double> elapsed_time =
            duration_cast<duration<double> >(end_time - start_time);
        return elapsed_time.count();
    };
    if (parser->is_set("-p")) {
        syslog->setSysTimeElapse(timeElapse());
        syslog->output();
        pthread_exit(NULL);
    }

    if (data->fd < 0)
        data->fd = 1;

    population_t pop = population_t{
        .parameters =
            {
                .AMOUNT_OF_CHROMOSOMES = 100,
                .AMOUNT_OF_R_CHROMOSOMES = 200,
                .EVOLUTION_RATE = 0.8,
                .SELECTION_RATE = 0.2,
                .GENERATIONS = stoi(arguments["times"]),
                .MAX_SETUP_TIMES = stoi(arguments["max_setup_times"]),
                .weights =
                    {
                        .WEIGHT_SETUP_TIMES =
                            stof(arguments["weight_setup_times"]),
                        .WEIGHT_TOTAL_COMPLETION_TIME =
                            stof(arguments["weight_total_completion_time"]),
                        .WEIGHT_MAX_SETUP_TIMES =
                            stof(arguments["weight_max_setup_times"]),
                        .WEIGHT_CR = stof(arguments["weight_cr"]),
                        .WEIGHT_TR = stof(arguments["weight_tr"]),
                    },
                .setup_times_parameters =
                    {
                        .TIME_CWN = stod(arguments["setup_time_cwn"]),
                        .TIME_CK = stod(arguments["setup_time_ck"]),
                        .TIME_EU = stod(arguments["setup_time_eu"]),
                        .TIME_MC = stod(arguments["setup_time_mc"]),
                        .TIME_SC = stod(arguments["setup_time_sc"]),
                        .TIME_CSC = stod(arguments["setup_time_csc"]),
                        .TIME_USC = stod(arguments["setup_time_usc"]),
                        .TIME_ICSI = stod(arguments["setup_time_icsi"]),
                    },
                .scheduling_parameters =
                    {.PEAK_PERIOD = stof(arguments["peak_period"]),
                     .MAX_SETUP_TIMES = stoi(arguments["max_setup_times"]),
                     .MINUTE_THRESHOLD = stoi(arguments["minute_threshold"])},
                .transportation_time_table = read_transportation_time_table(
                    arguments["transportation_time_table"]),
            },
    };


    machine_base_operations_initializer_t ops_init(
        {setupTimeCWN, setupTimeCK, setupTimeEU, setupTimeMC, setupTimeSC,
         setupTimeCSC, setupTimeUSC},
        pop.parameters.setup_times_parameters);
    machine_base_operations_t *ops = ops_init.getOps();

    Record_gap rg(ops, DIRECTORY_NAME);

    machines_t *machines = new machines_t(
        pop.parameters.setup_times_parameters,
        pop.parameters.transportation_time_table, pop.parameters.weights);

    machines->setThreshold(
        pop.parameters.scheduling_parameters.MINUTE_THRESHOLD);

    vector<entity_t *> all_entities = entities.allEntities();
    foreach (all_entities, i) {
        machines->addMachine(all_entities[i]->machine());
    }
    machines->setMachineConstraintA(entities.getMachineConstraintA());
    machines->setMachineConstraintR(entities.getMachineConstraintR());

    prescheduling(machines, &lots);
    int stage2_setup_times = stage2Scheduling(
        machines, &lots, pop.parameters.scheduling_parameters.PEAK_PERIOD);
    pop.parameters.scheduling_parameters.MAX_SETUP_TIMES -= stage2_setup_times;
    stage3Scheduling(machines, &lots, &pop, data->fd);

    csv_t result(DIRECTORY_NAME + "/result.csv", "w");
    auto outputBatchOfJobs = [&](auto jobs, int size) {
        for (int i = 0; i < size; ++i) {
            result.addData(outputJob(*jobs[i]));
            rg.addJob(jobs[i]);
        }
    };

    vector<job_t *> scheduled_jobs = machines->getScheduledJobs();
    vector<job_t *> on_machine_jobs = machines->getOnMachineJobs();
    outputBatchOfJobs(scheduled_jobs, scheduled_jobs.size());
    outputBatchOfJobs(on_machine_jobs, on_machine_jobs.size());
    outputBatchOfJobs(pop.objects.jobs, pop.objects.NUMBER_OF_JOBS);
    rg.record_gap_all_machines();
    result.write();

    syslog->setNumberOfScheduledJobs(scheduled_jobs.size() +
                                     pop.objects.NUMBER_OF_JOBS);

    // for (int i = 0; i < pop.objects.NUMBER_OF_JOBS; ++i) {
    //     result.addData(outputJob(*pop.objects.jobs[i]));
    //     rg.addJob(pop.objects.jobs[i]);
    // }

    syslog->setSysTimeElapse(timeElapse());
    syslog->output();
}



lots_t createLots(map<string, string> arguments)
{
    lots_t lots;
    lot_t *lot;
    if (parser->is_set("-r")) {
        string lots_csv_file_path = arguments["lots"];
        csv_t lots_csv(lots_csv_file_path, "r");
        try {
            lots_csv.read();
        } catch (exception &e) {
            cerr << "The pre-created lots.csv is not found, the file path is "
                 << lots_csv_file_path << endl;
            exit(EXIT_FAILURE);
        }
        vector<lot_t *> all_lots;
        for (int i = 0, nrows = lots_csv.nrows(); i < nrows; ++i) {
            lot = new lot_t(lots_csv.getElements(i));
            all_lots.push_back(lot);
        }
        lots.addLots(all_lots);
    } else {
        // printf("Create lots by using configure file : %s\n", argv[1]);
        lots.createLots(arguments);
    }
    lots.setProcessTimeRatio(stod(arguments["process_time_ratio"]));

    return lots;
}

entities_t createEntities(map<string, string> arguments)
{
    csv_t machine_csv(arguments["machines"], "r", true, true);
    machine_csv.trim(" ");
    machine_csv.setHeaders(map<string, string>({{"entity", "ENTITY"},
                                                {"model", "MODEL"},
                                                {"recover_time", "OUTPLAN"},
                                                {"in_time", "IN TIME"},
                                                {"prod_id", "PRODUCT"},
                                                {"pin_package", "PIN_PKG"},
                                                {"lot_number", "LOT#"},
                                                {"customer", "CUST"},
                                                {"bd_id", "BOND ID"},
                                                {"oper", "SETTING OPER"},
                                                {"qty", "WIP"},
                                                {"uph", "G.UPH"}}));

    csv_t location_csv(arguments["locations"], "r", true, true);
    location_csv.trim(" ");
    location_csv.setHeaders(
        map<string, string>({{"entity", "Entity"}, {"location", "Location"}}));

    setup_time_parameters_t setup_time_params = {
        .TIME_CWN = stod(arguments["setup_time_cwn"]),
        .TIME_CK = stod(arguments["setup_time_ck"]),
        .TIME_EU = stod(arguments["setup_time_eu"]),
        .TIME_MC = stod(arguments["setup_time_mc"]),
        .TIME_SC = stod(arguments["setup_time_sc"]),
        .TIME_CSC = stod(arguments["setup_time_csc"]),
        .TIME_USC = stod(arguments["setup_time_usc"]),
        .TIME_ICSI = stod(arguments["setup_time_icsi"]),
    };

    machine_base_operations_initializer_t ops_init(
        {setupTimeCWN, setupTimeCK, setupTimeEU, setupTimeMC, setupTimeSC,
         setupTimeCSC, setupTimeUSC},
        setup_time_params);

    machine_base_operations_t *ops = ops_init.getOps();
    entities_t entities(arguments);
    entities.addMachines(machine_csv, location_csv, ops);
    return entities;
}

map<string, string> outputJob(job_t job)
{
    return map<string, string>({{"lot_number", job.base.job_info.data.text},
                                {"bd_id", job.bdid.data.text},
                                {"part_no", job.part_no.data.text},
                                {"part_id", job.part_id.data.text},
                                {"cust", job.customer.data.text},
                                {"pin_pkg", job.pin_package.data.text},
                                {"prod_id", job.prod_id.data.text},
                                {"qty", to_string(job.base.qty)},
                                {"entity", job.base.machine_no.data.text},
                                {"start_time", to_string(job.base.start_time)},
                                {"end_time", to_string(job.base.end_time)},
                                {"oper", to_string(job.oper)},
                                {"arrival_time", to_string(job.base.arriv_t)},
                                {"process_time", to_string(job.base.ptime)}});
}

map<string, string> outputJobInMachine(machine_t *machine)
{
    return outputJob(machine->current_job);
}

void outputJobInMachine(map<string, machine_t *> machines, csv_t *csv)
{
    for (map<string, machine_t *>::iterator it = machines.begin();
         it != machines.end(); it++) {
        if (it->second->current_job.prod_id.text_size != 0)
            csv->addData(outputJob(it->second->current_job));
    }
}

std::map<std::pair<std::string, std::string>, double>
read_transportation_time_table(std::string file_name)
{
    csv_t location(file_name, "r", true, false);
    location.dropNullRow();
    location.trim(" ");

    std::map<std::pair<std::string, std::string>, double> data;

    for (int i = 1, size = location.nrows(); i < size; ++i)
        for (int j = 1; j < size; ++j)
            if (i <= j) {
                data.insert(pair<pair<std::string, std::string>, double>(
                    pair<std::string, std::string>(location.getElement(j, 0),
                                                   location.getElement(0, i)),
                    stof(location.getElement(j, i))));
                data.insert(pair<pair<std::string, std::string>, double>(
                    pair<std::string, std::string>(location.getElement(0, i),
                                                   location.getElement(j, 0)),
                    stof(location.getElement(j, i))));
            }
    return data;
}
