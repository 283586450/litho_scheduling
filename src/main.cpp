#include <map>
#include <vector>

#include "ortools/sat/cp_model.h"
#include "ortools/sat/model.h"
#include "ortools/sat/sat_parameters.pb.h"

#include "build_model.hpp"
#include "read_data.hpp"
#include "solve_model.hpp"
#include "types.hpp"

int main()
{
    // operations_research::sat::MinimalJobshopSat();
    // Read Data *******************************************************************************
    operations_research::sat::InstData inst_data;
    inst_data.job_ded_machines  = operations_research::sat::read_dedicated_machine_data();
    inst_data.job_release_times = operations_research::sat::read_job_release_time_data();
    inst_data.job_due_times     = operations_research::sat::read_job_due_time_data();
    inst_data.job_reticle_pairs = operations_research::sat::read_job_reticle_pair_data();
    auto all_task_ptime_map     = operations_research::sat::read_job_processing_time_data();
    operations_research::sat::filter_tasks(all_task_ptime_map, inst_data);

    inst_data.setup_times            = operations_research::sat::read_setup_time_data();
    inst_data.transfer_times         = operations_research::sat::read_transfer_time_data();
    inst_data.reticle_sharing_limits = operations_research::sat::read_reticle_sharing_data();
    inst_data.reticle_init_positions = operations_research::sat::read_reticle_init_positions_data();
    inst_data.reticle_init_usage     = operations_research::sat::read_reticle_init_usage();

    // Prepare Data *****************************************************************************

    auto max_horizon = operations_research::sat::find_max_horizon(inst_data);
    auto max_transfer_time_map =
        operations_research::sat::find_machine_max_transfer_time(inst_data);
    auto max_setup_time_map = operations_research::sat::find_task_max_setup_time(inst_data);

    // Build Model ******************************************************************************
    operations_research::sat::CpModelBuilder cp_model;
    operations_research::sat::TaskVars       task_vars;

    operations_research::sat::add_task_transfer_vars(cp_model, task_vars, inst_data);
    operations_research::sat::add_task_setup_vars(cp_model, task_vars, inst_data);
    operations_research::sat::add_task_start_vars(cp_model, task_vars, inst_data, max_horizon);
    operations_research::sat::add_task_end_vars(cp_model, task_vars, inst_data, max_horizon);
    operations_research::sat::add_task_presence_vars(cp_model, task_vars, inst_data);
    operations_research::sat::add_task_optional_interval_vars(cp_model, task_vars, inst_data);
    operations_research::sat::add_reticle_sharing_vars(cp_model, task_vars, inst_data);

    // not used now
    operations_research::sat::add_task_position_vars(cp_model, task_vars, inst_data);

    // constraints ******************************************************************************
    operations_research::sat::add_task_precense_constraints(cp_model, task_vars);
    operations_research::sat::add_job_release_time_constraints(cp_model, task_vars, inst_data);
    operations_research::sat::add_reticle_max_sharing_constraints(cp_model, task_vars, inst_data);
    operations_research::sat::add_machine_no_overlap_constraints(cp_model, task_vars);
    operations_research::sat::add_reticle_no_overlap_constraints(cp_model, task_vars, inst_data);
    operations_research::sat::add_setup_constraints(cp_model, task_vars, inst_data);
    operations_research::sat::add_transfer_constraints(cp_model, task_vars, inst_data);

    // obj **************************************************************************************
    std::vector<operations_research::sat::IntVar> obj_exprs;
    operations_research::sat::add_obj_minimize_makespan(
        cp_model, task_vars, obj_exprs, max_horizon);
    // operations_research::sat::add_obj_minimize_transfer_time(
    //     cp_model, task_vars, obj_exprs, max_horizon);
    // operations_research::sat::add_obj_minimize_setup_time(
    //     cp_model, task_vars, obj_exprs, max_horizon);
    operations_research::sat::add_obj_minimize_tardiness(cp_model, task_vars, obj_exprs, inst_data);

    cp_model.Minimize(operations_research::sat::LinearExpr::Sum(obj_exprs));

    // solve ***********************************************************************************
    operations_research::sat::Model         model;
    operations_research::sat::SatParameters parameters;

    operations_research::sat::set_time_limit(parameters, 60);
    operations_research::sat::set_num_search_workers(parameters, 16);
    operations_research::sat::enable_log_search_progress(parameters);
    operations_research::sat::add_parameters_to_model(model, parameters);

    auto response = operations_research::sat::solve_model(model, cp_model);

    operations_research::sat::print_obj_val(response);
    operations_research::sat::print_response_status(response);
    operations_research::sat::print_response_statistics(response);
    operations_research::sat::print_solution(response, task_vars, inst_data);

    return 0;
}

