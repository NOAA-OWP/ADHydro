#!/usr/bin/env python
import matplotlib
matplotlib.use('Qt5Agg')

import matplotlib.pyplot as plt

import pandas as pd
import geopandas as gpd

domain = gpd.read_file('sub9_catchment.shp')
area = domain['Area_mi2'][0]
print("Area {}".format(area))
area = area* 2.59e+6
print("Area {} m^2".format(area))

print( "Area of Sub {}".format(area))
aorc_area = (15+18)*1000
print( "Area of AORC Sub {}".format(aorc_area ) )
print( "Ratio {}".format(aorc_area/area) )
s = pd.read_csv('domain_precip_rates.csv', names=['Time', 'Rate'], parse_dates=['Time']).set_index('Time') #m^3/s

s = s/aorc_area
s *= 3600000
print( s.describe() )
s.plot()

plt.show()

