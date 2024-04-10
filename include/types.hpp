#pragma once

#include "ortools/sat/cp_model.h"

namespace operations_research {
namespace sat {

using JobID        = unsigned int;
using MachineID    = unsigned int;
using ReticleID    = unsigned int;
using TaskID       = std::pair<JobID, MachineID>;   // (job_id, machine_id)
using TimeStamp    = unsigned int;
using TimeDuration = unsigned int;

using MachinePair = std::pair<int, int>;   // (from_machine_id, to_machine_id)
using SetupPair   = std::tuple<MachineID, ReticleID,
                             ReticleID>;   // (machine_id, reticle_id_1, reticle_id_2)

struct TaskType
{   // job_id, machine_id, reticle_id
    IntVar       transfer;
    IntVar       setup;
    TimeDuration duration;

    IntVar  end;        // end time of the task, machine and reticle shared the end time
    BoolVar presence;   // job is presence on the machine or not, machine and
                        // reticle shared the presence
    IntVar sharing;     // reticle sharing count.
    // start processing time = end - duration
    IntVar task_position;   // job position in the job sequence

    IntVar      machine_start;
    IntVar      machine_duration;   // duration on the machine: setup + duration
    IntervalVar machine_interval;

    IntVar reticle_start;
    IntVar reticle_duration;   // duration on the reticle: transfer + setup +
                               // duration
    IntervalVar reticle_interval;
};

struct InstData
{
    std::map<JobID, MachineID>          job_ded_machines;
    std::map<JobID, TimeStamp>          job_release_times;
    std::map<JobID, TimeStamp>          job_due_times;
    std::map<JobID, ReticleID>          job_reticle_pairs;
    std::map<TaskID, TimeStamp>         processing_times;
    std::map<SetupPair, TimeDuration>   setup_times;
    std::map<MachinePair, TimeDuration> transfer_times;
    std::map<ReticleID, int>            reticle_sharing_limits;
    std::map<ReticleID, MachineID>      reticle_init_positions;
    std::map<ReticleID, int>            reticle_init_usage;
};

struct TaskVars
{
    std::map<TaskID, IntVar>      task_transfer_vars;
    std::map<TaskID, IntVar>      task_setup_vars;
    std::map<TaskID, IntVar>      task_start_vars;
    std::map<TaskID, IntVar>      task_end_vars;
    std::map<TaskID, BoolVar>     task_presence_vars;
    std::map<TaskID, IntervalVar> task_optional_interval_vars;
    std::map<TaskID, IntVar>      reticle_sharing_vars;
    std::map<TaskID, IntVar>      task_position_vars;
};

}   // namespace sat
}   // namespace operations_research