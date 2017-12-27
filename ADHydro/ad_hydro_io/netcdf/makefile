.PHONY: all netcdf

NETCDFOBJS := adhydro_netcdf.o

all: netcdf

netcdf: $(NETCDFOBJS)

adhydro_netcdf.o: netcdf_io_ops.cpp netcdf_io_ops.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

clean:
	rm -f *.o *.def.h *.decl.h
