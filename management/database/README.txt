The database directory should contain inputs for the charm code generation script.
Documentation on the input formats can be found in management/documents.

This directory will contain sub directories for each mesh, named after the mesh.
For example, the District 58 data is contained in the directory co_district_58.

The exceptions are the misc directory, which contains various testing and other
miscellaneous data, and the none directory, which contains no resevoirs,
diversions, or parcels, and can be used with any map to run with water
management structures turned off.

Each mesh subdirectory must contain the following subdirectories:
    reservoirs
    diversions
    parcels

Under reservoirs, the following files must be present:
    res_data.csv
    region_data.csv

Under diversions, the following files must be present:
    diversion_data.csv

Under parcels, the following files must be present:
    parcel_data.csv

For a structure that looks like

database
    co_district_58
        reservoirs
            res_data.csv
            region_data.csv
        diversions
            diversion_data.csv
        parcels
            parcel_data.csv

    <other mesh dir>
        reservoirs
            res_data.csv
            region_data.csv
        diversions
            diversion_data.csv
        parcels
            parcel_data.csv

