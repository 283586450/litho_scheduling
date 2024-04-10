#pragma once

#include "ortools/sat/cp_model.h"

#include "types.hpp"

namespace operations_research {
namespace sat {

void set_time_limit(SatParameters& parameters, int time_limit);
void set_num_search_workers(SatParameters& parameters, int num_search_workers);
void enable_log_search_progress(SatParameters& parameters);
void disable_log_search_progress(SatParameters& parameters);
void add_parameters_to_model(Model& model, SatParameters& parameters);

CpSolverResponse solve_model(Model& model, CpModelBuilder& cp_model);

void print_obj_val(const CpSolverResponse& response);
void print_response_status(const CpSolverResponse& response);
void print_response_statistics(const CpSolverResponse& response);
void print_solution(const CpSolverResponse& response, const TaskVars& task_vars,
                    const InstData& inst_data);

}   // namespace sat
}   // namespace operations_research
