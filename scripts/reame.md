
# Utility and Analysis scripts supporting ADHydro

## Description of Directories
### analysis 
> Scripts are used to post process and analyze ADHydro inputs and outputs
>- get_obs.py 
>- max_depth_adhydro.py
>- mesh_histogram.py
>- mesh_parameters.py

### calibration_py3
> Supporting code for calibration and regionalization of calibration parameters.  The main scripts are
>- calibration.py
>- regionalize.py

### channels
>Scripts related specifically to channel processing.
>- hack_streams.py
>- print_stream_neighbor_info.py
>- stream_walk.py

### forcing
> Scripts to convert various forcing products to ADHydro netcdf forcing files.  Also includes some analysis scripts useful in the development and debugging of forcing issues.
>- aorc_to_adhydro.py
>- aorc_volume_anim.py
>- aorc_volume.py
>- nwm_retro_to_adhydro.py
>- nwm_to_adhydro.py
>- plot_aorc_vol.py
>- volume_check.py
>- wrf_to_adhydro.py

### gis
> GIS integration tools
>- adhydro_to_raster.py
>- gdal_clip.sh
>- generate_element_WKT.py
>- mesh_outline.py
>
### preprocessing
> ADHydro mesh preparation/preprocessing tools
>- create_triangle_files.py
>- create_z_file.py
>- parameter_preprocessing.py
>- prepare_soil_data.py

> Written with [StackEdit](https://stackedit.io/).
