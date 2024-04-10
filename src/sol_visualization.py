
#%%
import random
import numpy as np
import pandas as pd
import plotly.express as px
import plotly.figure_factory as ff

def read_data(filename = '../data/sol.csv'):
    df = pd.read_csv(filename)

    result = pd.DataFrame(columns=['Job', 'Machine','Reticle', 'Start','During', 'Finish', 'Type', 'Reticle_usage', 'Position'])

    init_time = pd.to_datetime('today').replace(hour=8, minute=0, second=0)
    # iterrow, convert the data into each task in result dataframe
    for index, row in df.iterrows():
        job = row['Job']
        machine = row['Machine']
        reticle = row['Reticle']

        start = row['Start']
        start_time_stamp = init_time + pd.Timedelta(minutes=start)
        finish = row['End']
        finish_time_stamp = init_time + pd.Timedelta(minutes=finish)

        setup = row['Setup'] 
        setup_time_delta = pd.Timedelta(minutes=setup)
        
        transfer = row['Transfer']
        transfer_time_delta = pd.Timedelta(minutes=transfer)

        processing = row['Processing']
        position = row['Position']
        reticle_usage = row['Reticle_usage']

        # Task 
        task_df = pd.DataFrame({
            'Job': [job], 
            'Machine': [machine], 
            'Reticle': [reticle], 
            'Start': [start_time_stamp ],
            'During': [processing], 
            'Finish': [finish_time_stamp],
            'Type': ['Job'], 
            'Reticle_usage': [reticle_usage], 
            # 'Position': [position]
        })
        result = pd.concat([result, task_df], ignore_index=True)

        # Setup 
        if setup != 0:
            setup_df = pd.DataFrame({
                'Job': [job], 
                'Machine': [machine], 
                'Reticle': [reticle], 
                'Start': [start_time_stamp - setup_time_delta],
                'During': [setup],
                'Finish': [start_time_stamp ],
                'Type': ['Setup'], 
                'Reticle_usage': [reticle_usage], 
                # 'Position': [position]
            })
            result = pd.concat([result, setup_df], ignore_index=True)

        # Transfer
        if transfer != 0:
            transfer_df = pd.DataFrame({
                'Job': [job], 
                'Machine': [machine], 
                'Reticle': [reticle], 
                'Start': [start_time_stamp - setup_time_delta - transfer_time_delta],
                'During': [transfer],
                'Finish': [start_time_stamp - setup_time_delta],
                'Type': ['Transfer'], 
                'Reticle_usage': [reticle_usage], 
                # 'Position': [position]
            })
            result = pd.concat([result, transfer_df], ignore_index=True)

    # print(result)
    return result

def plot_gantt_chart(df):
    # create gantt chart with x-axis as time, y-axis as machine, color as Type
    fig = px.timeline(df, x_start='Start', x_end='Finish', y='Machine', color='Type',\
                      hover_data=['Job', 'Reticle', 'During', 'Reticle_usage', 'Type'],
                        title='Litho Scheduling')
    fig.update_yaxes(autorange="reversed")
    fig.show()

if __name__ == '__main__':
    df = read_data()
    plot_gantt_chart(df)


# %%
