
import random
import numpy as np
import pandas as pd

target_dir = "data"


def gen_transfer_matrix(m:int):
    # Generate transfer matrix with random values [1, 10)
    T = np.random.randint(2, 5, size=(m, m))
    np.fill_diagonal(T, 0)
    
    # Write T to a csv file in data folder
    df = pd.DataFrame(T)
    
    # remove the header and index and write the to the file in the target folder
    df.to_csv(target_dir + "/transfer_time.csv", header=False, index=False)
    
def gen_setup_time_matrix(m:int, r:int):
    # for each machine, and each pair of reticle type, generate a setup time
    # Generate setup time matrix with random values [1, 10), store them in a dict
    setup_time = {}
    for i in range(m):
        for j in range(r):
            for k in range(r):
                if j == k:
                    setup_time[(i, j, k)] = 0
                else:
                    setup_time[(i, j, k)] = np.random.randint(1, 5)

    # Write setup_time to a csv file in data folder
    # df = pd.DataFrame(setup_time)
    df = pd.Series(setup_time).to_frame()
        
    # remove the header and index
    df.to_csv(target_dir + "/setup_time.csv", header=False, index=True)

def gen_job_release_time(j:int):
    # Generate job release time with random values [1, 10)
    lb_release = 0
    ub_release = j * 2
    job_release_time = np.random.randint(lb_release,ub_release, size=(j, 1))
    
    # Write job_release_time to a csv file in data folder
    df = pd.DataFrame(job_release_time)
    
    # remove the header and index
    df.to_csv(target_dir + "/job_release_time.csv", header=False, index=True)
    
def gen_job_due_time(j:int):
    # Generate job due time with random values [30, 50)
    lb_due = j * 2
    ub_due = j * 3
    job_due_time = np.random.randint(lb_due,ub_due, size=(j, 1))
    
    # Write job_due_time to a csv file in data folder
    df = pd.DataFrame(job_due_time)
    
    # remove the header and index
    df.to_csv(target_dir + "/job_due_time.csv", header=False, index=True)
    
def gen_dedicated_machines(j:int, m:int):
    dedicate_machine = {}
    for j in range(j):
        if random.random() < 0.1:
            dedicate_machine[j] = random.choice(range(m))
            
    # Write dedicate_machine to a csv file in data folder
    df = pd.Series(dedicate_machine).to_frame()
    
    # remove the header
    df.to_csv(target_dir + "/dedicated_machines.csv", header=False)

    return df

def gen_processing_time(j:int, m:int, ded_machines:pd.DataFrame = None):
    # for each job, random select m/2 machines to process the job,if the job has no dedicated machine
    # if the job has a dedicated machine, then the job will be processed only on the dedicated machine

    processing_time = {}
    for i in range(j):
        assigned_machines = []
        for k in range(m):
            if random.random() < 0.5:
                processing_time[(i,k)] = random.randint(2, 5)
                assigned_machines.append(k)

        # Ensure at least one machine is assigned to the job
        if not assigned_machines:
            k = random.choice(range(m))
            processing_time[(i,k)] = random.randint(2, 5)

    # Write processing_time to a csv file in data folder
    df = pd.Series(processing_time).to_frame()

    # remove the header and index
    df.to_csv(target_dir + "/job_processing_time.csv", header=False)



def gen_maximum_reticle_sharing(r:int):
    # Generate maximum reticle sharing with random values [1,3] for each reticle type
    max_reticle_sharing = np.random.randint(r/2, r, size=(r, 1))
    
    # Write max_reticle_sharing to a csv file in data folder
    df = pd.DataFrame(max_reticle_sharing)
    
    # remove the header and index
    df.to_csv(target_dir + "/reticle_sharing.csv", header=False, index=True)

    return df 

def gen_reticle_init_positions(r:int, m:int):
    # Generate reticle initial positions at random machines
    reticle_init_positions = np.random.randint(0, m, size=(r, 1))

    # Write reticle_init_positions to a csv file in data folder
    df = pd.DataFrame(reticle_init_positions)

    # remove the header and index
    df.to_csv(target_dir + "/reticle_init_positions.csv", header=False, index=True)

def gen_reticle_init_usage(r:int ):
    usage_df = gen_maximum_reticle_sharing(r)

    # Generate reticle initial usage at random machines
    reticle_init_usage = np.random.randint(0, usage_df.values, size=(r, 1))

    # Write reticle_init_positions to a csv file in data folder
    df = pd.DataFrame(reticle_init_usage)

    # remove the header and index
    df.to_csv(target_dir + "/reticle_init_usage.csv", header=False, index=True)

def gen_job_reticle_pairs(j:int, r:int):
    # Generate job reticle pairs
    job_reticle_pairs = {}
    for i in range(j):
        job_reticle_pairs[i] = random.choice(range(r))

    # Write job_reticle_pairs to a csv file in data folder
    df = pd.Series(job_reticle_pairs).to_frame()

    # remove the header
    df.to_csv(target_dir + "/job_reticle_pairs.csv", header=False)

if __name__ == "__main__":
    j: int = 50
    m: int = 5
    r: int = 10
    
    # 1. Generate transfer matrix: transfer_matrix[m,m] = transfer_time
    gen_transfer_matrix(m)
    
    # 2. Generate setup time matrix: setup_time[m,r,r] = setup_time
    gen_setup_time_matrix(m, r)
    
    # 3. Generate job release time: job_release_time[j] = release_time
    gen_job_release_time(j)

    # 4. Generate job due time : job_due_time[j] = due_time
    gen_job_due_time(j)
    
    # 5. Generate machine dedicated data: machine_dedicated_data[j] = dedi_machine 
    ded_machines = gen_dedicated_machines(j, m)
    
    # 6. Generate machine processing time matrix: processing_time[j, m] > 0, then the job can be processing on the machine 
    gen_processing_time(j, m, ded_machines)
    
    # 7. Generate maximum reticle sharing {reticle: max_sharing}
    gen_maximum_reticle_sharing(r)

    # 8. Generate reticle initial positions {reticle: init_position}
    gen_reticle_init_positions(r, m)

    # 9. Generate reticle initial usage {reticle: init_usage}
    gen_reticle_init_usage(r)

    # 10. Generate job reticle pairs {job: reticle}
    gen_job_reticle_pairs(j, r)


