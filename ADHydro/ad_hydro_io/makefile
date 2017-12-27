.PHONY: all netcdf

IO_OBJS := adhydro_io.o
NETCDFOBJS := netcdf/adhydro_netcdf.o
VPATH := ./netcdf

all: netcdf $(IO_OBJS)

netcdf: $(NETCDFOBJS)

netcdf/adhydro_netcdf.o: adhydro_netcdf.cc netcdf/netcdf_io_ops.h netcdf/netcdf_io_ops.cpp
	$(MAKE) -C netcdf

adhydro_io.o: adhydro_netcdf.cc netcdf/adhydro_netcdf.o
	$(CHARMC) $(CPPFLAGS) $< -o tmp.o
	$(LD) -r tmp.o $(NETCDFOBJS) -o $@ #These two lines nessicary because the charmc shell script doesn't properly handle creating an obj file from source and obj file
	rm tmp.o

clean:
	$(MAKE) -C netcdf clean
	rm -f *.o *.def.h *.decl.h
