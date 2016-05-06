#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""

Management Template Test Script

"""
import templates.charm.Parcel as pt
import templates.charm.Diversion as dt
import templates.charm.Reservoir as rt
import templates.charm.Region as rg
import templates.charm.Factory as factory
import templates.common.Utilities as ut
import pandas as pd
div_dir = "../generated_code/charm_diversions"
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

try:
    makedirs(div_dir)
except OSError as e:
    if not path.isdir(div_dir):  
        raise

#input_dir = '../database/reservoir_monthly_parameters/div6'
input_dir = '../database/workshop/'
res_file = path.join(input_dir, 'res_data.csv')
#res_file = path.join(input_dir, 'test_5_large_res_data.csv')
reg_file = path.join(input_dir, 'region_data.csv')
div_file = path.join('../database/irrigation/', 'diversion_data.csv')
parcel_file = path.join('../database/irrigation/', 'parcel_data.csv')

def makeParcelClassFiles(s):
    #names Name, ElementID, DecreedAmount, RequestedAmount, ApproriationDate, ParcelIds
    s = s.replace(pd.np.nan, -1)
    p = pt.Parcel()
    p.name(s.Name)
    p.parcelID(str(s.Name)[3:])
    p.area(s.Area)
    decrees = [d.strip() for d in s.Decrees.split(';')]
    decreeMap = {}
    for decree in decrees:
        key, value = decree.split('=')
        decreeMap[key] = value
    p.decreedAmount(decreeMap)
    #TODO/FIXME For now, setting requests to be decrees
    p.requestAmount(decreeMap)
    
    elements = [e.strip() for e in s.Elements.split(';')]
    p.elementIds(elements)

    with open(path.join(div_dir, str(s.Name)+".cpp"), 'w') as output:
        output.write(str(p))

    with open(path.join(div_dir, "parcels.ci"), 'a') as output:
        output.write(str(pt.ParcelCI(s.Name)))

def makeDivClassFiles(s):
    #names Name, ElementID, DecreedAmount, RequestedAmount, ApproriationDate, ParcelIds
    s = s.replace(pd.np.nan, -1)
    div = dt.Diversion()
    div.name(s.Name)
    div.elementID(s.ElementID)
    div.decreedAmount(s.DecreedAmount)
    div.requestedAmount(s.RequestedAmount)
    d = str(s.ApproriationDate)
    #Break up the date yyyymmdd into its components
    div.appropriationDate(d[0:4], d[4:6], d[6:8])
    p = [i.strip() for i in s.ParcelIds.split(';')]
    div.parcelIds(p)

    with open(path.join(div_dir, str(s.Name)+".cpp"), 'w') as output:
        output.write(str(div))

    with open(path.join(div_dir, "diversions.ci"), 'a') as output:
        output.write(str(dt.DiversionCI(s.Name)))

def makeResClassFiles(s):
    #names Name,Reach,Region,SubRegion,UseRegion,MinRelease,MaxRelease,MinVolume,MaxVolume,BaseVolume,Jan,Feb,Mar,Apr,May,Jun,Jul,Aug,Sep,Oct,Nov,Dec
    s = s.replace(pd.np.nan, -1)
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
    s = s.replace(pd.np.nan, -1)
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

def generateReservoirs():
    #
    #Reservoirs
    #
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
    f = factory.ResFactory()
    #Create the abstract classes for a ReservoirFactory
    for k,v in f.abstract().iteritems():
        with open(path.join(fact_dir, k), 'w') as output:
            output.write(v)
    #Apply the creators for each object we are going to create
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

def generateCommon():
    #Generate the common utilities, simply put them in the res_dir
    for k,v in ut.classes().iteritems():
        with open(path.join(res_dir, k), 'w') as output:
            output.write(v)

def generateDiversions():
    #
    # Diversions
    #
    #Create the abstract classes for Diversions
    for k,v in dt.abstract().iteritems():
        with open(path.join(div_dir, k), 'w') as output:
            output.write(v)
    
    #Cheap hack for clearing the diversions.ci file if it exists
    with open(path.join(div_dir, "diversions.ci"), 'w') as output:
        pass
    
    df = pd.read_csv(div_file, sep=',\\s*', engine='python')
    #treat each row as an entry of data
    #apply the makeClassFiles function to each
    df.apply(makeDivClassFiles, axis=1)
    
    #Create the DiversionFactory
    f = factory.DiversionFactory()
    #Create the abstract classes for a DiversionFactory
    for k,v in f.abstract().iteritems():
        with open(path.join(fact_dir, k), 'w') as output:
            output.write(v)
    #Apply the creators for each object we are going to create
    f.creators(df['Name'])

    with open(path.join(fact_dir,'DiversionFactory.cpp'), 'w') as output:
        output.write(str(f))

def generateParcels():
    #
    # Parcels
    #

    #Create the abstract classes for Parcels
    for k,v in pt.abstract().iteritems():
        with open(path.join(div_dir, k), 'w') as output:
            output.write(v)
    
    #Cheap hack for clearing the parcels.ci file if it exists
    with open(path.join(div_dir, "parcels.ci"), 'w') as output:
        pass

    df = pd.read_csv(parcel_file, sep=',\\s*', engine='python')
    #treat each row as an entry of data
    #apply the makeClassFiles function to each
    df.apply(makeParcelClassFiles, axis=1)

    #Create the ParcelFactory
    f = factory.ParcelFactory()
    #Create the abstract classes for a ParcelFactory
    for k,v in f.abstract().iteritems():
        with open(path.join(fact_dir, k), 'w') as output:
            output.write(v)
    #Apply the creators for each object we are going to create
    f.creators(df['Name'])

    with open(path.join(fact_dir,'ParcelFactory.cpp'), 'w') as output:
        output.write(str(f))

def main():
    
    generateReservoirs()
    generateCommon()    
    #generateDiversions()
    #generateParcels()
  
if __name__ == "__main__":
    main()
