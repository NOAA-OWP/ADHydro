#!/usr/bin/env python
import matplotlib
matplotlib.use('Qt5Agg')
import matplotlib.pyplot as plt
import os
import pandas as pd
import geopandas as gpd
import netCDF4 as nc

from shapely.geometry import Point, Polygon
adhydro_projection = '+proj=sinu +lon_0={} +x_0=20000000 +y_0=10000000 +datum=WGS84 +units=m +no_defs'.format(-81)

domain = gpd.read_file('sub9_catchment.shp', crs={'init' :'epsg:4326'} )
domain = domain.to_crs(crs=adhydro_projection)
area = domain['Area_mi2'][0]
adhydro_extent = gpd.read_file('adhydro_extent.shp')
#print("Area {}".format(area))
area = adhydro_extent.iloc[0].geometry.area
#print("Area {} m^2".format(area))

#print( "Area of Sub {}".format(area))
aorc_area = (15+18)*(765.3078651577234*924.4040725033035)
#print( "Area of AORC Sub {}".format(aorc_area ) )
#print( "Ratio {}".format(aorc_area/area) )
s = pd.read_csv('domain_precip_rates.csv', names=['Time', 'Rate'], parse_dates=['Time']).set_index('Time') #m^3/s

s = s/area
#s *= 360000
#s *= 360000000
#print( s.describe() )
s.plot()




usgs =  '02146300' #for 7462 -> '02146285'
observed_file = 'usgs_{}_observed.csv'.format(usgs)
observed_hydrograph = pd.read_csv(observed_file, parse_dates=['Date_Time'], ).rename(index=str, columns={'Discharge_cms':'observed'}).set_index('Date_Time')
observed_hydrograph = observed_hydrograph.tz_localize('UTC')

regions  = [21, 32, 40, 42, 57, 65, 66, 68, 74, 77,
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


parameter_nc = nc.Dataset('/project/ci-water/data/maps/sugar_creek/drain_down/parameter.nc')
geometry_nc = nc.Dataset('/project/ci-water/data/maps/sugar_creek/drain_down/geometry.nc')
mesh_regions = pd.Series( parameter_nc.variables['meshRegion'][0,:] )
mesh_elements = mesh_regions.index[ mesh_regions.isin(regions) ]
channel_regions = pd.Series( parameter_nc.variables['channelRegion'][0,:] )
channel_elements = channel_regions.index[ channel_regions.isin(regions) ]

"""
adhydroMesh = {'X':geometry_nc.variables['meshVertexX'][0][mesh_elements], 'Y':geometry_nc.variables['meshVertexY'][0][mesh_elements]}
adhydroMeshVerticies =[Polygon(p) for p in [ list(zip(xs,ys)) for xs, ys in zip(adhydroMesh['X'], adhydroMesh['Y']) ] ]
adhydroMeshGeometry = gpd.GeoSeries(adhydroMeshVerticies, crs=adhydro_projection)
meshFrame = gpd.GeoDataFrame([], geometry=adhydroMeshGeometry)
#adhydroMeshGeometry.unary_union().to_file('adhydro_mesh_boundary.shp')
adhydro_extent = gpd.overlay(meshFrame, domain, how='intersection')
adhydro_extent.to_file('adhydro_sub_intersect')

adhydro_extent = gpd.read_file('adhydro_sub_intersect')
plt.figure()
#adhydro_extent.plot()
adhydro_extent = gpd.GeoDataFrame([], geometry=gpd.GeoSeries(Polygon(adhydro_extent.unary_union.exterior)),crs=adhydro_projection)
adhydro_extent.to_file('adhydro_extent.shp')
"""
#adhydro_extent.plot(facecolor='none')
mesh_area = pd.Series(geometry_nc.variables['meshElementArea'][0][mesh_elements], name='area', index=mesh_elements)
#channel_area = pd.Series(geometry_nc.variables['channelElementArea'][0][channel_elements], name='area', index=channel_elements)

hydrograph_reference_date = 2455772.5
#forcing = nc.Dataset('./delme.nc')
#forcing = nc.Dataset('../forcing_improved_2011-07-30_2011-08-8.nc')
forcing = nc.Dataset('./please_be_fixed.nc')
mesh_precip = pd.DataFrame(forcing.variables['TPREC'][:,], index=forcing.variables['JULTIME'][:])
mesh_precip = mesh_precip[mesh_elements]
mesh_precip['time'] = mesh_precip.index
mesh_precip['time'] = pd.to_datetime(mesh_precip['time'], utc=True, unit='D', origin='julian').dt.round('1s')
mesh_precip.set_index('time', inplace=True) # m/s
mesh_precip.fillna(0.0, inplace=True)
"""
channel_precip = pd.DataFrame(forcing.variables['TPREC_C'][:,], index=forcing.variables['JULTIME'][:])
channel_precip = channel_precip[channel_elements]
channel_precip['time'] = channel_precip.index
channel_precip['time'] = pd.to_datetime(channel_precip['time'], utc=True, unit='D', origin='julian').dt.round('1s')
channel_precip.set_index('time', inplace=True) # m/s
channel_precip.fillna(0.0, inplace=True)
"""
mesh_volume = mesh_precip.apply(lambda x: x*mesh_area, axis=1)
#channel_volume = channel_precip.apply(lambda x: x*channel_area, axis=1)

#basin_precip = (mesh_volume.sum(axis=1) + channel_volume.sum(axis=1)) / (mesh_area.sum() + channel_area.sum())
basin_precip = mesh_volume.sum(axis=1) / area #mesh_area.sum()
#basin_precip = basin_precip / mesh_area_total
print(basin_precip.describe())
print("Total {}".format(basin_precip.sum()))
basin_precip.plot(ax=plt.gca(), label='ADHydro Rate')
plt.legend()
plt.show()
print("ADHydro area ", mesh_area.sum())
print("S area ", area)
os._exit(1)

mesh_area = pd.Series(geometry_nc.variables['meshElementArea'][0][mesh_elements], name='area', index=mesh_elements)
mesh_area.to_csv('mesh_element_areas_in_domain.csv')

common_index = mesh_precip.index
observed_hydrograph = observed_hydrograph.loc[common_index]


def test(s):
    return s*mesh_area

vol_per_element = mesh_precip.apply(test, axis=1)
#print(vol_per_element)
total_volume_per_time = vol_per_element.sum(axis=1)
#print(total_volume_per_time)
#mesh_flow.plot()
#plt.show()
volume = (total_volume_per_time.shift(-1)+total_volume_per_time)*(15*60)
#print(volume)
observed_volume = (observed_hydrograph['observed'].shift(-1)+observed_hydrograph['observed'])*(15*60)
#print(observed_volume)
print("Area under curves, precip, observed")
print(volume.sum())
print(observed_volume.sum())
print(mesh_area.sum())

total_volume_per_time.to_csv('total_precip_per_time_over_domain_cms.csv')
observed_hydrograph['observed'].to_csv('observed_usgs_station_{}_cms.csv'.format(usgs))

#channel_precip = forcing.variables['TPREC_C']


def read_adhydro_out(iteration):
    simulated_hydrograph_file = 'state.nc.7510.txt'
    simulated_hydrograph = pd.read_csv(simulated_hydrograph_file, header=None, usecols=[1,2], names=['time', 'simulated_{}'.format(iteration)])
    simulated_hydrograph['time'] /= 86400 #partial day
    simulated_hydrograph['time'] += hydrograph_reference_date #Julian date from reference
    simulated_hydrograph['time'] = pd.to_datetime(simulated_hydrograph['time'], utc=True, unit='D', origin='julian').dt.round('1s')
    simulated_hydrograph.drop_duplicates('time', keep='last', inplace=True)
    simulated_hydrograph.set_index('time', inplace=True)
    return simulated_hydrograph

df = observed_hydrograph
simulated_hydrograph = read_adhydro_out(0)
simulated_hydrograph.loc[common_index].to_csv('adhydro_output_at_usgs_{}_cms.csv'.format(usgs))
df = pd.merge(simulated_hydrograph, df, left_index=True, right_index=True)
#simulated_hydrograph = read_adhydro_out(4)
#df = pd.merge(simulated_hydrograph, df, left_index=True, right_index=True)
df['precip_vol'] = total_volume_per_time
df.plot()
plt.show()

