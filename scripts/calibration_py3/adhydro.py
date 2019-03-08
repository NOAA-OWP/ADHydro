import pandas as pd
import geopandas as gpd
import netCDF4 as nc
import os
"""
Notes:
When parameter preprocessing occurs (scripts/parameter_preprocessing.py), road network representations get assigned as 'impervious' elements.
This is done by giving them an 'urban' land cover type (1) and bedrock-like soil (15).  When we attempt to re-classify the urban classes
into the proper 4 class (open, low, meduim, high) these cause trouble, because roads in non-urban areas aren't considered developed by NLCD.

Also, for mesh elements that fall over open-water classes in NLCD, ADHydro mesh massage assigns these the an aribitray neighbor land cover value.
So if this neighbor happens to be urban developed (1), then these elements are changed from 16 to 1, but when the NLCD is read again, these elements
come back as 11 (the NLCD open water class).

This was verified on the sugar_creek mesh with roads.

Current proposed solution is to take all classes not mapped to development by the real_nlcd below and assign open water to medium development,
and assign all others (these are road segments) as high developed.

FIXME a better way to verify these errors each time a calibration is initialized is needed. This can be done by inspecting the intermediate data
produced by parameter_preprocessing.py, or by re-inspecting the road network shape file.  Anything that can't be properly attributed to correct
classes needs to flag an error/warning for further inspection.

for sugar_creek, manual inspection outside of this script verified all the mapping errors, so the above solution is applied and we carry on.
"""

from utils import Configuration
#Only used for NLCD...FIXME later
from qgis.core import * #VERY BAD PRACTICE...SHOULD FIND ALL REFERENCES AND PREPEND QGIS.CORE. RATHER THAN IMPORTING ALL OF QGIS.CORE
data_path = '/project/ci-water/data'
#ENUM Dictionariers for land cover types
urban_development_map = {"open_development":121, "low_development":122, "medium_development":123, "high_development":124}
nlcd_development_map = {21:"open_development", 22:"low_development", 23:"medium_development", 24:"high_development"}
#These are the mappings of land cover classes from NLCD to ADHYDRO (which is just a mapping into the NoahMP land classes, see parameter_preprocessing.py)
land_cover_mapping = {'water':16, 'wooded wetland':18, 'herbaceous wetland':17, 'barren land':19, 'shrub':8, 'grassland':7, 'pasture':2, 'cultivated':3, 'deciduous forest':11, 'evergreen forest':13, 'mixed forest':15, 'developed open':121, 'developed low':122, 'developed medium':123, 'developed high':124}
land_cover_inverse = {v:k for k,v in land_cover_mapping.items()}

#mapping of soil texture classes to ADHYDRO (see parameter_preprocessing.py)
soil_texture_mapping = {'sand':1, 'loamy sand':2, 'sandy loam':3, 'silt loam':4, 'silt':5, 'loam':6, 'sandy clay loam':7, 'silt clay loam':8, 'clay loam':9, 'sandy clay':10, 'silty clay':11, 'clay':12, 'rock_or_rubble':15, 'water':14 }
soil_texture_inverse = {v:k for k,v in soil_texture_mapping.items()}

def read_nlcd(mesh_veg, geometry_file):
    mesh_df = pd.DataFrame(mesh_veg)
    #Get geometry information for mesh elements
    geometry_nc = nc.Dataset(geometry_file)
    mesh_df['AD_X'] = geometry_nc.variables['meshElementX'][0][mesh_df.index.values]
    mesh_df['AD_Y'] = geometry_nc.variables['meshElementY'][0][mesh_df.index.values]
    #Debugging coordinate issues, FIXME remove
    qgishome = '/project/ci-water/tools/opt/'
    app = QgsApplication([], True)
    app.setPrefixPath(qgishome, True)
    app.initQgis()
    #FIXME a lot of this is hard coded....path, PROJ string...quick hack to get this going...
    NLCD_path = os.path.join(data_path, os.path.join("NLCD", "nlcd_2011"))
    NLCD_rast = os.path.join(NLCD_path, 'nlcd_2011_landcover_2011_edition_2014_10_10.img')
    NLCD_layer = QgsRasterLayer(NLCD_rast)
    #AD_HYDRO_PROJ_WKT = 'PROJCS["Sinusoidal",GEOGCS["GCS_WGS_1984",DATUM["D_WGS_1984",SPHEROID["WGS_1984",6378137.0,298.257223563]],PRIMEM["Greenwich",0.0],UNIT["Degree",0.0174532925199433]],PROJECTION["Sinusoidal"],PARAMETER["false_easting",20000000.0],PARAMETER["false_northing",10000000.0],PARAMETER["central_meridian",-81.0],UNIT["Meter",1.0]]'


    AD_HYDRO_PROJ_WKT = 'PROJCS["Sinusoidal", GEOGCS["GCS_WGS_1984", DATUM["WGS_1984", SPHEROID["WGS_84",6378137,298.257223563]], PRIMEM["Greenwich",0], UNIT["Degree",0.017453292519943295]], PROJECTION["Sinusoidal"], PARAMETER["longitude_of_center",-81], PARAMETER["false_easting",20000000], PARAMETER["false_northing",10000000], UNIT["Meter",1]]'
    ADHydro_crs = QgsCoordinateReferenceSystem(AD_HYDRO_PROJ_WKT)
    NLCD_xform = QgsCoordinateTransform(ADHydro_crs, NLCD_layer.crs())



    def getType(s):
        point = NLCD_xform.transform(QgsPoint(s['AD_X'], s['AD_Y']))
        #Get NLCD value for this point
        query = NLCD_layer.dataProvider().identify(point, QgsRaster.IdentifyFormatValue)
        #If query worked, set value if the query returned a known type, otherwise, set to unknown (-1)
        if query.isValid():
            #TODO cannot map 21, 22, 23, 24 since 22 has an existing mapping to NoahMP classes... FIXME
            #print( query.results() )
            result = int(query.results()[1])
            if result in list(nlcd_development_map.keys()):
                s['land_cover'] = int( urban_development_map[ nlcd_development_map[ result ] ] )
            else:
                s['land_cover'] = result
        return s
    mesh_df = mesh_df.apply(getType, axis=1)
    mesh_df.drop(['AD_X', 'AD_Y'], axis=1, inplace=True)
    return mesh_df['land_cover']

"""
    ADHydroParameter is a wrapper that holds STATIC (i.e. non-calibrated) data from
    the ADHydro mesh which is needed to calibrate other parameters
"""
class ADHydroParameter(object):
    def __init__(self, key, elements, data, adhydroName):
        #self.elements = elements
        #self.key = key
        self.data = pd.Series(data.variables[adhydroName][0][elements],index=elements, name=key)

class MeshVegitationType(ADHydroParameter):
    def __init__(self, key, elements, data, hackGeomFile):
        super(MeshVegitationType, self).__init__(key, elements, data, 'meshVegetationType')
        #self.data = pd.Series(data.variables['meshVegetationType'][0][elements],index=elements, name=key)
        #Map int classes to string class, should be mapped to same names as found in meshManningsN.csv
        self.fix_nlcd(hackGeomFile)
        self.data = self.data.map(lambda x: land_cover_inverse[x])

    def fix_nlcd(self, geometry_file):
        #reread NLCD for better land classification
        development_index = self.data[ self.data == 1 ].index
        print("Looking up land cover for {} urban elements".format(len(development_index)))
        self.data.loc[ development_index ] = read_nlcd(self.data.loc[ development_index ], geometry_file)

        water_body_index =  self.data.loc[ development_index ][ self.data.loc[ development_index ] == 11 ].index
        self.data.loc[ water_body_index ] = urban_development_map['medium_development']

        road_index = self.data.loc[ development_index ][ ~self.data.loc[ development_index ].isin(list(urban_development_map.values())) ].index
        self.data.loc[ road_index ] = urban_development_map['high_development']

    """
    def fix_nlcd(self):
        #reread NLCD for better land classification
        development_index = self.mesh_calibration_df[ self.mesh_calibration_df['land_cover'] == 1 ].index
        print "Looking up land cover for {} urban elements".format(len(development_index))
        self.mesh_calibration_df.loc[ development_index, 'land_cover' ] = read_nlcd(self.mesh_calibration_df.loc[ development_index ], self.config.geometry_file)

        water_body_index =  self.mesh_calibration_df.loc[ development_index ][ self.mesh_calibration_df.loc[ development_index ]['land_cover'] == 11 ].index
        self.mesh_calibration_df.loc[ iwater_body_index, 'land_cover' ] = urban_development_map['medium_development']

        road_index = mesh_calibration_df.loc[ development_index ][ ~mesh_calibration_df.loc[ development_index ]['land_cover'].isin(urban_development_map.values()) ].index
        self.mesh_calibration_df.loc[ road_index, 'land_cover' ] = urban_development_map['high_development']
    """

class MeshSoilType(ADHydroParameter):
    def __init__(self, key, elements, data):
        super(MeshSoilType, self).__init__(key, elements, data, 'meshSoilType')
        #self.data = pd.Series(data.variables['meshSoilType'][0][elements], index=elements, name=key)
        #Map int classes to string class, should be mapped to same names as found in meshConductivity.csv
        self. data = self.data.map(lambda x: soil_texture_inverse[x])

class ChannelReachCode(ADHydroParameter):
    def __init__(self, key, elements, data):
        super(ChannelReachCode, self).__init__(key, elements, data, 'channelReachCode')
        #self.data = pd.Series(data.variables['channelReachCode'][0][elements, index=elements, name=key)

"""
    CalibrationParameter is a wrapper that holds calibration parameters and the various
    dependencies that are required to calibrate that parameter
"""
class CalibrationParameter(object):
    def __init__(self, key, elements, data, adhydroName, dependents=[]):
        #self.elements = elements
        self.key = key
        tmp = {}
        for d in dependents:
            tmp[d.data.name] = d.data
        tmp[key] = data.variables[ adhydroName ][0][elements]
        self.data = pd.DataFrame( tmp, index=elements )
        self.adhydroName = adhydroName

    def update_parameter(self, parameter_nc):
        #TODO unify adhydroName and key???
        parameter_nc.variables[self.adhydroName][0, self.data.index] = self.data[self.key].values

    def update_class(self, state):
        name = state[ 'index' ]
        data = self.groups.get_group(name)
        #print( data )
        #print( "\n\n\n\n\n" )
        #print( self.data )
        index = data.index
        value = state.iloc[1]
        self.data.loc[ index, self.key ] = value
        #parameter_nc.variables[self.adhydroName][0, index] = value

    def update_state(self, state, best):
        state[ state['param'] == self.key ] [ ['index', best] ].apply(self.update_class, axis=1) 

class MeshConductivity(CalibrationParameter):
    """
        REQUIRES meshVegitationType and meshSoilType
    """
    def __init__(self, key, elements, data, vegitation, soil):
        if not isinstance(vegitation, MeshVegitationType):
            raise TypeError("MeshManningsN requires vegitation to be a MeshVegitationType, got {}".format(type(vegitation)))
        if not isinstance(soil, MeshSoilType):
            raise TypeError("MeshManningsN requires soil to be a MeshSoilType, got {}".format(type(soil)))
        super(MeshConductivity, self).__init__(key, elements, data, 'meshConductivity', [vegitation, soil])
        #get elements groupbed by landcover/soiltexture classes for ksat calibration
        self.groups = self.data.groupby(['land_cover', 'soil_texture'])
        #Read ksat info from csv file.  The values in this file are in cm/h, adhydro uses m/s, must convert min/max by (cm/hr)/360000
        #TODO/FIXME make the csv a parameter??? hard coded read here not ideal
        ksat_df = pd.read_csv('meshConductivity.csv', skipinitialspace=True) #index_col=['land_cover', 'soil_texture'])
        ksat_df['Ks_min'] = ksat_df['Ks_min']/360000
        ksat_df['Ks_max'] = ksat_df['Ks_max']/360000
        ksat_df['init'] = (ksat_df['Ks_min'] + ksat_df['Ks_max']) / 2.0
        #get ksat classes
        ksat_classes = ksat_df.groupby(['land_cover', 'soil_texture'])
        #Hold data for each class found in the desired calibration region for meshConductivity
        self.calibration_df = self.groups.filter(lambda x: x.name in list(ksat_classes.groups.keys()))
        calibration_rows = []
        keys = list(self.calibration_df.groupby(['land_cover', 'soil_texture']).groups.keys())
        for k in keys:
            #name = '{}_{}'.format(key[0], key[1])
            data = ksat_classes.get_group(k).iloc[0]
            calibration_rows.append({'init':data['init'],
                                     'lower':data['Ks_min'],
                                     'upper':data['Ks_max'],
                                     'param':self.key,
                                     'index':k})
        self.calibration_df = pd.DataFrame(calibration_rows)

 
class MeshManningsN(CalibrationParameter):
    """
        REQUIRES meshVegitationType
    """
    def __init__(self, key, elements, data, vegitation):
        if not isinstance(vegitation, MeshVegitationType):
            raise TypeError("MeshManningsN requires vegitation to be a MeshVegitationType, got {}".format(type(vegitation)))
        super(MeshManningsN, self).__init__(key, elements, data, 'meshManningsN', [vegitation])
        #TODO consider promoting to dataframes for better readability of groupby statements
        #Get elements grouped by land cover class for roughness calibration
        #self.groups = vegitation.data.groupby(vegitation.data)
        self.groups = self.data.groupby('land_cover')
        #read manningsN calibration info for mesh and channels
        #TODO/FIXME hard coded file read of csv
        self.calibration_parameters = pd.read_csv('meshManningsN.csv', skipinitialspace=True, index_col='land_cover')
        self.calibration_parameters['init'] = (self.calibration_parameters['lower'] + self.calibration_parameters['upper']) / 2.0
        #Get non-fixed mannings N classes
        self.classes = self.calibration_parameters[ self.calibration_parameters['fixed'] == False ].index
        #Initialize all mannins n parameters, both fixed and non-fixed, to be the median between lower and upper
        #provided in the meshManninsN.csv table.  For fixed values, lower = upper, so we apply the same function to all
        self.data[self.key] = self.data.apply( lambda x: self.calibration_parameters.loc[x['land_cover'], 'init'], axis=1) 

        #find the intersection of classes that exist in the mesh_calibration_df and the user input classes that are not to be calibrated
        # and only save those
        calibration_rows = []
        for land_cover in self.data['land_cover'].unique():
            if land_cover in self.classes:
                calibration_rows.append({'init':self.calibration_parameters.loc[land_cover]['init'],
                                         'lower':self.calibration_parameters.loc[land_cover]['lower'],
                                         'upper':self.calibration_parameters.loc[land_cover]['upper'],
                                         'param':self.key,
                                         'index':land_cover})
        self.calibration_df = pd.DataFrame(calibration_rows)

class ChannelManningsN(CalibrationParameter):
    """
        REQUIRES channelStreamOrder
    """
    def __init__(self, key, elements, data):
        super(ChannelManningsN, self).__init__(key, elements, data, 'channelManningsN')
        #FIXME compose from channeLReachCode class
        self.data['reachCode'] = data.variables[ 'channelReachCode' ][0][elements]
        self.find_stream_order()
        calibration_rows = []
        #Only searching for provided stream orders that exist in the elements we are calibrating
        for stream_order in self.data['streamOrder'].unique():
            calibration_rows.append({'init':self.calibration_parameters.loc[stream_order]['init'],
                             'lower':self.calibration_parameters.loc[stream_order]['min'],
                             'upper':self.calibration_parameters.loc[stream_order]['max'],
                             'param':self.key,
                             'index':stream_order}) 
        self.calibration_df = pd.DataFrame(calibration_rows)
        #print( self.calibration_df )
    
    #FIXME split find stream order and reading of channelManningsN parameters
    def find_stream_order(self):
        #TODO/FIXME find stream order parameter code from input interperter and push to master ASAP
        #in the mean time, hack in stream order reading for sugar creek from original shapefile
        channel_meta_df = gpd.read_file('/project/ci-water/data/maps/sugar_creek/ArcGIS/streams_singlepart.shp')
        stream_orders = channel_meta_df.groupby('LINKNO').first()['strmOrder']
        #print( stream_orders.reindex( self.data['reachCode'] )  ) == stream_orders.loc[ self.data['reachCode'] )
        #print( self.data )
        #print( stream_orders )
        self.data['streamOrder'] = stream_orders.reindex( self.data['reachCode'] ).values
        #If we were not able to find stream order from the shapefile above, then it is likely a reservoir,
        #either way, canpt proceed if it is NaN, so drop it...
        self.data.dropna(subset=['streamOrder'], inplace=True)
        #Initialize channel mannings N values
        self.groups = self.data.groupby('streamOrder')
        self.calibration_parameters = pd.read_csv('channelManningsN.csv', skipinitialspace=True, index_col=0)
        self.calibration_parameters['init'] = (self.calibration_parameters['min'] + self.calibration_parameters['max'] ) / 2.0
        #print(self.calibration_parameters)
        #print(self.data)
        self.data[self.key]  = self.data.apply( lambda x: self.calibration_parameters.loc[x['streamOrder'], 'init'], axis=1)  

class ADHydroCalibration():
    def __init__(self, config, calibration_regions):
        self.config = config
       
        #needed variables for calibration ['meshRegion', 'meshVegetationType', 'meshSoilType', 'meshManningsN', 'meshConductivity', 'channelRegion', 'channelManningsN']
        #Read the calibration parameter netcdf file
        self.parameter_nc = nc.Dataset( self.config.calibration_parameter_file, 'a')
        #Get the list of mesh regions
        mesh_regions = pd.Series( self.parameter_nc.variables['meshRegion'][0,:] )
        #find all elements in regions defined in calibration_regions
        mesh_elements = mesh_regions.index[ mesh_regions.isin(calibration_regions) ]

        #Get the list of channel regions
        channel_regions = pd.Series( self.parameter_nc.variables['channelRegion'][0,:] )
        channel_elements = channel_regions.index [ channel_regions.isin(calibration_regions) ]
        #Build up the calibration data/parameters

        #Index classes to use for calibration variables
        #FIXME uncomment
        self.veg = MeshVegitationType('land_cover', mesh_elements, self.parameter_nc, self.config.geometry_file)
        self.soil = MeshSoilType('soil_texture', mesh_elements, self.parameter_nc)
        #TODO if the interface is well defined, we can hold an arbitrary collection of calibration parameters
        #and simply let each class handle the details of updating ect...
        #calibration_variables = [] 
        #FIXME uncomment
        self.meshManningsN = MeshManningsN('manningsN', mesh_elements, self.parameter_nc, self.veg)
        self.ksat = MeshConductivity('meshConductivity', mesh_elements, self.parameter_nc, self.veg, self.soil)
        self.channelManningsN = ChannelManningsN('channelManningsN', channel_elements, self.parameter_nc)
        #Collection of vars used to make calling functions work on arbitray params which are currently hard coded above ^^^ FIXME
        self.calibration_vars = [self.meshManningsN, self.ksat, self.channelManningsN]
        #print( self.meshManningsN.calibration_df )
        #print( self.ksat.calibration_df )
        #print( self.channelManningsN.calibration_df )
        self.calibration_df = pd.concat( [self.meshManningsN.calibration_df, self.ksat.calibration_df, self.channelManningsN.calibration_df] )
        self.calibration_df.reset_index(inplace=True)
        #Update parameter file with initial values for all calibration variables
        self.update_parameter_file()

    def update_parameter_file(self):
        #print self.parameter_nc['meshManningsN'][0][ self.meshManningsN.data.index ]
        #self.meshManningsN.update_parameter(self.parameter_nc)
        #self.channelManningsN.update_parameter(self.parameter_nc)
        #self.ksat.update_parameter(self.parameter_nc)
        for var in self.calibration_vars:
            var.update_parameter(self.parameter_nc)
        #print self.parameter_nc['meshManningsN'][0][ self.meshManningsN.data.index ]
        #TODO sync parameter_nc
        self.parameter_nc.sync()

    
    def update_state(self, best):
        for var in self.calibration_vars:
            var.update_state(self.calibration_df, best)
            var.update_parameter(self.parameter_nc)
        self.parameter_nc.sync()

    def group_classes(self):
        """
            Remove this, all functionality isololated inside each calibration paramter class
        """
        pass
        #FIXME clean up file reads in this function!!!!
        #Get elements grouped by land cover class for roughness calibration
        #manningsN_group = mesh_calibration_df.groupby('land_cover')
        #get elements groupbed by landcover/soiltexture classes for ksat calibration
        #ksat_group = mesh_calibration_df.groupby(['land_cover', 'soil_texture'])

        #Read ksat info from csv file.  The values in this file are in cm/h, adhydro uses m/s, must convert min/max by (cm/hr)/360000
        #ksat_df = pd.read_csv('meshConductivity.csv', skipinitialspace=True) #index_col=['land_cover', 'soil_texture'])
        #ksat_df['Ks_min'] = ksat_df['Ks_min']/360000
        #ksat_df['Ks_max'] = ksat_df['Ks_max']/360000
        #ksat_df['init'] = (ksat_df['Ks_min'] + ksat_df['Ks_max']) / 2.0
        #read manningsN calibration info for mesh and channels
        #mesh_mannings_n_df = pd.read_csv('meshManningsN.csv', skipinitialspace=True, index_col='land_cover')
        #TODO/FIXME find stream order parameter code from input interperter and push to master ASAP
        #in the mean time, hack in stream order reading for sugar creek from original shapefile
        #channel_meta_df = gpd.read_file('/project/ci-water/data/maps/sugar_creek/ArcGIS/streams_singlepart.shp')
        #stream_orders = channel_meta_df.groupby('LINKNO').first()['strmOrder']
        #channel_calibration_df['streamOrder'] = stream_orders.loc[ channel_calibration_df['reachCode'] ].values
        #If we were not able to find stream order from the shapefile above, then it is likely a reservoir,
        #either way, canpt proceed if it is NaN, so drop it...
        #channel_calibration_df.dropna(subset=['streamOrder'], inplace=True)
        #channel_group = channel_calibration_df.groupby('streamOrder')
        #channel_mannings_n_df = pd.read_csv('channelManningsN.csv', skipinitialspace=True)

        #Get non-fixed mannings N classes
        #manningsN_classes = mesh_mannings_n_df[ mesh_mannings_n_df['fixed'] == False ].index
        #get ksat classes
        #ksat_classes = ksat_df.groupby(['land_cover', 'soil_texture'])
        #Hold data for each class found in the desired calibration region for meshConductivity
        #ksat_calibration_df = ksat_group.filter(lambda x: x.name in ksat_classes.groups.keys())


 

 
