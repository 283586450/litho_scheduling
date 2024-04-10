#include <fstream>

#include "ortools/sat/cp_model.h"
#include "solve_model.hpp"

namespace operations_research {
namespace sat {
void set_time_limit(SatParameters& parameters, int time_limit)
{
    parameters.set_max_time_in_seconds(time_limit);
}

void set_num_search_workers(SatParameters& parameters, int num_search_workers)
{
    parameters.set_num_search_workers(num_search_workers);
}

void enable_log_search_progress(SatParameters& parameters)
{
    parameters.set_log_search_progress(true);
}

void disable_log_search_progress(SatParameters& parameters)
{
    parameters.set_log_search_progress(false);
}

void add_parameters_to_model(Model& model, SatParameters& parameters)
{
    model.Add(NewSatParameters(parameters));
}

CpSolverResponse solve_model(Model& model, CpModelBuilder& cp_model)
{

    CpSolverResponse response = SolveCpModel(cp_model.Build(), &model);

    return response;
}

void print_obj_val(const CpSolverResponse& response)
{
    if (response.status() == CpSolverStatus::OPTIMAL) {
        std::cout << "Optimal solution found! ";
        std::cout << "Objective value: " << response.objective_value() << std::endl;
    }
    else if (response.status() == CpSolverStatus::FEASIBLE) {
        std::cout << "A feasible solution found! ";
        std::cout << "Objective value: " << response.objective_value() << std::endl;
    }
    else {
        std::cout << "No optimal solution found!" << std::endl;
    }
}

void print_response_status(const CpSolverResponse& response)
{
    // std::cout << "Status: " << response.status() << std::endl;
    switch (response.status()) {
    case CpSolverStatus::OPTIMAL: std::cout << "Status: OPTIMAL" << std::endl; break;
    case CpSolverStatus::FEASIBLE: std::cout << "Status: FEASIBLE" << std::endl; break;
    case CpSolverStatus::INFEASIBLE: std::cout << "Status: INFEASIBLE" << std::endl; break;
    case CpSolverStatus::MODEL_INVALID: std::cout << "Status: MODEL_INVALID" << std::endl; break;
    case CpSolverStatus::UNKNOWN: std::cout << "Status: UNKNOWN" << std::endl; break;
    default: std::cout << "Status: UNDEFINED" << std::endl; break;
    }
}

void print_response_statistics(const CpSolverResponse& response)
{
    std::cout << "Statistics:" << std::endl;
    std::cout << CpSolverResponseStats(response) << std::endl;
}

void print_solution(const CpSolverResponse& response, const TaskVars& task_vars,
                    const InstData& inst_data)

{
    // write to the sol.csv file under data folder


    std::ofstream sol_file;
    sol_file.open("data/sol.csv");

    // write the header
    sol_file << "Job,Machine,Reticle,Transfer,Setup,Start,Processing,End,"
                "Position,Reticle_usage\n";

    std::cout << "print solutions ... \n";

    for (const auto& [task_id, task_presence_var] : task_vars.task_presence_vars) {
        auto [job_id, machine_id] = task_id;
        if (SolutionBooleanValue(response, task_presence_var)) {
            auto reticle_id = inst_data.job_reticle_pairs.at(job_id);
            auto transfer_time =
                SolutionIntegerValue(response, task_vars.task_transfer_vars.at(task_id));
            auto setup_time = SolutionIntegerValue(response, task_vars.task_setup_vars.at(task_id));
            auto start_time = SolutionIntegerValue(response, task_vars.task_start_vars.at(task_id));
            auto end_time   = SolutionIntegerValue(response, task_vars.task_end_vars.at(task_id));
            auto processing_time = inst_data.processing_times.at(task_id);

            if (end_time - start_time != processing_time) {
                std::cout << "Error: processing time is not equal to the duration\n";
                std::cout << "processing time: " << processing_time << std::endl;
                std::cout << "start time: " << start_time << std::endl;
                std::cout << "end time: " << end_time << std::endl;
            }

            auto task_position =
                SolutionIntegerValue(response, task_vars.task_position_vars.at(task_id));
            auto reticle_sharing =
                SolutionIntegerValue(response, task_vars.reticle_sharing_vars.at(task_id));

            std::cout << "Job " << job_id << " is processed on machine " << machine_id;
            std::cout << " with reticle " << reticle_id;
            std::cout << " with transfer time: " << transfer_time;
            std::cout << ", setup time: " << setup_time;
            std::cout << ", start time: " << start_time;
            std::cout << ", duration: " << processing_time;
            std::cout << ", end time: " << end_time;
            std::cout << ", task position on machine: " << task_position;
            std::cout << ", reticle usage: " << reticle_sharing << std::endl;

            sol_file << job_id << "," << machine_id << "," << reticle_id << "," << transfer_time
                     << "," << setup_time << "," << start_time << "," << processing_time << ","
                     << end_time << "," << task_position << "," << reticle_sharing << std::endl;
        }
    }

    // close the file
    sol_file.close();
}

}   // namespace sat
}   // namespace operations_research
