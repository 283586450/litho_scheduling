#include <fstream>

#include "read_data.hpp"
#include "types.hpp"

namespace operations_research {
namespace sat {

std::map<JobID, MachineID> read_dedicated_machine_data()
{
    // Read the dedicated machine data from dedicated_machine.csv file, and
    // store it in a map
    std::map<JobID, MachineID> dedicated_machine_data;
    std::ifstream              dedicated_machine_file;
    dedicated_machine_file.open("data/dedicated_machines.csv");
    if (!dedicated_machine_file.is_open()) {
        std::cerr << "Unable to open dedicated_machine.csv file" << std::endl;
        return dedicated_machine_data;
    }

    std::string line;
    while (std::getline(dedicated_machine_file, line)) {
        std::stringstream        line_stream(line);
        std::string              cell;
        std::vector<std::string> row;
        while (std::getline(line_stream, cell, ',')) {
            row.push_back(cell);
        }
        if (row.size() < 2) {
            std::cerr << "Invalid data format in dedicated_machine.csv file" << std::endl;
            continue;
        }
        try {
            JobID     job_id               = std::stoi(row[0]);
            MachineID machine_id           = std::stoi(row[1]);
            dedicated_machine_data[job_id] = machine_id;
            std::cout << "Job ID: " << job_id << " Dedicated Machine ID: " << machine_id
                      << std::endl;
        }
        catch (const std::invalid_argument& ia) {
            std::cerr << "Invalid data in dedicated_machine.csv file: " << ia.what() << std::endl;
        }
    }

    dedicated_machine_file.close();   // Close the file

    return dedicated_machine_data;
}

std::map<JobID, TimeStamp> read_job_release_time_data()
{
    std::map<JobID, TimeStamp> job_release_time_data;
    std::ifstream              job_release_time_file;
    job_release_time_file.open("data/job_release_time.csv");
    if (!job_release_time_file.is_open()) {
        std::cerr << "Unable to open job_release_times.csv file" << std::endl;
        return job_release_time_data;
    }

    std::string line;
    while (std::getline(job_release_time_file, line)) {
        std::stringstream        line_stream(line);
        std::string              cell;
        std::vector<std::string> row;
        while (std::getline(line_stream, cell, ',')) {
            row.push_back(cell);
        }
        if (row.size() < 2) {
            std::cerr << "Invalid data format in job_release_times.csv file" << std::endl;
            continue;
        }
        try {
            JobID     job_id              = std::stoi(row[0]);
            TimeStamp release_time        = std::stoi(row[1]);
            job_release_time_data[job_id] = release_time;
            std::cout << "Job ID: " << job_id << " Release Time: " << release_time << std::endl;
        }
        catch (const std::invalid_argument& ia) {
            std::cerr << "Invalid data in job_release_times.csv file: " << ia.what() << std::endl;
        }
    }

    job_release_time_file.close();   // Close the file

    return job_release_time_data;
}

std::map<JobID, TimeStamp> read_job_due_time_data()
{
    std::map<JobID, TimeStamp> job_due_time_data;   // key: job_id, value: due_time
    std::ifstream              job_due_time_file;
    job_due_time_file.open("data/job_due_time.csv");
    if (!job_due_time_file.is_open()) {
        std::cerr << "Unable to open job_due_times.csv file" << std::endl;
        return job_due_time_data;
    }

    std::string line;
    while (std::getline(job_due_time_file, line)) {
        std::stringstream        line_stream(line);
        std::string              cell;
        std::vector<std::string> row;
        while (std::getline(line_stream, cell, ',')) {
            row.push_back(cell);
        }
        if (row.size() < 2) {
            std::cerr << "Invalid data format in job_due_times.csv file" << std::endl;
            continue;
        }
        try {
            JobID     job_id          = std::stoi(row[0]);
            TimeStamp due_time        = std::stoi(row[1]);
            job_due_time_data[job_id] = due_time;
            std::cout << "Job ID: " << job_id << " Due Time: " << due_time << std::endl;
        }
        catch (const std::invalid_argument& ia) {
            std::cerr << "Invalid data in job_due_times.csv file: " << ia.what() << std::endl;
        }
    }

    job_due_time_file.close();   // Close the file

    return job_due_time_data;
}

std::map<TaskID, TimeStamp> read_job_processing_time_data()
{
    std::map<TaskID, TimeStamp> job_processing_time_data;   // key: (job_id, machine_id), value:
    std::ifstream               job_processing_time_file;
    job_processing_time_file.open("data/job_processing_time.csv");
    if (!job_processing_time_file.is_open()) {
        std::cerr << "Unable to open job_processing_time.csv file" << std::endl;
        return job_processing_time_data;
    }

    std::string line;
    while (std::getline(job_processing_time_file, line)) {
        std::stringstream line_stream(line);
        std::string       cell;
        std::vector<int>  row;
        std::getline(line_stream, cell, ',');
        int job_id = std::stoi(cell);
        std::getline(line_stream, cell, ',');
        int machine_id = std::stoi(cell);
        std::getline(line_stream, cell, ',');
        TimeStamp processing_time     = std::stoi(cell);
        auto      key                 = std::make_pair(job_id, machine_id);
        job_processing_time_data[key] = processing_time;
        std::cout << "Job ID: " << job_id << " Machine ID: " << machine_id
                  << " Processing Time: " << processing_time << std::endl;
    }

    job_processing_time_file.close();   // Close the file

    return job_processing_time_data;
}

std::map<ReticleID, int> read_reticle_sharing_data()
{
    std::map<ReticleID, int> reticle_sharing_data;   // key: reticle_id, value: reticle_sharing
    std::ifstream            reticle_sharing_file;
    reticle_sharing_file.open("data/reticle_sharing.csv");
    if (!reticle_sharing_file.is_open()) {
        std::cerr << "Unable to open reticle_sharing.csv file" << std::endl;
        return reticle_sharing_data;
    }

    std::string line;
    while (std::getline(reticle_sharing_file, line)) {
        std::stringstream        line_stream(line);
        std::string              cell;
        std::vector<std::string> row;
        while (std::getline(line_stream, cell, ',')) {
            row.push_back(cell);
        }
        if (row.size() < 2) {
            std::cerr << "Invalid data format in reticle_sharing.csv file" << std::endl;
            continue;
        }
        try {
            ReticleID reticle_id             = std::stoi(row[0]);
            int       max_sharing            = std::stoi(row[1]);
            reticle_sharing_data[reticle_id] = max_sharing;
            std::cout << "Reticle ID: " << reticle_id << " Max Sharing: " << max_sharing
                      << std::endl;
        }
        catch (const std::invalid_argument& ia) {
            std::cerr << "Invalid data in reticle_sharing.csv file: " << ia.what() << std::endl;
        }
    }

    reticle_sharing_file.close();   // Close the file

    return reticle_sharing_data;
}

std::map<SetupPair, TimeDuration> read_setup_time_data()
{
    std::map<SetupPair, TimeDuration> setup_time_data;   // key: (machine_id, reticle_id,
                                                         // reticle_id), value: setup_time
    std::ifstream setup_time_file;
    setup_time_file.open("data/setup_time.csv");
    if (!setup_time_file.is_open()) {
        std::cerr << "Unable to open setup_time.csv file" << std::endl;
        return setup_time_data;
    }

    std::string line;
    while (std::getline(setup_time_file, line)) {
        std::stringstream        line_stream(line);
        std::string              cell;
        std::vector<std::string> row;
        while (std::getline(line_stream, cell, ',')) {
            row.push_back(cell);
        }
        if (row.size() < 2) {
            std::cerr << "Invalid data format in dedicated_machine.csv file" << std::endl;
            continue;
        }
        try {
            MachineID    machine_id   = std::stoi(row[0]);
            ReticleID    reticle_id_1 = std::stoi(row[1]);
            ReticleID    reticle_id_2 = std::stoi(row[2]);
            TimeDuration setup_time   = std::stoi(row[3]);
            SetupPair    key          = std::make_tuple(machine_id, reticle_id_1, reticle_id_2);
            setup_time_data[key]      = setup_time;
            std::cout << "Machine ID: " << machine_id << " Reticle ID 1: " << reticle_id_1
                      << " Reticle ID 2: " << reticle_id_2 << " Setup Time: " << setup_time
                      << std::endl;
        }
        catch (const std::invalid_argument& ia) {
            std::cerr << "Invalid data in dedicated_machine.csv file: " << ia.what() << std::endl;
        }
    }

    setup_time_file.close();   // Close the file

    return setup_time_data;
}

std::map<MachinePair, TimeDuration> read_transfer_time_data()
{
    std::map<MachinePair, TimeDuration> transfer_time_data;   // key: (machine_id, machine_id),
                                                              // value: transfer_time
    std::ifstream transfer_time_file;
    transfer_time_file.open("data/transfer_time.csv");
    if (!transfer_time_file.is_open()) {
        std::cerr << "Unable to open transfer_time.csv file" << std::endl;
        return transfer_time_data;
    }

    std::string line;
    int         row_count = 0;
    while (std::getline(transfer_time_file, line)) {
        int               col_count = 0;
        std::stringstream line_stream(line);
        std::string       cell;
        while (std::getline(line_stream, cell, ',')) {
            try {
                auto         key           = std::make_pair(row_count, col_count);
                TimeDuration transfer_time = std::stoi(cell);
                transfer_time_data[key]    = transfer_time;
                std::cout << "From Machine ID: " << row_count << " To Machine ID: " << col_count
                          << " Transfer Time: " << transfer_time << std::endl;
            }
            catch (const std::invalid_argument& ia) {
                std::cerr << "Invalid data in transfer_time.csv file: " << ia.what() << std::endl;
            }
            col_count++;
        }
        row_count++;
    }

    transfer_time_file.close();   // Close the file

    return transfer_time_data;
}

std::map<ReticleID, MachineID> read_reticle_init_positions_data()
{
    std::map<ReticleID, MachineID>
                  reticle_init_positions_data;   // key: reticle_id, value: machine_id
    std::ifstream reticle_init_positions_file;
    reticle_init_positions_file.open("data/reticle_init_positions.csv");
    if (!reticle_init_positions_file.is_open()) {
        std::cerr << "Unable to open reticle_init_positions.csv file" << std::endl;
        return reticle_init_positions_data;
    }

    std::string line;
    while (std::getline(reticle_init_positions_file, line)) {
        std::stringstream        line_stream(line);
        std::string              cell;
        std::vector<std::string> row;
        while (std::getline(line_stream, cell, ',')) {
            row.push_back(cell);
        }
        if (row.size() < 2) {
            std::cerr << "Invalid data format in reticle_init_positions.csv file" << std::endl;
            continue;
        }
        try {
            ReticleID reticle_id                    = std::stoi(row[0]);
            MachineID machine_id                    = std::stoi(row[1]);
            reticle_init_positions_data[reticle_id] = machine_id;
            std::cout << "Reticle ID: " << reticle_id << " Init Position at Machine: " << machine_id
                      << std::endl;
        }
        catch (const std::invalid_argument& ia) {
            std::cerr << "Invalid data in reticle_init_positions.csv file: " << ia.what()
                      << std::endl;
        }
    }

    reticle_init_positions_file.close();   // Close the file

    return reticle_init_positions_data;
}

std::map<ReticleID, int> read_reticle_init_usage()
{
    std::map<ReticleID, int> reticle_init_usage_data;   // key: reticle_id, value: usage_count
    std::ifstream            reticle_init_usage_file;
    reticle_init_usage_file.open("data/reticle_init_usage.csv");
    if (!reticle_init_usage_file.is_open()) {
        std::cerr << "Unable to open reticle_init_usage.csv file" << std::endl;
        return reticle_init_usage_data;
    }

    std::string line;
    while (std::getline(reticle_init_usage_file, line)) {
        std::stringstream        line_stream(line);
        std::string              cell;
        std::vector<std::string> row;
        while (std::getline(line_stream, cell, ',')) {
            row.push_back(cell);
        }
        if (row.size() < 2) {
            std::cerr << "Invalid data format in reticle_init_usage.csv file" << std::endl;
            continue;
        }
        try {
            ReticleID reticle_id                = std::stoi(row[0]);
            int       usage_count               = std::stoi(row[1]);
            reticle_init_usage_data[reticle_id] = usage_count;
            std::cout << "Reticle ID: " << reticle_id << " Init Usage Count: " << usage_count
                      << std::endl;
        }
        catch (const std::invalid_argument& ia) {
            std::cerr << "Invalid data in reticle_init_usage.csv file: " << ia.what() << std::endl;
        }
    }

    reticle_init_usage_file.close();   // Close the file

    return reticle_init_usage_data;
}

std::map<JobID, ReticleID> read_job_reticle_pair_data()
{
    std::map<JobID, ReticleID> job_reticle_data;   // key: job_id, value: reticle_id
    std::ifstream              job_reticle_file;
    job_reticle_file.open("data/job_reticle_pairs.csv");
    if (!job_reticle_file.is_open()) {
        std::cerr << "Unable to open job_reticle.csv file" << std::endl;
        return job_reticle_data;
    }

    std::string line;
    while (std::getline(job_reticle_file, line)) {
        std::stringstream        line_stream(line);
        std::string              cell;
        std::vector<std::string> row;
        while (std::getline(line_stream, cell, ',')) {
            row.push_back(cell);
        }
        if (row.size() < 2) {
            std::cerr << "Invalid data format in job_reticle.csv file" << std::endl;
            continue;
        }
        try {
            JobID     job_id         = std::stoi(row[0]);
            ReticleID reticle_id     = std::stoi(row[1]);
            job_reticle_data[job_id] = reticle_id;
            std::cout << "Job ID: " << job_id << " Reticle ID: " << reticle_id << std::endl;
        }
        catch (const std::invalid_argument& ia) {
            std::cerr << "Invalid data in job_reticle.csv file: " << ia.what() << std::endl;
        }
    }

    job_reticle_file.close();   // Close the file

    return job_reticle_data;
}

}   // namespace sat
}   // namespace operations_research