# Use these values for Mt Moran.

# Use these values for EN3038 lab.

EXTRACPPFLAGS := -I/usr/include/libshp -fdiagnostics-show-option
EXTRALDFLAGS  := -lnetcdf -lshp -fdiagnostics-show-option

# Use these values for all.

CPPFLAGS := $(EXTRACPPFLAGS) -I../ADHydro -g -Wall
LDFLAGS  := $(EXTRALDFLAGS) -g -Wall

VPATH := ../ADHydro

EXES := adhydro_channel_preprocessing    \
        adhydro_mesh_check               \
        adhydro_create_channel_shapefile \
        adhydro_create_xdmf_file         \
        test_date_functions

all: $(EXES)

.PHONY: all

adhydro_channel_preprocessing: all.h # Uses implicit rule to compile from adhydro_channel_preprocessing.cpp.  This just adds the header file dependency.

adhydro_mesh_check: all.h # Uses implicit rule to compile from adhydro_mesh_check.cpp.  This just adds the header file dependency.

adhydro_create_channel_shapefile: all.h # Uses implicit rule to compile from adhydro_create_channel_shapefile.cpp.  This just adds the header file dependency.

adhydro_create_xdmf_file: all.h # Uses implicit rule to compile from adhydro_create_xdmf_file.cpp.  This just adds the header file dependency.

test_date_functions: all.h # Uses implicit rule to compile from test_date_functions.cpp.  This just adds the header file dependency.

clean:
	rm -f $(EXES) *.o

.PHONY: clean
