
#include "ortools/sat/cp_model.h"

#include "build_model.hpp"
#include "types.hpp"
#include <iostream>
#include <vector>

namespace operations_research {
namespace sat {
constexpr bool DEBUG = true;

CpModelBuilder build_model()
{
    CpModelBuilder cp_model;

    return cp_model;
}

void filter_tasks(const std::map<TaskID, TimeStamp>& all_task_ptime_map, InstData& inst_data)
{
    // if a job is assigned to a dedicated machine,
    // then remove the other task for the same job from the task_ptime_map
    std::map<TaskID, TimeStamp> new_task_ptime_map;

    for (const auto& [task_id, duration] : all_task_ptime_map) {
        const auto [job_id, machine_id] = task_id;
        // 2. if the job is not in the job_ded_machines_map, then keep the task
        if (inst_data.job_ded_machines.find(job_id) == inst_data.job_ded_machines.end()) {
            new_task_ptime_map[task_id] = duration;
            continue;
        }
        // 3. else if the job is in the job_ded_machines_map, then only keep the
        // task for the dedicated machine
        if (inst_data.job_ded_machines.at(job_id) == machine_id and
            new_task_ptime_map.find(task_id) == new_task_ptime_map.end()) {
            new_task_ptime_map[task_id] = duration;
        }
    }

    // print the number of filtered tasks
    if (DEBUG) {
        std::cout << "Original task size: " << all_task_ptime_map.size() << std::endl;
        std::cout << "Filtered task size: " << new_task_ptime_map.size() << std::endl;
    }

    // swap the new_task_ptime_map with the inst_data.processing_times
    inst_data.processing_times.swap(new_task_ptime_map);
}

TimeStamp find_max_horizon(const InstData& inst_data)
{
    // TODO: maybe need refine the max horizon calculation
    TimeStamp max_horizon = 0;
    for (const auto& [_, duration] : inst_data.processing_times) {
        max_horizon += duration;
    }

    // print the max horizon
    if (DEBUG) {
        std::cout << "Max Horizon: " << max_horizon << std::endl;
    }

    return max_horizon;
}

std::map<MachineID, TimeStamp> find_machine_max_transfer_time(const InstData& inst_data)
{

    std::map<MachineID, TimeStamp> max_transfer_times_map;   // max transfer time for each machine
    // min transfer time = 0 (no need transfer)

    for (const auto& [machine_pair, transfer_time] : inst_data.transfer_times) {
        const auto [from_machine, to_machine] = machine_pair;
        if (max_transfer_times_map.find(to_machine) == max_transfer_times_map.end()) {
            max_transfer_times_map[to_machine] = transfer_time;
        }
        else {
            max_transfer_times_map[to_machine] =
                std::max(max_transfer_times_map[to_machine], transfer_time);
        }
    }

    // print the content of max_machine_ttime_map with lambda function
    if (DEBUG) {
        std::for_each(
            max_transfer_times_map.begin(), max_transfer_times_map.end(), [](const auto& pair) {
                const auto [machine, transfer_time] = pair;
                std::cout << "Machine: " << machine << ", Max Transfer Time: " << transfer_time
                          << std::endl;
            });
    }

    return max_transfer_times_map;
}

std::map<TaskID, TimeStamp> find_task_max_setup_time(const InstData& inst_data)
{
    std::map<TaskID, TimeStamp> max_setup_time_map;   // max setup time for each task
    // min setup time = 0 (no need setup)

    // find the jobs and reticles that can be processed on the same machine from
    // task_ptime_map
    std::map<MachineID, std::set<ReticleID>> machine_reticles_map;
    for (const auto& [task_id, _] : inst_data.processing_times) {
        const auto [job_id, machine_id] = task_id;
        const auto reticle_id           = inst_data.job_reticle_pairs.at(job_id);
        if (machine_reticles_map.find(machine_id) == machine_reticles_map.end()) {
            machine_reticles_map[machine_id] = std::set<ReticleID>{reticle_id};
        }
        else {
            machine_reticles_map[machine_id].insert(reticle_id);
        }
    }

    // from the machine_reticles_map and task_stime_map , find the max setup
    // time for each task
    for (const auto& [task_id, _] : inst_data.processing_times) {
        const auto [job_id, machine_id] = task_id;
        const auto reticle_id           = inst_data.job_reticle_pairs.at(job_id);
        for (const auto& [setup_pair, setup_time] : inst_data.setup_times) {
            const auto [machine_id, reticle_id_1, reticle_id_2] = setup_pair;
            // only consider the setup time to the same reticle on same machine
            if (reticle_id_2 != reticle_id) {
                continue;
            }

            // only consider the setup time to the reticle that can be processed
            // on the same machine
            if (machine_reticles_map[machine_id].find(reticle_id_1) ==
                machine_reticles_map[machine_id].end()) {
                continue;
            }

            if (max_setup_time_map.find(task_id) == max_setup_time_map.end()) {
                max_setup_time_map[task_id] = setup_time;
            }
            else {
                max_setup_time_map[task_id] = std::max(max_setup_time_map[task_id], setup_time);
            }
        }
    }

    // print the content of max_setup_time_map with lambda function
    if (DEBUG) {
        std::for_each(max_setup_time_map.begin(), max_setup_time_map.end(), [](const auto& pair) {
            const auto [task, setup_time] = pair;
            const auto [job, machine]     = task;
            std::cout << "Job: " << job << ", Machine: " << machine
                      << ", Max Setup Time: " << setup_time << std::endl;
        });
    }

    return max_setup_time_map;
}

void add_task_transfer_vars(CpModelBuilder& cp_model, TaskVars& task_vars,
                            const InstData& inst_data)
{
    task_vars.task_transfer_vars.clear();

    std::map<MachineID, TimeStamp> machine_max_transfer_times =
        find_machine_max_transfer_time(inst_data);

    for (const auto& [task_id, duration] : inst_data.processing_times) {
        const auto [job_id, machine_id] = task_id;
        const auto ub_transfer_time     = machine_max_transfer_times[machine_id];

        Domain      domain = {0, ub_transfer_time};
        std::string suffix = std::format("_{}_{}", job_id, machine_id);
        task_vars.task_transfer_vars[task_id] =
            cp_model.NewIntVar(domain).WithName(std::string("transfer") + suffix);

        if (DEBUG) {
            std::cout << "Job: " << job_id << ", Machine: " << machine_id
                      << ", Transfer Time Var: " << task_vars.task_transfer_vars[task_id]
                      << std::endl;
        }
    }
}


void add_task_setup_vars(CpModelBuilder& cp_model, TaskVars& task_vars, const InstData& inst_data)
{
    // std::map<TaskID, IntVar> task_setup_vars;
    task_vars.task_setup_vars.clear();

    std::map<TaskID, TimeStamp> max_setup_time_map = find_task_max_setup_time(inst_data);

    for (const auto& [task_id, duration] : inst_data.processing_times) {
        const auto [job_id, machine_id] = task_id;
        const auto ub_setup_time        = max_setup_time_map[task_id];

        Domain      domain = {0, ub_setup_time};
        std::string suffix = std::format("_{}_{}", job_id, machine_id);
        task_vars.task_setup_vars[task_id] =
            cp_model.NewIntVar(domain).WithName(std::string("setup") + suffix);

        if (DEBUG) {
            std::cout << "Job: " << job_id << ", Machine: " << machine_id
                      << ", Setup Time Var: " << task_vars.task_setup_vars[task_id] << std::endl;
        }
    }
}

void add_task_start_vars(CpModelBuilder& cp_model, TaskVars& task_vars, const InstData& inst_data,
                         TimeStamp horizon)
{
    task_vars.task_start_vars.clear();
    Domain domain = {0, horizon};

    for (const auto& [task_id, duration] : inst_data.processing_times) {
        const auto [job_id, machine_id] = task_id;

        std::string suffix = std::format("_{}_{}", job_id, machine_id);
        task_vars.task_start_vars[task_id] =
            cp_model.NewIntVar(domain).WithName(std::string("start") + suffix);

        if (DEBUG) {
            std::cout << "Job: " << job_id << ", Machine: " << machine_id
                      << ", Start Time Var: " << task_vars.task_start_vars[task_id] << std::endl;
        }
    }
}

void add_task_end_vars(CpModelBuilder& cp_model, TaskVars& task_vars, const InstData& inst_data,
                       TimeStamp horizon)
{
    task_vars.task_end_vars.clear();
    Domain domain = {0, horizon};

    for (const auto& [task_id, duration] : inst_data.processing_times) {
        const auto [job_id, machine_id] = task_id;

        std::string suffix = std::format("_{}_{}", job_id, machine_id);
        task_vars.task_end_vars[task_id] =
            cp_model.NewIntVar(domain).WithName(std::string("end") + suffix);

        if (DEBUG) {
            std::cout << "Job: " << job_id << ", Machine: " << machine_id
                      << ", End Time Var: " << task_vars.task_end_vars[task_id] << std::endl;
        }
    }
}

void add_task_presence_vars(CpModelBuilder& cp_model, TaskVars& task_vars,
                            const InstData& inst_data)
{
    task_vars.task_presence_vars.clear();

    for (const auto& [task_id, duration] : inst_data.processing_times) {
        const auto [job_id, machine_id] = task_id;

        std::string suffix = std::format("_{}_{}", job_id, machine_id);
        task_vars.task_presence_vars[task_id] =
            cp_model.NewBoolVar().WithName(std::string("presence") + suffix);

        if (DEBUG) {
            std::cout << "Job: " << job_id << ", Machine: " << machine_id
                      << ", Presence Var: " << task_vars.task_presence_vars[task_id] << std::endl;
        }
    }
}

void add_task_optional_interval_vars(CpModelBuilder& cp_model, TaskVars& task_vars,
                                     const InstData& inst_data)
{
    task_vars.task_optional_interval_vars.clear();

    for (const auto& [task_id, duration] : inst_data.processing_times) {
        const auto [job_id, machine_id] = task_id;

        std::string suffix = std::format("_{}_{}", job_id, machine_id);
        task_vars.task_optional_interval_vars[task_id] =
            cp_model
                .NewOptionalIntervalVar(task_vars.task_start_vars.at(task_id),
                                        duration,
                                        task_vars.task_end_vars.at(task_id),
                                        task_vars.task_presence_vars.at(task_id))
                .WithName(std::string("interval") + suffix);

        if (DEBUG) {
            std::cout << "Job: " << job_id << ", Machine: " << machine_id
                      << ", Interval Var: " << task_vars.task_optional_interval_vars[task_id]
                      << std::endl;
        }
    }
}

void add_reticle_sharing_vars(CpModelBuilder& cp_model, TaskVars& task_vars,
                              const InstData& inst_data)
{
    task_vars.reticle_sharing_vars.clear();

    for (const auto& [task_id, duration] : inst_data.processing_times) {
        const auto& [job_id, machine_id] = task_id;
        const auto reticle_id            = inst_data.job_reticle_pairs.at(job_id);
        const auto ub_reticle_sharing    = inst_data.reticle_sharing_limits.at(reticle_id);
        Domain     domain                = {1, ub_reticle_sharing};

        std::string suffix = std::format("_{}_{}", job_id, machine_id);
        task_vars.reticle_sharing_vars[task_id] =
            cp_model.NewIntVar(domain).WithName(std::string("sharing") + suffix);

        if (DEBUG) {
            std::cout << "Job: " << job_id << ", Machine: " << machine_id
                      << ", Sharing Var: " << task_vars.reticle_sharing_vars[task_id] << std::endl;
        }
    }
}

void add_task_position_vars(CpModelBuilder& cp_model, TaskVars& task_vars,
                            const InstData& inst_data)
{
    // count the number of candidate tasks for each machine
    std::map<MachineID, int> machine_task_count;
    for (const auto& [task_id, duration] : inst_data.processing_times) {
        const auto [job_id, machine_id] = task_id;
        if (machine_task_count.find(machine_id) == machine_task_count.end()) {
            machine_task_count[machine_id] = 1;
        }
        else {
            machine_task_count[machine_id]++;
        }
    }

    task_vars.task_position_vars.clear();

    for (const auto& [task_id, duration] : inst_data.processing_times) {
        const auto [job_id, machine_id] = task_id;
        const auto ub_position          = machine_task_count[machine_id];
        Domain     domain               = {0, ub_position};


        std::string suffix = std::format("_{}_{}", job_id, machine_id);
        task_vars.task_position_vars[task_id] =
            cp_model.NewIntVar(domain).WithName(std::string("position") + suffix);

        std::cout << "Job: " << job_id << ", Machine: " << machine_id
                  << ", Position Var: " << task_vars.task_position_vars[task_id] << std::endl;
    }
}

void add_task_precense_constraints(CpModelBuilder& cp_model, const TaskVars& task_vars)
{
    // for each job, at exactly one task is presence
    std::map<JobID, std::vector<BoolVar>> job_presence_vars_map;

    for (const auto& [task_id, presence_var] : task_vars.task_presence_vars) {
        const auto [job_id, machine_id] = task_id;
        if (job_presence_vars_map.find(job_id) == job_presence_vars_map.end()) {
            job_presence_vars_map[job_id] = std::vector<BoolVar>{presence_var};
        }
        else {
            job_presence_vars_map[job_id].push_back(presence_var);
        }
    }

    // use bool or constraint to ensure that at exactly one task is presence
    for (const auto& [job_id, presence_vars] : job_presence_vars_map) {
        //  cp_model.AddBoolOr(presence_vars);
        cp_model.AddExactlyOne(presence_vars);
        // cp_model.AddEquality(LinearExpr::Sum(presence_vars), 1);
    }
}

void add_job_release_time_constraints(CpModelBuilder& cp_model, const TaskVars& task_vars,
                                      const InstData& inst_data)
{
    // add release time constraints for each job
    for (const auto& [task_id, start_var] : task_vars.task_start_vars) {
        const auto [job_id, machine_id] = task_id;
        auto release_time               = inst_data.job_release_times.at(job_id);

        if (release_time == 0) {
            continue;
        }

        cp_model.AddGreaterOrEqual(start_var, release_time);

        if (DEBUG) {
            std::cout << "Job: " << job_id << ", Machine: " << machine_id
                      << ", Release Time Constraint: " << start_var << " >= " << release_time
                      << std::endl;
        }
    }
}

void add_reticle_max_sharing_constraints(CpModelBuilder& cp_model, const TaskVars& task_vars,
                                         const InstData& inst_data)
{
    // add max reticle sharing constraints for each reticle
    for (const auto& [task_id, sharing_var] : task_vars.reticle_sharing_vars) {
        const auto [job_id, machine_id] = task_id;
        const auto reticle_id           = inst_data.job_reticle_pairs.at(job_id);
        auto       max_sharing          = inst_data.reticle_sharing_limits.at(reticle_id);

        cp_model.AddLessOrEqual(sharing_var, max_sharing);

        if (DEBUG) {
            std::cout << "Job: " << job_id << ", Machine: " << machine_id
                      << ", Reticle: " << reticle_id << ", Max Sharing Constraint: " << sharing_var
                      << " <= " << max_sharing << std::endl;
        }
    }
}


void add_machine_no_overlap_constraints(CpModelBuilder& cp_model, const TaskVars& task_vars)
{
    // add no overlap constraints for the tasks on the same machine
    std::map<MachineID, std::vector<IntervalVar>> machine_intervals_map;
    for (const auto& [task_id, interval_var] : task_vars.task_optional_interval_vars) {
        const auto [job_id, machine_id] = task_id;
        if (machine_intervals_map.find(machine_id) == machine_intervals_map.end()) {
            machine_intervals_map[machine_id] = std::vector<IntervalVar>{interval_var};
        }
        else {
            machine_intervals_map[machine_id].push_back(interval_var);
        }
    }

    for (const auto& [machine_id, interval_vars] : machine_intervals_map) {
        auto name    = std::format("Machine_{}_no_overlap_constraint", machine_id);
        auto overlap = cp_model.AddNoOverlap(interval_vars).WithName(name);

        if (DEBUG) {
            std::cout << "Machine: " << machine_id << ", No Overlap Constraint: " << overlap.Name()
                      << std::endl;
        }
    }
}

void add_reticle_no_overlap_constraints(CpModelBuilder& cp_model, const TaskVars& task_vars,
                                        const InstData& inst_data)
{
    // add no overlap constraints for the tasks on the same reticle
    std::map<ReticleID, std::vector<IntervalVar>> reticle_intervals_map;
    for (const auto& [task_id, interval_var] : task_vars.task_optional_interval_vars) {
        const auto [job_id, machine_id] = task_id;
        const auto reticle_id           = inst_data.job_reticle_pairs.at(job_id);
        if (reticle_intervals_map.find(reticle_id) == reticle_intervals_map.end()) {
            reticle_intervals_map[reticle_id] = std::vector<IntervalVar>{interval_var};
        }
        else {
            reticle_intervals_map[reticle_id].push_back(interval_var);
        }
    }

    for (const auto& [reticle_id, interval_vars] : reticle_intervals_map) {
        auto name    = std::format("Reticle_{}_no_overlap_constraint", reticle_id);
        auto overlap = cp_model.AddNoOverlap(interval_vars).WithName(name);

        if (DEBUG) {
            std::cout << "Reticle: " << reticle_id << ", No Overlap Constraint: " << overlap.Name()
                      << std::endl;
        }
    }
}


void add_setup_constraints(CpModelBuilder& cp_model, const TaskVars& task_vars,
                           const InstData& inst_data)
{
    // for all intervals, add the job to the vector of each machine
    std::map<MachineID, std::vector<JobID>> machine_local_jobs_map;
    for (const auto& [task_id, _] : task_vars.task_optional_interval_vars) {
        const auto [job_id, machine_id] = task_id;
        if (machine_local_jobs_map.find(machine_id) == machine_local_jobs_map.end()) {
            machine_local_jobs_map[machine_id] = std::vector<JobID>{job_id};
        }
        else {
            machine_local_jobs_map[machine_id].push_back(job_id);
        }
    }

    if (DEBUG) {
        for (const auto& [machine_id, job_ids] : machine_local_jobs_map) {
            std::cout << "Machine: " << machine_id << ", Job IDs: ";
            for (const auto& job_id : job_ids) {
                std::cout << job_id << " ";
            }
            std::cout << std::endl;
        }
    }


    // for each machine,
    for (const auto& [machine_id, job_ids] : machine_local_jobs_map) {
        CircuitConstraint circuit = cp_model.AddCircuitConstraint();

        for (auto id1 = 0; id1 < job_ids.size(); id1++) {
            JobID     job1     = job_ids[id1];
            TaskID    task1    = {job1, machine_id};
            ReticleID reticle1 = inst_data.job_reticle_pairs.at(job1);

            auto start_lit = cp_model.NewBoolVar().WithName(std::format("start_lit_{}", job1));
            auto last_lit  = cp_model.NewBoolVar().WithName(std::format("last_lit_{}", job1));

            circuit.AddArc(0, id1 + 1, start_lit);
            circuit.AddArc(id1 + 1, 0, last_lit);
            circuit.AddArc(id1 + 1, id1 + 1, ~task_vars.task_presence_vars.at(task1));
            cp_model.AddImplication(start_lit, task_vars.task_presence_vars.at(task1));
            cp_model.AddImplication(last_lit, task_vars.task_presence_vars.at(task1));
            // cp_model.AddEquality(task_vars.task_position_vars.at(task1), 0)
            //     .OnlyEnforceIf(start_lit);

            // start time of the task >= transfer time + setup time
            cp_model
                .AddLessOrEqual(
                    task_vars.task_transfer_vars.at(task1) + task_vars.task_setup_vars.at(task1),
                    task_vars.task_start_vars.at(task1))
                .OnlyEnforceIf(start_lit);

            if (inst_data.reticle_init_positions.at(reticle1) == machine_id) {
                // if the init position of reticle1 is current machine. and the task is the first
                // task then the reticle sharing count = initial reticle usage + 1, if start_lit is
                // true

                // should be set in transfer constraints instead of setup constraints

                // cp_model
                //     .AddEquality(task_vars.reticle_sharing_vars.at(task1),
                //                  inst_data.reticle_init_usage.at(reticle1) + 1)
                //     .OnlyEnforceIf(start_lit);
            }

            if (inst_data.reticle_init_positions.at(reticle1) != machine_id) {
                // else, setup time is needed
                // TimeDuration setup_time1 = 1;
                // cp_model.AddEquality(task_vars.task_setup_vars.at(task1), setup_time1)
                //     .OnlyEnforceIf(start_lit);
            }

            // for each pair of jobs
            for (auto id2 = 0; id2 < job_ids.size(); id2++) {
                if (id1 == id2) {
                    continue;
                }

                JobID     job2     = job_ids[id2];
                TaskID    task2    = {job2, machine_id};
                ReticleID reticle2 = inst_data.job_reticle_pairs.at(job2);

                auto adjacency =
                    cp_model.NewBoolVar().WithName(std::format("adjacency_{}_{}", job1, job2));
                circuit.AddArc(id1 + 1, id2 + 1, adjacency);

                // # precent constraints
                cp_model
                    .AddBoolAnd({task_vars.task_presence_vars.at({job_ids[id1], machine_id}),
                                 task_vars.task_presence_vars.at({job_ids[id2], machine_id})})
                    .OnlyEnforceIf(adjacency);

                // # adjacency constraints
                cp_model
                    .AddLessOrEqual(task_vars.task_end_vars.at(task1) +
                                        task_vars.task_setup_vars.at(task2) +
                                        task_vars.task_transfer_vars.at(task2),
                                    task_vars.task_start_vars.at(task2))
                    .OnlyEnforceIf(adjacency);

                // if they share the same reticle
                if (reticle1 == reticle2) {
                    // # reticle sharing constraints
                    cp_model
                        .AddGreaterOrEqual(task_vars.reticle_sharing_vars.at(task2),
                                           task_vars.reticle_sharing_vars.at(task1) + 1)
                        .OnlyEnforceIf(adjacency);
                }

                // if they do not share the same reticle
                if (reticle1 != reticle2) {
                    // # setup time constraints
                    TimeDuration setup_time2 =
                        inst_data.setup_times.at({machine_id, reticle1, reticle2});

                    cp_model.AddGreaterOrEqual(task_vars.task_setup_vars.at(task2), setup_time2)
                        .OnlyEnforceIf(adjacency);
                }
            }
        }
    }
}

void add_transfer_constraints(CpModelBuilder& cp_model, const TaskVars& task_vars,
                              const InstData& inst_data)
{
    // for all tasks, add the TaskID to the vector of each reticle
    std::map<ReticleID, std::vector<TaskID>> reticle_local_tasks_map;
    for (const auto& [task_id, _] : task_vars.task_optional_interval_vars) {
        const auto [job_id, machine_id] = task_id;
        const auto reticle_id           = inst_data.job_reticle_pairs.at(job_id);
        if (reticle_local_tasks_map.find(reticle_id) == reticle_local_tasks_map.end()) {
            reticle_local_tasks_map[reticle_id] = std::vector<TaskID>{task_id};
        }
        else {
            reticle_local_tasks_map[reticle_id].push_back(task_id);
        }
    }

    if (DEBUG) {
        for (const auto& [reticle_id, task_ids] : reticle_local_tasks_map) {
            std::cout << "Reticle: " << reticle_id << ", Task IDs: ";
            for (const auto& task_id : task_ids) {
                const auto [job_id, machine_id] = task_id;
                auto id                         = job_id * 1000 + machine_id + 1;
                std::cout << id << " ";
            }
            std::cout << std::endl;
        }
    }

    // for each reticle,
    for (const auto& [reticle_id, task_ids] : reticle_local_tasks_map) {
        CircuitConstraint circuit = cp_model.AddCircuitConstraint();

        for (auto id1 = 0; id1 < task_ids.size(); ++id1) {
            const auto task1            = task_ids[id1];
            const auto [job1, machine1] = task1;
            const auto init_position1   = inst_data.reticle_init_positions.at(reticle_id);
            const auto init_usage1      = inst_data.reticle_init_usage.at(reticle_id);

            auto start_lit =
                cp_model.NewBoolVar().WithName(std::format("start_lit_{}_{}", job1, machine1));
            auto last_lit =
                cp_model.NewBoolVar().WithName(std::format("last_lit_{}_{}", job1, machine1));

            circuit.AddArc(0, id1 + 1, start_lit);
            circuit.AddArc(id1 + 1, 0, last_lit);
            circuit.AddArc(id1 + 1, id1 + 1, ~task_vars.task_presence_vars.at(task1));
            cp_model.AddImplication(start_lit, task_vars.task_presence_vars.at(task1));
            cp_model.AddImplication(last_lit, task_vars.task_presence_vars.at(task1));

            // if the init position of reticle1 is current machine.
            if (init_position1 == machine1) {
                // then the reticle sharing count = initial reticle usage + 1, if start_lit is true
                cp_model
                    .AddGreaterOrEqual(task_vars.reticle_sharing_vars.at(task1), init_usage1 + 1)
                    .OnlyEnforceIf(start_lit);
            }

            // else: if the init position of reticle1 is not current machine.
            if (init_position1 != machine1) {
                // transfer time is needed
                const auto transfer_time1 = inst_data.transfer_times.at({init_position1, machine1});
                cp_model.AddGreaterOrEqual(task_vars.task_transfer_vars.at(task1), transfer_time1)
                    .OnlyEnforceIf(start_lit);

                // setup time is needed (assume the first setup time is 3)
                TimeDuration setup_time1 = 2;
                cp_model.AddGreaterOrEqual(task_vars.task_setup_vars.at(task1), setup_time1)
                    .OnlyEnforceIf(start_lit);

                // start time of the task >= transfer time + setup time
                cp_model
                    .AddGreaterOrEqual(task_vars.task_start_vars.at(task1),
                                       task_vars.task_transfer_vars.at(task1) +
                                           task_vars.task_setup_vars.at(task1))
                    .OnlyEnforceIf(start_lit);
            }


            for (auto id2 = 0; id2 < task_ids.size(); ++id2) {
                if (id1 == id2) {
                    continue;
                }

                const auto task2            = task_ids[id2];
                const auto [job2, machine2] = task2;

                auto adjacency = cp_model.NewBoolVar().WithName(
                    std::format("reticle_adjacency_{}_{}_{}_{}", job1, machine1, job2, machine2));
                circuit.AddArc(id1 + 1, id2 + 1, adjacency);

                // # precent constraints
                cp_model
                    .AddBoolAnd({task_vars.task_presence_vars.at(task1),
                                 task_vars.task_presence_vars.at(task2)})
                    .OnlyEnforceIf(adjacency);

                // # adjacency constraints
                cp_model
                    .AddLessOrEqual(task_vars.task_end_vars.at(task1) +
                                        task_vars.task_setup_vars.at(task2) +
                                        task_vars.task_transfer_vars.at(task2),
                                    task_vars.task_start_vars.at(task2))
                    .OnlyEnforceIf(adjacency);

                // if they are processed on the same machine [may not be needed, because we already
                // have the constraint on setup constraints]
                if (machine1 == machine2) {
                    // # reticle sharing constraints
                    // cp_model
                    //     .AddEquality(task_vars.reticle_sharing_vars.at(task1) + 1,
                    //                  task_vars.reticle_sharing_vars.at(task2))
                    //     .OnlyEnforceIf(adjacency);
                }

                // if they are processed on different machines
                if (machine1 != machine2) {
                    // # transfer time constraints
                    const auto transfer_time2 = inst_data.transfer_times.at({machine1, machine2});
                    cp_model
                        .AddGreaterOrEqual(task_vars.task_transfer_vars.at(task2), transfer_time2)
                        .OnlyEnforceIf(adjacency);

                    // # setup time constraints
                    const auto setup_time2 = 2;
                    cp_model.AddGreaterOrEqual(task_vars.task_setup_vars.at(task2), setup_time2)
                        .OnlyEnforceIf(adjacency);
                }
            }
        }
    }







    // // for each reticle,
    // for (const auto& [reticle_id, task_ids] : reticle_local_tasks_map) {
    //     CircuitConstraint circuit = cp_model.AddCircuitConstraint();
    //     // for each pair of tasks
    //     for (const auto& task1 : task_ids) {
    //         const auto [job1, machine1]      = task1;
    //         unsigned long long id1           = job1 * 1000 + machine1 + 1;
    //         auto               init_position = inst_data.reticle_init_positions.at(reticle_id);
    //         auto               initial_usage = inst_data.reticle_init_usage.at(reticle_id);

    //         auto start_lit_name =
    //             std::format("reticle_{}_start_at_{}_{}", reticle_id, job1, machine1);
    //         auto start_literals = cp_model.NewBoolVar().WithName(start_lit_name);
    //         circuit.AddArc(0, id1, start_literals);

    //         // if job1 is the first job on the reticle, then:
    //         // #0. the task1 is presence
    //         cp_model.AddEquality(task_vars.task_presence_vars.at(task1), 1)
    //             .OnlyEnforceIf(start_literals);

    //         // 1. if the reticle's init position is the same as the machine, and the job
    //         // which use the reticle, then the reticle sharing count >= init usage, no setup
    //         // time is needed, no transfer time is needed
    //         if (init_position == machine1) {
    //             cp_model
    //                 .AddGreaterOrEqual(task_vars.reticle_sharing_vars.at(task1), initial_usage +
    //                 1) .OnlyEnforceIf(start_literals);
    //         }
    //         // else, the retilce is at another machine, then the transfer time is needed
    //         // setup time is needed, reticle sharing count = 1
    //         if (init_position != machine1) {
    //             auto trans_time1 = inst_data.transfer_times.at({init_position, machine1});

    //             // precedence constraint: transfer time + setup time  <= task2
    //             cp_model.AddLessOrEqual(trans_time1 + 3, task_vars.task_start_vars.at(task1))
    //                 .OnlyEnforceIf(start_literals);

    //             // we assume the first setup time is 3.
    //             // In practice, the setup time should be depend on [m,r,r]
    //             cp_model.AddGreaterOrEqual(task_vars.task_setup_vars.at(task1), 3)
    //                 .OnlyEnforceIf(start_literals);

    //             cp_model.AddGreaterOrEqual(task_vars.task_transfer_vars.at(task1), trans_time1)
    //                 .OnlyEnforceIf(start_literals);

    //             // cp_model.AddEquality(task_vars.reticle_sharing_vars.at(task1), 1)
    //             //     .OnlyEnforceIf(start_literals);
    //         }


    //         // add the arc from the last job to 0
    //         auto last_lit_name =
    //             std::format("reticle_{}_last_at_{}_{}", reticle_id, job1, machine1);
    //         auto last_literals = cp_model.NewBoolVar().WithName(last_lit_name);
    //         circuit.AddArc(id1, 0, last_literals);

    //         for (const auto& task2 : task_ids) {
    //             const auto [job2, machine2] = task2;
    //             unsigned long long id2      = job2 * 1000 + machine2 + 1;

    //             if (job1 == job2 && machine1 == machine2) {
    //                 continue;
    //             }

    //             auto adjacency_name = std::format("reticle_{}_adjacency_{}_{}_to_{}——{}",
    //                                               reticle_id,
    //                                               job1,
    //                                               machine1,
    //                                               job2,
    //                                               machine2);
    //             auto adjacency      = cp_model.NewBoolVar().WithName(adjacency_name);
    //             circuit.AddArc(id1, id2, adjacency);


    //             auto trans_time2 = inst_data.transfer_times.at({machine1, machine2});

    //             // if job1 is processed before job2 on the same reticle, then:
    //             // 0. the both task is presence
    //             cp_model
    //                 .AddBoolAnd({task_vars.task_presence_vars.at(task1),
    //                              task_vars.task_presence_vars.at(task2)})
    //                 .OnlyEnforceIf(adjacency);

    //             // 1. if they are processed on the same machine, then the transfer time = 0
    //             // setup time = 0, reticle sharing count += 1
    //             if (machine1 == machine2) {
    //                 // precedence constraint
    //                 // cp_model
    //                 //     .AddLessOrEqual(task_vars.task_end_vars.at(task1),
    //                 //                     task_vars.task_start_vars.at(task2))
    //                 //     .OnlyEnforceIf(adjacency);

    //                 // reticle sharing count += 1
    //                 cp_model
    //                     .AddGreaterOrEqual(task_vars.reticle_sharing_vars.at(task2),
    //                                        task_vars.reticle_sharing_vars.at(task1) + 1)
    //                     .OnlyEnforceIf(adjacency);
    //             }
    //             // 2. if they are processed on different machines, then the transfer time is
    //             // needed and setup time is needed, reticle sharing count = 1
    //             if (machine1 != machine2) {
    //                 // transfer time = transfer time from machine1 to machine2
    //                 cp_model.AddEquality(task_vars.task_transfer_vars.at(task2), trans_time2)
    //                     .OnlyEnforceIf(adjacency);

    //                 // precedence constraint: task1 + transfer time + setup time <= task2
    //                 // where the setup time is handled by the setup constraint
    //                 cp_model
    //                     .AddLessOrEqual(task_vars.task_end_vars.at(task1) +
    //                                         task_vars.task_transfer_vars.at(task2) +
    //                                         task_vars.task_setup_vars.at(task2),
    //                                     task_vars.task_start_vars.at(task2))
    //                     .OnlyEnforceIf(adjacency);
    //             }
    //         }
    //     }
    // }
}


void add_obj_minimize_makespan(CpModelBuilder& cp_model, const TaskVars& task_vars,
                               std::vector<IntVar>& obj_exprs, TimeStamp horizon)
{
    // add the objective makespan
    auto makespan = cp_model.NewIntVar({0, horizon}).WithName("makespan");

    std::vector<IntVar> end_vars;
    for (const auto& [task_id, end_var] : task_vars.task_end_vars) {
        end_vars.push_back(end_var);
    }

    cp_model.AddMaxEquality(makespan, end_vars);

    obj_exprs.push_back(makespan);

    if (DEBUG) {
        std::cout << "Objective Makespan: " << makespan << std::endl;
    }
}


void add_obj_minimize_transfer_time(CpModelBuilder& cp_model, const TaskVars& task_vars,
                                    std::vector<IntVar>& obj_exprs, TimeStamp horizon)
{
    // add the objective minimize transfer time
    IntVar total_transfer = cp_model.NewIntVar({0, horizon}).WithName("total_transfer");

    std::vector<IntVar> transfer_vars;
    for (const auto& [task_id, transfer_var] : task_vars.task_transfer_vars) {
        transfer_vars.push_back(transfer_var);
    }

    cp_model.AddEquality(total_transfer, LinearExpr::Sum(transfer_vars));


    obj_exprs.push_back(total_transfer);

    if (DEBUG) {
        std::cout << "Objective Total Transfer Time: " << total_transfer << std::endl;
    }
}

void add_obj_minimize_setup_time(CpModelBuilder& cp_model, const TaskVars& task_vars,
                                 std::vector<IntVar>& obj_exprs, TimeStamp horizon)
{
    // add the objective minimize setup time
    IntVar total_setup = cp_model.NewIntVar({0, horizon}).WithName("total_setup");

    std::vector<IntVar> setup_vars;
    for (const auto& [task_id, setup_var] : task_vars.task_setup_vars) {
        setup_vars.push_back(setup_var);
    }

    cp_model.AddEquality(total_setup, LinearExpr::Sum(setup_vars));

    obj_exprs.push_back(total_setup);

    if (DEBUG) {
        std::cout << "Objective Total Setup Time: " << total_setup << std::endl;
    }
}

void add_obj_minimize_tardiness(CpModelBuilder& cp_model, const TaskVars& task_vars,
                                std::vector<IntVar>& obj_exprs, const InstData& inst_data)
{
    // add the objective minimize tardiness
    IntVar total_tardiness = cp_model.NewIntVar({0, 100000}).WithName("total_tardiness");

    std::vector<IntVar> tardiness_vars;
    for (const auto& [task_id, end_var] : task_vars.task_end_vars) {
        const auto [job_id, machine_id] = task_id;
        auto due_time                   = inst_data.job_due_times.at(job_id);
        auto tardiness                  = cp_model.NewIntVar({0, 100000}).WithName("tardiness");
        cp_model.AddMaxEquality(tardiness, {0, end_var - due_time});
        tardiness_vars.push_back(tardiness);
    }

    cp_model.AddEquality(total_tardiness, LinearExpr::Sum(tardiness_vars));

    obj_exprs.push_back(total_tardiness);

    if (DEBUG) {
        std::cout << "Objective Total Tardiness: " << total_tardiness << std::endl;
    }
}


}   // namespace sat
}   // namespace operations_research