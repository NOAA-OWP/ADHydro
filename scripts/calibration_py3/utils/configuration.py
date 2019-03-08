import configparser
import argparse
import io
import os
import shutil

class Configuration():
    """
        Class for holding calibration configuration information
    """
    
    def __init__(self):
        parser = argparse.ArgumentParser(description="Calibrate an existing ADHydro model.")
        parser.add_argument('ADHydroSuperfile', help='The ADHydro superfile describing the model to be run/calibrated.')
        parser.add_argument('-w', '--workdir', help='Working dir to write temporary and final data to. Defaults to current working directory')
        parser.add_argument('-r', '--restart', help='Restart calibration in workdir, overwrites existing files.', default=False, action='store_true')
        args = parser.parse_args()
        
        if args.workdir and os.path.isdir(args.workdir):
            self.workdir = args.workdir
        else:
            self.workdir = './'

        if not os.path.isfile(args.ADHydroSuperfile):
            parser.error("Superfile '{}' does not exist.".format(args.ADHydroSuperfile))

        #We have a superfile that exists on disk, try to read it with configparser
        config = configparser.SafeConfigParser()
        config.optionxform = str #prevent removing camel case from name (default xform is lower case)
        try:
            config.read(args.ADHydroSuperfile)
        except configparser.MissingSectionHeaderError:
            #well formed INI files have section headers [], if none are found, assume the superfile defines all global config
            print(("WARNING: Superfile '{}' is missing section headers, adding [global] to the beginning of the file will prevent this warning.".format(args.ADHydroSuperfile)))
            with open(args.ADHydroSuperfile) as fp:
                #Hack a header onto the ini string so configparser will read it
                fixed = '[global]\n'+fp.read()
                fixed_fp = io.StringIO(fixed)
            config.readfp(fixed_fp)

        #We should now have a valid config, make sure we have a defined ADHydro model state ready for calibration
        #First check that the ini doesn't expect to init from ascii, because this won't work to calibrate

        #For this script, python 3 is ideal, configparser is much more robust in 3 and handles inline comments.  For some reason in 2.7 it doesn't well.
        #So have to treat everything as strings and hanlde them explicitly.
        if config.get('global', 'initializeFromASCIIFiles').split(';')[0].strip().lower() == 'true' :
            raise Exception("Cannot calibrate from ASCII files, run mesh massage and then initialize from netcdf for calibration")


        #TODO eventually we should re process parameters in the workflow to account for the developed types...for now,
        #we will re-read the NLCD and look thses up to avoid re-processing the entire mesh...
        self.geometry_file = ''
        if config.has_option('global', 'adhydroInputGeometryFilePath'):
            self.geometry_file = config.get('global', 'adhydroInputGeometryFilePath').split(';')[0].strip()
        elif config.has_option('global', 'adhydroInputDirectoryPath'):
            self.geometry_file = os.path.join( config.get('global', 'adhydroInputDirectoryPath').split(';')[0].strip(), 'geometry.nc' )

        if not os.path.isfile(self.geometry_file):
            parser.error("Error in superfile, cannot find specified geometry file {}".format(self.geometry_file))


        self.state_nc_file = ''
        #Find the state file to use
        if config.has_option('global', 'adhydroInputStateFilePath'):
            #User specified state file
            self.state_nc_file = config.get('global', 'adhydroInputStateFilePath').split(';')[0].strip()
        elif config.has_option('global', 'adhydroInputDirectoryPath'):
            self.state_nc_file = os.path.join( config.get('global', 'adhydroInputDirectoryPath').split(';')[0].strip(), 'state.nc' )

        self.initial_parameter_file = ''
        #Find the parameter file to use
        if config.has_option('global', 'adhydroInputParameterFilePath'):
            #User specified parameter file
            self.initial_parameter_file = config.get('global', 'adhydroInputParameterFilePath').split(';')[0].strip()
        elif config.has_option('global', 'adhydroInputDirectoryPath'):
            self.initial_parameter_file = os.path.join( config.get('global', 'adhydroInputDirectoryPath').split(';')[0].strip(), 'parameter.nc' )

        if not os.path.isfile(self.initial_parameter_file):
            parser.error("Error in superfile, cannot find specified parameter file {}".format(self.initial_parameter_file))

        #If we have already copied the init, warn the user and don't copy again, have them clean up if they need a fresh calibration in the workdir
        self.calibration_parameter_file = os.path.join(self.workdir, 'parameters.nc.calib')

        if os.path.isfile(self.calibration_parameter_file):
            if not args.restart:
                parser.error('A previous parameter.nc.calib file exists in workdir {}, remove the file, provide a clean workdir to calibrate, or set the -r flag to restart'.format(args.workdir))
        else:
            #Need to copy the initial parameter file we will update with calibration params
            print(("Copying initial parameters {} to {}/parameters.nc.calib".format(self.initial_parameter_file, self.workdir)))
            shutil.copy2(self.initial_parameter_file,  self.calibration_parameter_file)

        self.calibration_superfile = os.path.join(self.workdir, 'calibration_superfile.ini')
        self.config = config
        config.set('global', 'adhydroInputParameterFilePath', self.calibration_parameter_file)
        config.set('global', 'adhydroOutputDirectoryPath', self.workdir)
        #FIXME remove this, hacking for testing
        #config.set('global', 'simulationDuration', '86400')
        self.args = args
    def edit_superfile(self, parameters):
        for k, v in parameters.items():
            self.config.set('global', k, v)
    
    def write_superfile(self):
        #Need to write a new adhydro conf, but have to remove the [global] section, or adhydro won't read it correctly
        hack = []
        with open(self.calibration_superfile, 'w') as conf:
            self.config.write(conf)
        with open(self.calibration_superfile, 'r') as conf:
            hack = conf.readlines()
        with open(self.calibration_superfile, 'w') as conf:
            conf.writelines(hack[1:])

