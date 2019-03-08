#!/usr/bin/env python
from adhydro import ADHydroCalibration
from utils import Configuration
from utils.objectives import custom
import glob
import subprocess
#import configparser
#import argparse
#import StringIO
import shutil
import os
import pandas as pd
import geopandas as gpd
#import netCDF4 as nc
from math import log

def objective_func(simulated_hydrograph_file, observed_hydrograph, eval_range=None):
    simulated_hydrograph = pd.read_csv(simulated_hydrograph_file, header=None, usecols=[1,2], names=['time', 'flow'])
    simulated_hydrograph['time'] /= 86400 #partial day
    simulated_hydrograph['time'] += hydrograph_reference_date #Julian date from reference
    simulated_hydrograph['time'] = pd.to_datetime(simulated_hydrograph['time'], utc=True, unit='D', origin='julian').dt.round('1s')
    simulated_hydrograph.drop_duplicates('time', keep='last', inplace=True)
    simulated_hydrograph.set_index('time', inplace=True)
    #Join the data where the indicies overlap
    #print( simulated_hydrograph )
    #print( observed_hydrograph )
    df = pd.merge(simulated_hydrograph, observed_hydrograph, left_index=True, right_index=True)
    #print( df) 
    if eval_range:
        df = df.loc[eval_range[0]:eval_range[1]]
    #print( df )
    #Evaluate custom objective function providing simulated, observed series
    return custom(df['flow'], df['Discharge_cms'])

def write_objective_log_file(i, score):
    with open(os.path.join(config.workdir, 'objective.log'), 'a') as log_file:
        log_file.write('{}, '.format(i))
        log_file.write('{}\n'.format(score))

def write_param_log_file(i, best, score):
    with open(os.path.join(config.workdir, 'best_params.log'), 'w') as log_file:
        log_file.write('{}\n'.format(i))
        log_file.write('{}\n'.format(best))
        log_file.write('{}\n'.format(score))

def read_param_log_file():
    with open(os.path.join(config.workdir, 'best_params.log'), 'r') as log_file:
        iteration = int(log_file.readline())
        best_params = int(log_file.readline())
        best_score = float(log_file.readline())
    return iteration, best_params, best_score
config = Configuration()
#This is the range of the hydrograph dates to run the objective function over
evaluation_range = ('2011-08-05 12:00:00', '2011-08-08 00:00:00')
#FIXME this isn't always an accurate thing to look at...maybe recompute reference date from superfile???
#Hacked for 2011 right now...
hydrograph_reference_date = 2455772.5  #state_nc.variables['referenceDate'][0]
#print "REFERENCE DATE {}".format(hydrograph_reference_date)

#TODO make this an input...
outlet_element_id = 7510
#gage id correspoinding to outlet_element_id
usgs =  '02146300' #for 7462 -> '02146285'
hydrograph_output_file = os.path.join(config.workdir, 'state.nc.{}.txt'.format(outlet_element_id) )
#hydrograph_output_file = '/gscratch/rsteinke/runs/sugar_creek/2011-07-30-limited-regions/state.nc.7462.txt'
observed_file = os.path.join(config.workdir, 'usgs_{}_observed.csv'.format(usgs))
observed_df = pd.read_csv(observed_file, parse_dates=['Date_Time']).set_index('Date_Time')
observed_df = observed_df.tz_localize('UTC')
"""
best_score = objective_func(os.path.join(config.workdir, 'state.nc.{}.txt_0'.format(outlet_element_id)), observed_df, evaluation_range)
print(best_score)
os._exit(1)
"""
#FIXME see below
"""

In general, grouping is useful to get approriate indicies.  I think setting all DF's with elementID index will save
a lot of index tracking/referencing to elementID.  Try this out soon!

"""
"""
FIXME this section
"""
data_path = '/project/ci-water/data/'
"""
FIXME this section ^^^^^^^^
"""
#changes = {'simulationDuration':'86400'}
#config.edit_superfile(changes)
config.write_superfile()

print("Starting calib")
#List of ADHydro region IDs to include in calibration run
"""
calibration_regions = [57, 65, 78, 100, 139, 146, 241, 283, 320, 364, 424, 782, 930, 932, 935, 964, 999, 1000,
                       1004, 1007, 1011, 1015, 1016, 1026, 1030, 1440, 1445, 1454, 66, 96, 101, 141, 168, 242, 
                       284, 321, 367, 472, 928, 931, 933, 937, 997, 1001, 1005, 1008, 1013, 1018, 1028, 1031,
                       1441, 1447, 77, 99, 105, 142, 235, 244, 286, 411, 479, 929, 934, 938, 998, 1003, 1006, 
                       1009, 1014, 1019, 1029, 1033, 1443, 1453]
"""
calibration_regions  = [21, 32, 40, 42, 57, 65, 66, 68, 74, 77,
                        78, 84, 95, 96, 97, 99, 100, 101, 102, 103,
                        104, 105, 108, 137, 138, 139, 141, 142, 146, 168,
                        184, 196, 223, 234, 235, 241, 242, 244, 248, 276,
                        280, 282, 283, 284, 285, 296, 320, 321, 345, 364,
                        367, 372, 399, 403, 405, 411, 415, 417, 424, 448,
                        449, 466, 467, 468, 469, 471, 472, 479, 571, 582,
                        587, 590, 603, 610, 636, 719, 765, 769, 771, 772,
                        775, 781, 782, 900, 901, 905, 907, 908, 909, 910,
                        911, 912, 913, 914, 916, 921, 922, 927, 928, 929,
                        930, 931, 932, 933, 934, 935, 936, 937, 938, 956,
                        960, 961, 962, 963, 964, 996, 997, 998, 999, 1000,
                        1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009, 1011,
                        1013, 1014, 1015, 1016, 1018, 1019, 1020, 1021, 1022, 1023,
                        1024, 1025, 1026, 1027, 1028, 1029, 1030, 1031, 1033, 1040,
                        1100, 1109, 1117, 1319, 1320, 1321, 1322, 1325, 1326, 1327,
                        1328, 1329, 1330, 1331, 1332, 1333, 1334, 1335, 1336, 1337,
                        1338, 1339, 1351, 1352, 1353, 1355, 1356, 1357, 1359, 1440,
                        1441, 1444, 1445, 1448, 1449, 1467, 1474, 1847, 1848, 1862,
                        1866, 1904]
data = ADHydroCalibration(config, calibration_regions)

"""
At this point the ADHydroCalibration object contains the elements and all their properties that are going to be calibrated, grouped by the respective calibration parameters defined in adhydro.py

The next steps require the initial DDS vector, which is each parameter, and its initial value
Since we may not have to build out all parameters becuase some don't exist in our calibration region, build
this vector from the identified classes then read the min/max from the user provided tables for these classes, and set the initial value to be in the middle.

Also, before calibration begins, we need to take any fixed land cover classes and update parameter file to contain the provided
fixed values.
"""

#then we can start the DDS search from this initial vector
#Build the initial parameter space vector in the calibration matrix.

data.calibration_df.rename(columns={'init':0}, inplace=True)
variables = pd.Series(data.calibration_df.index.values)
#print calibration_df
#print variables
#Start defining the DDS search algorithm TODO move this to module
neighborhood_size = 0.2
iterations = 150
#restart_iteration = iterations+1
#print calibration_df
#precompute sigma for each variable based on neighborhood_size and bounds
data.calibration_df['sigma'] = 0.2*(data.calibration_df['upper'] - data.calibration_df['lower'])

#TODO move most of this to utils module
adhydro_bin = "/project/ci-water/tools/ADHydro/ADHydro/adhydro"
adhydro_extra_args = "{} +LBObjOnly +LBPeriod 10.0 +balancer GreedyCommLB +balancer RefineCommLB".format(config.calibration_superfile)
adhydro_cmd = "{} {}".format(adhydro_bin, adhydro_extra_args)
#slurm_run_command = "module restore adhydro && srun -A ci-water -N 4 --ntasks-per-node=16 -t 300"
slurm_run_command = "module restore adhydro && time srun -n 96 -A ci-water -N 6 --ntasks-per-node=16 -t 600"
print("Running initial ADHydro")
#sub_shell_cmd = slurm_run_command.split(' ')
#sub_shell_cmd.append(adhydro_bin)
#sub_shell_cmd.append(config.calibration_superfile)
#sub_shell_cmd.extend(adhydro_extra_args.split(' '))
#sub_shell_cmd.append(adhydro_cmd)
sub_shell_cmd = "{} {}".format(slurm_run_command, adhydro_cmd)
print(sub_shell_cmd)

#state_nc = nc.Dataset(state_nc_file)
start_iteration = 0
#run ADHydro here on initial parameters if needed
adhydro_log = open(os.path.join(config.workdir, 'adhydro_calibration_log'), 'a')
if config.args.restart:
    #Restarting means we don't have to do this...find a clean way to convey this as long as an output file
    #exists for a previous time.  This script renames state.nc.*.txt to state.nc.*.txt_<i>
    #where i is the iteration, so if an _i exists, and a state.nc.*.txt exists, this file is the i+1 state
    #and we can restart from there.
    states = glob.glob(os.path.join(config.workdir, 'state.nc.{}.txt_*'.format(outlet_element_id)))
    if states:
        #FIXME just read from log and maybe verify...
        start_iteration = sorted( [ int(name.split('_')[-1]) for name in states ] )[-1]
        #last_evaluated = os.path.join(config.workdir, 'state.nc.{}.txt_{}'.format(outlet_element_id, start_iteration))
        #I don't think we can use next_eval, becuase we can't gaurantee the run is complete...will have to restart it to be ensure
        #next_eval = os.path.join(config.workdir, 'state.nc.{}.txt'.format(outlet_element_id))
        #FIXME file should always exist in this case, but what happens if it doesnt???
        last_iteration, best_params, best_score = read_param_log_file()
        """
        with open(os.path.join(config.workdir, 'best_params.log'), 'r') as param_log:
            best_params = int(param_log.readline())
            best_score = float(param_log.readline())
        """
        if last_iteration != start_iteration:
            print("ERROR: best_params.log iteration doesn't match output file iteration number for restart")
            os._exit(1)
        #We will start at the next iteration that hasn't been completed
        start_iteration += 1
        #It is possible another run has finished but hasn't been evaluated.
        #If the generated state file exists, and was created/modified after the last evaluated,
        #then its safe to assume  we can start from there
        #TODO read superfile, calculate how many entrys should be in state and verify that run completed.
        """
        next_eval = os.path.join(config.workdir, 'state.nc.{}.txt'.format(outlet_element_id))
        if os.path.isfile( next_eval ):
            if os.path.getmtime(next_eval) > os.path.getmtime(last_evaluated):
                start_iteration += 1 #TODO this tells us that we have a new output file, is it guaranteed complete?
            else:
                #Just continue with the next iteration of the DDS search...
                #need to compute next_eval
                #subprocess.check_call(sub_shell_cmd, stdout=adhydr_log, shell=True) #FIXME clean up this logic
                #best_score = objective_func(hydrograph_output_file, observed_df, evaluation_range)
                #best_params = 0
                #write_param_log_file(best_params, best_score)
                pass
        else:
            #We have a prior state, but not the next one.  Recompute the score/evalute from this last one and continue
            #Evaluate initial score....FIXME Reading from the log should be enough...just need to verify log iteration is same as start_iteration
            #best_score = objective_func(last_evaluated, observed_df, evaluation_range)
            #best_params = start_iteration - 1 
            #FIXME file should always exist in this case, but what happens if it doesnt???
            with open(os.path.join(config.workdir, 'best_params.log'), 'r') as param_log:
                best_params = int(param_log.readline())
                best_score = float(param_log.readline())
        """ 
        """ FIXME can use this in an emergency if you know that the last run completed successfully
        next_eval = os.path.join(config.workdir, 'state.nc.{}.txt'.format(outlet_element_id))
        if os.path.isfile( next_eval ):
            #Evaluate score, run finished but wasn't evaluated
            score =  objective_func(hydrograph_output_file, observed_df, evaluation_range)
            shutil.move(hydrograph_output_file, '{}_{}'.format(hydrograph_output_file, start_iteration))
            if score <= best_score:
                best_params = start_iteration 
                best_score = score
                #Score has improved, run next simulation with
            print("Current score {}\nBest score {}".format(score, best_score))
            write_param_log_file(start_iteration, best_params, best_score)
            write_objective_log_file(start_iteration, score)
            start_iteration += 1
        """
    elif os.path.isfile( hydrograph_output_file ): #Iteration 0 complete
        #Evaluate initial score
        best_score = objective_func(hydrograph_output_file, observed_df, evaluation_range)
        best_params = 0
        write_param_log_file(0, best_params, best_score)
        write_objective_log_file(0, best_score)
        shutil.move(hydrograph_output_file, '{}_{}'.format(hydrograph_output_file, start_iteration))
        start_iteration = 1
    else:
        #No prior states written, start from the beginning
        subprocess.check_call(sub_shell_cmd, stdout=adhydro_log, shell=True)
        #Evaluate initial score
        best_score = objective_func(hydrograph_output_file, observed_df, evaluation_range)
        best_params = 0
        write_param_log_file(0, best_params, best_score)
        write_objective_log_file(0, best_score)
        shutil.move(hydrograph_output_file, '{}_{}'.format(hydrograph_output_file, start_iteration))
        start_iteration = 1

    if os.path.isfile( os.path.join(config.workdir, 'calibration_df_state.msg') ):
        data.calibration_df = pd.read_msgpack( os.path.join(config.workdir, 'calibration_df_state.msg') ) 
else:
    subprocess.check_call(sub_shell_cmd, stdout=adhydro_log, shell=True)
    #Evaluate initial score
    best_score = objective_func(hydrograph_output_file, observed_df, evaluation_range)
    best_params = 0
    write_param_log_file(0, best_params, best_score)
    write_objective_log_file(0, best_score)
    shutil.move(hydrograph_output_file, '{}_{}'.format(hydrograph_output_file, start_iteration))
    start_iteration = 1

#best_params = start_iteration #TODO this isn't really needed since best is always copied to next 

print("Starting Iteration: {}".format(start_iteration))
print("Starting Best param: {}".format(best_params))
print("Starting Best score: {}".format(best_score))
print("Starting DDS loop")
for i in range(start_iteration, iterations+1):
    #Calculate probability of inclusion
    inclusion_probability = 1 - log(i)/log(iterations)
    print( "inclusion probability: {}".format(inclusion_probability) )
    #select a random subset of variables to modify
    #TODO convince myself that grabbing a random selction of P fraction of items
    #is the same as selecting item with probability P
    neighborhood = variables.sample(frac=inclusion_probability)
    if neighborhood.empty:
       neighborhood = variables.sample(n=1)
    print( "neighborhood: {}".format(neighborhood) )
    #Copy the best parameter values so far into the next iterations parameter list
    data.calibration_df[i] = data.calibration_df[best_params]
    #print( data.calibration_df )
    for n in neighborhood:
        #permute the variables in neighborhood
        #using a random normal sample * sigma, sigma = 0.2*(max-min)
        #print(n, best_params)
        new = data.calibration_df.loc[n, best_params] + data.calibration_df.loc[n, 'sigma']*pd.np.random.normal(0,1)
        lower =  data.calibration_df.loc[n, 'lower']
        upper = data.calibration_df.loc[n, 'upper']
        #print( new )
        #print( lower )
        #print( upper )
        if new < lower:
            new = lower + (lower - new)
            if new > upper:
                new = lower
        elif new > upper:
            new = upper - (new - upper)
            if new < lower:
                new = upper
        data.calibration_df.loc[n, i] = new
    """
        At this point, we need to re-run ADHydro with the new parameters assigned correctly and evaluate the objective function
    """
    data.update_state(i)
    #Run ADHydro Again...
    print("Running ADHydro for iteration {}".format(i))
    subprocess.check_call(sub_shell_cmd, stdout=adhydro_log, shell=True)
    #read output and calculate objective_func
    score =  objective_func(hydrograph_output_file, observed_df, evaluation_range)
    shutil.move(hydrograph_output_file, '{}_{}'.format(hydrograph_output_file, i))
    if score <= best_score:
        best_params = i
        best_score = score
        #Score has improved, run next simulation with
    print("Current score {}\nBest score {}".format(score, best_score))
    data.calibration_df.to_msgpack(os.path.join(config.workdir, 'calibration_df_state.msg') )
    write_param_log_file(i, best_params, best_score)
    write_objective_log_file(i, score)
    #with open(os.path.join(config.workdir, 'best_params.log'), 'w') as log_file:
    #    log_file.write('{}\n'.format(i))
    #    log_file.write('{}\n'.format(best_params))
    #    log_file.write('{}\n'.format(best_score))
    #if i == restart_iteration:
    #    break
#print calibration_df
print("Best parameters at column {} in calibration_df_state.msg".format(best_params))

