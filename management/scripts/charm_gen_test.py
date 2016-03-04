#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""

Management Template Test Script

"""
import templates.charm.Reservoir as rt
import templates.charm.Region as rg
import templates.charm.Factory as rf
import templates.common.Utilities as ut
import pandas as pd
res_dir = "../generated_code/charm_reservoirs"
fact_dir = "../generated_code/charm_factory"
from os import path, makedirs

#Create the dirs if not existent
try:
    makedirs(res_dir)
except OSError as e:
    if not path.isdir(res_dir):  
        raise
try:
    makedirs(fact_dir)
except OSError as e:
    if not path.isdir(fact_dir):  
        raise

input_dir = '../database/reservoir_monthly_parameters/div6'

res_file = path.join(input_dir, 'test_5_large_res_data.csv')
reg_file = path.join(input_dir, 'region_data.csv')


def makeResClassFiles(s):
    #names Name,Reach,Region,SubRegion,UseRegion,MinRelease,MaxRelease,MinVolume,MaxVolume,BaseVolume,Jan,Feb,Mar,Apr,May,Jun,Jul,Aug,Sep,Oct,Nov,Dec
    res = rt.Reservoir(s.UseRegion)
    res.name(s.Name)
    res.reachCode(s.Reach)
    res.region(s.Region)
    res.subRegion(s.SubRegion)
    res.release(s.MinRelease, s.MaxRelease)
    res.volume(s.MinVolume, s.MaxVolume, s.BaseVolume)
    rates = [s.Jan, s.Feb, s.Mar, s.Apr, s.May, s.Jun, s.Jul, s.Aug, s.Sep, s.Oct, s.Nov, s.Dec]
    res.targetRates(rates)

    #print res
    with open(path.join(res_dir, str(s.Name)+".cpp"), 'w') as output:
        output.write(str(res))

    with open(path.join(res_dir, "reservoirs.ci"), 'a') as output:
        output.write(str(rt.ReservoirCI(s.Name)))

def makeRegionClassFiles(s):
    #names Name,Region,SubRegion,UseRegion,MinRelease,MaxRelease,MinVolume,MaxVolume,BaseVolume,Jan,Feb,Mar,Apr,May,Jun,Jul,Aug,Sep,Oct,Nov,Dec    
    reg = rg.Region(s.SubRegion, s.UseRegion)
    reg.name(s.Name)
    reg.region(s.Region)
    reg.release(s.MinRelease, s.MaxRelease)
    reg.volume(s.MinVolume, s.MaxVolume, s.BaseVolume)
    rates = [s.Jan, s.Feb, s.Mar, s.Apr, s.May, s.Jun, s.Jul, s.Aug, s.Sep, s.Oct, s.Nov, s.Dec]
    reg.targetRates(rates)
    region_dir = path.join(res_dir, str(s.Region))
    with open(path.join(region_dir, str(s.Name)+".h"), 'w') as output:
        output.write(str(reg))
 
def main():

    #Create the abstract classes for reservoirs
    for k,v in rt.abstract().iteritems():
        with open(path.join(res_dir, k), 'w') as output:
            output.write(v)
    
    #Cheap hack for clearing the reservoirs.ci file if it exists
    with open(path.join(res_dir, "reservoirs.ci"), 'w') as output:
        pass

    df = pd.read_csv(res_file)
    #treat each row as an entry of data
    #apply the makeClassFiles function to each
    df.apply(makeResClassFiles, axis=1)

    #print df
    #Create the abstract classes for Factory
    for k,v in rf.abstract().iteritems():
        with open(path.join(fact_dir, k), 'w') as output:
            output.write(v)
    f = rf.Factory()
    f.creators(df['Name'])

    with open(path.join(fact_dir,'ReservoirFactory.cpp'), 'w') as output:
        output.write(str(f))
    
    #Put the regional abstract class in the reservoir dir
    for k,v in rg.abstract().iteritems():
        with open(path.join(res_dir, k), 'w') as output:
            output.write(v)

    regions = pd.read_csv(reg_file)
    for region in regions['Region'].drop_duplicates():
        reg_path = path.join(res_dir, region)
        if not path.exists(reg_path):
            makedirs(path.join(res_dir,region))
    regions.apply(makeRegionClassFiles, axis=1)

    #Generate the common utilities, simply put them in the res_dir
    for k,v in ut.classes().iteritems():
        with open(path.join(res_dir, k), 'w') as output:
            output.write(v)

if __name__ == "__main__":
    main()
