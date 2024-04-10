#pragma once

#include "ortools/sat/cp_model.h"
#include "types.hpp"
#include <vector>

namespace operations_research {
namespace sat {

CpModelBuilder build_model();

void filter_tasks(const std::map<TaskID, TimeStamp>& all_task_ptime_map, InstData& inst_data);

TimeStamp find_max_horizon(const InstData& inst_data);

std::map<MachineID, TimeStamp> find_machine_max_transfer_time(const InstData& inst_data);

std::map<TaskID, TimeStamp> find_task_max_setup_time(const InstData& inst_data);

void add_task_transfer_vars(CpModelBuilder& cp_model, TaskVars& task_vars,
                            const InstData& inst_data);

void add_task_setup_vars(CpModelBuilder& cp_model, TaskVars& task_vars, const InstData& inst_data);

void add_task_start_vars(CpModelBuilder& cp_model, TaskVars& task_vars, const InstData& inst_data,
                         TimeStamp horizon);

void add_task_end_vars(CpModelBuilder& cp_model, TaskVars& task_vars, const InstData& inst_data,
                       TimeStamp horizon);

void add_task_presence_vars(CpModelBuilder& cp_model, TaskVars& task_vars,
                            const InstData& inst_data);

void add_task_optional_interval_vars(CpModelBuilder& cp_model, TaskVars& task_vars,
                                     const InstData& inst_data);

void add_reticle_sharing_vars(CpModelBuilder& cp_model, TaskVars& task_vars,
                              const InstData& inst_data);

void add_task_position_vars(CpModelBuilder& cp_model, TaskVars& task_vars,
                            const InstData& inst_data);

// **************************************************************************
void add_task_precense_constraints(CpModelBuilder& cp_model, const TaskVars& task_vars);

void add_job_release_time_constraints(CpModelBuilder& cp_model, const TaskVars& task_vars,
                                      const InstData& inst_data);

void add_reticle_max_sharing_constraints(CpModelBuilder& cp_model, const TaskVars& task_vars,
                                         const InstData& inst_data);

void add_machine_no_overlap_constraints(CpModelBuilder& cp_model, const TaskVars& task_vars);

void add_reticle_no_overlap_constraints(CpModelBuilder& cp_model, const TaskVars& task_vars,
                                        const InstData& inst_data);

void add_setup_constraints(CpModelBuilder& cp_model, const TaskVars& task_vars,
                           const InstData& inst_data);

void add_transfer_constraints(CpModelBuilder& cp_model, const TaskVars& task_vars,
                              const InstData& inst_data);

// **************************************************************************
void add_obj_minimize_makespan(CpModelBuilder& cp_model, const TaskVars& task_vars,
                               std::vector<IntVar>& obj_exprs, TimeStamp horizon);

void add_obj_minimize_transfer_time(CpModelBuilder& cp_model, const TaskVars& task_vars,
                                    std::vector<IntVar>& obj_exprs, TimeStamp horizon);

void add_obj_minimize_setup_time(CpModelBuilder& cp_model, const TaskVars& task_vars,
                                 std::vector<IntVar>& obj_exprs, TimeStamp horizon);

void add_obj_minimize_tardiness(CpModelBuilder& cp_model, const TaskVars& task_vars,
                                std::vector<IntVar>& obj_exprs, const InstData& inst_data);

}   // namespace sat
}   // namespace operations_research