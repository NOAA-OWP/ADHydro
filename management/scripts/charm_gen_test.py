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
import argparse
from os import path, makedirs, remove
from glob import glob

parser = argparse.ArgumentParser(description="Generate Charm++ code for management components.")
parser.add_argument('inputDirectory', help='Directory containing management component data for generation.  Should contain reservoirs, diversions, and parcels sub directories, each of these containing their respective .csv data.')
parser.add_argument('-o', '--outputDirectory', help='Optional output directory to place generated code into.  Defaults to ../generated_code .')
group = parser.add_argument_group('Components', 'Which management components should code be generated for.')
group.add_argument('-r', '--reservoirs', help='Flag for generating reservoir object code.', action='store_true', default=False)
group.add_argument('-d', '--diversions', help='Flag for generating diversion object code.', action='store_true', default=False)
group.add_argument('-p', '--parcels', help='Flag for generating irrigation parcel object code.', action='store_true', default=False) 

args = parser.parse_args()
output = path.abspath(args.outputDirectory) if args.outputDirectory else path.abspath('../generated_code')

div_dir = path.join(output, 'charm_diversions')
res_dir = path.join(output, 'charm_reservoirs')
fact_dir = path.join(output, 'charm_factory')

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

input_dir = path.abspath(args.inputDirectory)

res_file = path.join(input_dir, path.join('reservoirs', 'res_data.csv'))
reg_file = path.join(input_dir, path.join('reservoirs', 'region_data.csv'))
div_file = path.join(input_dir, path.join('diversions', 'diversion_data.csv'))
parcel_file = path.join(input_dir, path.join('parcels', 'parcel_data.csv'))

#CDSS crop types [1=GRASS_PASTURE, 2=ALFALFA, 3=BLUEGRASS, 4=SMALL_GRAINS, 99=missing data]
#ASSUMING unknown is likely grass, and bluegrass is similar to grass hay.  GRAIN not likely,
#but here for completeness.
cropMap = {1:'GRASS', 2:'ALFALFA', 3:'GRASS', 4:'GRAIN', 99:'GRASS'}
#CDSS Irrigation types [1= FLOOD, 2= FURROW, 3= SPRINKLER, 4= DRIP, 99=missing data]
#ASSUMING furrow is similar enough to flood, and drip is similar enough to PIVOT
irrigMap = {1:'FLOOD', 2:'FLOOD', 3:'PIVOT', 4:'PIVOT', 99:'FLOOD'}

"""
A bug in charmc causes segfault when parsing more than 334 modules from a single .ci file.
So each object type is split into multiple .ci files with no more than 334 modules per file.
"""
parCount = 1
parFileCount = 1 
parFile = 'parcels.ci'
divCount = 1
divFileCount = 1
divFile = 'diversions.ci'
resCount = 1
resFileCount = 1
resFile = 'reservoirs.ci'

def makeParcelClassFiles(s):
    #names Name, CalYear, Region, SubRegion, CropType, IrrigType, LandSize, DecreedRate, ParcelRatioToDiv, ElementIDs
    s = s.replace(pd.np.nan, -1)
    p = pt.Parcel()
    p.name(s.Name)
    p.parcelID(str(s.Name)[3:])
    p.cropType(cropMap[s.CropType])
    p.irrigType(irrigMap[s.IrrigType])
    p.area(s.LandSize)
    decrees = [d.strip() for d in s.DecreedRate.split(';')]
    decreeMap = {}
    for decree in decrees:
        key, value = decree.split('=')
        decreeMap[key] = value
    p.decreedAmount(decreeMap)
    #TODO/FIXME For now, setting requests to be decrees
    p.requestAmount(decreeMap)
    
    elements = [e.strip() for e in str(s.ElementIDs).split(';')]
    if elements[0] == '-1':
        print "WARNING: Parcel {} has no elements mapped.".format(s.Name)
    p.elementIds(elements)

    with open(path.join(div_dir, str(s.Name)+".cpp"), 'w') as output:
        output.write(str(p))

    global parFile
    global parFileCount
    global parCount
    if parCount % 335 == 0:
        parFile = 'parcels{}.ci'.format(parFileCount)
        parFileCount = parFileCount + 1

    with open(path.join(div_dir, parFile), 'a') as output:
        output.write(str(pt.ParcelCI(s.Name)))
    parCount = parCount + 1

def makeDivClassFiles(s):
    #names Name, ElementID, DecreedAmount, RequestedAmount, ApproriationDate, ParcelIds
    s = s.replace(pd.np.nan, -1)
    div = dt.Diversion()
    div.name(s.Name)
    div.diversionID(str(s.Name)[3:])
    div.elementID(s.ElementID)
    div.decreedAmount(s.DecreedAmount)
    div.requestedAmount(s.RequestedAmount)
    d = str(s.AppropriationDate)
    #Break up the date yyyymmdd into its components
    div.appropriationDate(int(d[0:4]), int(d[4:6]), int(d[6:8]))
    p = [i.strip() for i in str(s.ParcelIds).split(';')]
    if p[0] == '-1':
        print "WARNING: Diversion {} has no parcels.".format(s.Name)
    div.parcelIds(p)
    div.upstreamIDs([])

    with open(path.join(div_dir, str(s.Name)+".cpp"), 'w') as output:
        output.write(str(div))

    global divFile
    global divFileCount
    global divCount
    if divCount % 335 == 0:
        divFile = 'diversions{}.ci'.format(divFileCount)
        divFileCount = divFileCount + 1

    with open(path.join(div_dir, divFile), 'a') as output:
        output.write(str(dt.DiversionCI(s.Name)))
    divCount = divCount + 1

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
    global resFile
    global resFileCount
    global resCount
    if resCount % 335 == 0:
        resFile = 'resrvoirs{}.ci'.format(resFileCount)
        resFileCount = resFileCount + 1
    with open(path.join(res_dir, resFile), 'a') as output:
        output.write(str(rt.ReservoirCI(s.Name)))
    resCount = resCount + 1

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
    #with open(path.join(res_dir, "reservoirs.ci"), 'w') as output:
    #    pass
    for f in glob(path.join(res_dir, 'reservoirs*.ci')):
        remove(f)

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
    #with open(path.join(div_dir, "diversions.ci"), 'w') as output:
    #    pass
    for f in glob(path.join(div_dir, 'diversions*.ci')):
        remove(f)

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
    #with open(path.join(div_dir, "parcels.ci"), 'w') as output:
    #    pass
    for f in glob(path.join(div_dir, 'parcels*.ci')):
        remove(f)

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
    if(args.reservoirs): 
        generateReservoirs()
    if(args.diversions):
        generateDiversions()
    if(args.parcels):
        generateParcels()
    if(args.reservoirs or args.diversions or args.parcels):
        generateCommon()    
  
if __name__ == "__main__":
    main()
