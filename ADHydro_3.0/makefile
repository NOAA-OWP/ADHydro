# Use these values for Mt Moran.

# FIXME

# Use these values for EN3038 lab.

METISDIR         := /opt/lib64
EXTRAFLAGS       := -fdiagnostics-show-option

# Use these values for all.

CHARMC            := $(CHARMDIR)/bin/charmc -language charm++
NETCDFLDFLAGS     := -lnetcdff -lnetcdf -lhdf5_hl -lhdf5 -lz -lcurl
METISLDFLAGS      := -L$(METISDIR) -lmetis -balancer MetisLB
#OPTIMIZATIONFLAGS := -O3 -optimize -production
CPPFLAGS          := $(OPTIMIZATIONFLAGS) $(EXTRAFLAGS) -I../inih/cpp -g -Wall
LDFLAGS           := $(NETCDFLDFLAGS) $(METISLDFLAGS) $(OPTIMIZATIONFLAGS) $(EXTRAFLAGS) -lgfortran -g -Wall

VPATH := ../HRLDAS-v3.6/Noah             \
         ../HRLDAS-v3.6/IO_code          \
         ../HRLDAS-v3.6/Utility_routines \
         ../inih                         \
         ../inih/cpp

EXES := adhydro

ADHYDRO_OBJS := adhydro.o                \
                initialization_manager.o \
                checkpoint_manager.o     \
                file_manager_NetCDF.o    \
                time_point_state.o       \
                region.o                 \
                mesh_element.o           \
                channel_element.o        \
                neighbor_proxy.o         \
                simple_groundwater.o     \
                evapo_transpiration.o    \
                readonly.o

NOAHMP_OBJS := module_sf_noahmpdrv.o          \
               module_sf_noahmplsm.o          \
               module_sf_noahmp_glacier.o     \
               module_sf_noahmp_groundwater.o \
               module_sf_myjsfc.o             \
               module_sf_sfclay.o             \
               module_NoahMP_hrldas_driver.o  \
               module_hrldas_netcdf_io.o      \
               kwm_string_utilities.o         \
               module_date_utilities.o        \
               module_wrf_utilities.o

INIH_OBJS := INIReader.o \
             ini.o

all: $(EXES)

.PHONY: all

adhydro: $(ADHYDRO_OBJS) $(NOAHMP_OBJS) $(INIH_OBJS)
	$(CHARMC) $(LDFLAGS) $^ -o $@

adhydro.o: adhydro.cpp                   \
           adhydro.h                     \
           adhydro.decl.h                \
           adhydro.def.h                 \
           initialization_manager.h      \
           initialization_manager.decl.h \
           checkpoint_manager.h          \
           checkpoint_manager.decl.h     \
           time_point_state.h            \
           region.h                      \
           region.decl.h                 \
           mesh_element.h                \
           channel_element.h             \
           neighbor_proxy.h              \
           simple_groundwater.h          \
           evapo_transpiration.h         \
           readonly.h                    \
           all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

adhydro.decl.h \
adhydro.def.h: adhydro.ci                \
               initialization_manager.ci \
               checkpoint_manager.ci     \
               region.ci
	$(CHARMC) $<

initialization_manager.o: initialization_manager.cpp    \
                          initialization_manager.h      \
                          initialization_manager.decl.h \
                          initialization_manager.def.h  \
                          adhydro.h                     \
                          adhydro.decl.h                \
                          checkpoint_manager.h          \
                          checkpoint_manager.decl.h     \
                          time_point_state.h            \
                          region.h                      \
                          region.decl.h                 \
                          mesh_element.h                \
                          channel_element.h             \
                          neighbor_proxy.h              \
                          simple_groundwater.h          \
                          evapo_transpiration.h         \
                          readonly.h                    \
                          all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

initialization_manager.decl.h \
initialization_manager.def.h: initialization_manager.ci
	$(CHARMC) $<

checkpoint_manager.o: checkpoint_manager.cpp    \
                      checkpoint_manager.h      \
                      checkpoint_manager.decl.h \
                      checkpoint_manager.def.h  \
                      file_manager_NetCDF.h     \
                      time_point_state.h        \
                      mesh_element.h            \
                      channel_element.h         \
                      neighbor_proxy.h          \
                      simple_groundwater.h      \
                      evapo_transpiration.h     \
                      readonly.h                \
                      all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

checkpoint_manager.decl.h \
checkpoint_manager.def.h: checkpoint_manager.ci
	$(CHARMC) $<

file_manager_NetCDF.o: file_manager_NetCDF.cpp \
                       file_manager_NetCDF.h   \
                       time_point_state.h      \
                       mesh_element.h          \
                       channel_element.h       \
                       neighbor_proxy.h        \
                       simple_groundwater.h    \
                       evapo_transpiration.h   \
                       readonly.h              \
                       all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

time_point_state.o: time_point_state.cpp  \
                    time_point_state.h    \
                    mesh_element.h        \
                    channel_element.h     \
                    neighbor_proxy.h      \
                    simple_groundwater.h  \
                    evapo_transpiration.h \
                    all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

region.o: region.cpp                \
          region.h                  \
          region.decl.h             \
          region.def.h              \
          adhydro.h                 \
          adhydro.decl.h            \
          checkpoint_manager.h      \
          checkpoint_manager.decl.h \
          time_point_state.h        \
          mesh_element.h            \
          channel_element.h         \
          neighbor_proxy.h          \
          simple_groundwater.h      \
          evapo_transpiration.h     \
          readonly.h                \
          all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

region.decl.h \
region.def.h: region.ci
	$(CHARMC) $<

mesh_element.o: mesh_element.cpp      \
                mesh_element.h        \
                neighbor_proxy.h      \
                simple_groundwater.h  \
                evapo_transpiration.h \
                readonly.h            \
                all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

channel_element.o: channel_element.cpp   \
                   channel_element.h     \
                   neighbor_proxy.h      \
                   evapo_transpiration.h \
                   readonly.h            \
                   all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

neighbor_proxy.o: neighbor_proxy.cpp \
                  neighbor_proxy.h   \
                  readonly.h         \
                  all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

simple_groundwater.o: simple_groundwater.cpp \
                      simple_groundwater.h   \
                      all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

evapo_transpiration.o: evapo_transpiration.cpp \
                       evapo_transpiration.h   \
                       readonly.h              \
                       all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

readonly.o: readonly.cpp \
            readonly.h   \
            all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

clean:
	rm -f charmrun $(EXES) *.o *.decl.h *.def.h

.PHONY: clean
