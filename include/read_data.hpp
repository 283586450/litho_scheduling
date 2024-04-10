#pragma once

#include "types.hpp"

namespace operations_research {
namespace sat {

std::map<JobID, MachineID>          read_dedicated_machine_data();
std::map<JobID, TimeStamp>          read_job_release_time_data();
std::map<JobID, TimeStamp>          read_job_due_time_data();
std::map<TaskID, TimeStamp>         read_job_processing_time_data();
std::map<ReticleID, int>            read_reticle_sharing_data();
std::map<SetupPair, TimeDuration>   read_setup_time_data();
std::map<MachinePair, TimeDuration> read_transfer_time_data();
std::map<ReticleID, MachineID>      read_reticle_init_positions_data();
std::map<ReticleID, int>            read_reticle_init_usage();
std::map<JobID, ReticleID>          read_job_reticle_pair_data();

}   // namespace sat
}   // namespace operations_research