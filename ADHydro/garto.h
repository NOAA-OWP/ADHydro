#ifndef __GARTO_H__
#define __GARTO_H__

#include "all_charm.h"

// Green-Ampt with Redistribution/Talbot-Ogden (GARTO) is an infiltration model
// that calculates how much surfacewater soaks into the ground and the
// resulting moisture content of the soil at all depths.

// A garto_parameters struct stores constant soil parameters for a GARTO
// domain.  In the GARTO model, the Brooks-Corey soil model is used because the
// capillary head across the wetting front between any water content
// G(theta_1, theta_2) is available.  If van Genutchen soil parameters are
// given, vg_alpha and vg_n are transformed to bc_lambda and bc_psib.
typedef struct
{
  int    num_bins;                    // The maximum number of bins that can be used.
  double theta_r;                     // Residual water content, unitless.
  double theta_s;                     // Saturation water content, unitless.
  double vg_alpha;                    // Van Genutchen parameter in one over meters [1/m].
  double vg_n;                        // Van Genutchen parameter, unitless.
  double bc_lambda;                   // Brooks-Corey parameter, unitless.
  double bc_psib;                     // Brooks-Corey parameter in meters [m].  Value must be positive.
  double saturated_conductivity;      // Saturated hydraulic conducivity in meters per second [m/s].
  double effective_capillary_suction; // Effective capillary suction head used in Green-Ampt model in meters [m].
} garto_parameters;

PUPbytes(garto_parameters);

// A garto_domain struct stores all of the state of a single GARTO domain.
// Note: groundwater_front_theta, and groundwater_front_depth are allocated
// even if yes_groundwater is false.
typedef struct
{
  garto_parameters* parameters;              // Constant soil paramters.
  double            top_depth;               // The depth of the top of the garto_domain in meters [m].
  double            bottom_depth;            // The depth of the bottom of the garto_domain in meters [m].
  double*           d_theta;                 // 1D array of velocities of surface front water content change in each bin, unitless.
  double*           surface_front_theta;     // 1D array of surface front water content in each bin, unitless.
  double*           surface_front_depth;     // 1D array of depth of surface front in each bin in meters[m].
  double*           groundwater_front_theta; // 1D array of groundwater front water content in each bin, unitless.
  double*           groundwater_front_depth; // 1D array of depth of groundwater front in each bin in meters[m].
  double            initial_water_content;   // Initial water content.  Used only if yes_groundwater if false.
  int               yes_groundwater;         // Whether to simulate groundwater.  If true, the groundwater state is stored in groundwater_front_theta and
                                             // groundwater_front_depth.  If false, the soil is assumed to be wet up to initial_water_content all the way down,
                                             // and water beyond initial_water_content falls out the bottom of the domain as if there were dry soil below it.
} garto_domain;

inline void operator|(PUP::er &p, garto_domain &gd)
{
  if (p.isUnpacking())
    {
      gd.parameters = new garto_parameters;
    }
  
  p | *gd.parameters;
  
  if (p.isUnpacking())
    {
      gd.d_theta                 = new double[gd.parameters->num_bins + 1];
      gd.surface_front_theta     = new double[gd.parameters->num_bins + 1];
      gd.surface_front_depth     = new double[gd.parameters->num_bins + 1];
      gd.groundwater_front_theta = new double[gd.parameters->num_bins + 1];
      gd.groundwater_front_depth = new double[gd.parameters->num_bins + 1];
    }
  
  p | gd.top_depth;
  p | gd.bottom_depth;
  PUParray(p, gd.d_theta,                 gd.parameters->num_bins + 1);
  PUParray(p, gd.surface_front_theta,     gd.parameters->num_bins + 1);
  PUParray(p, gd.surface_front_depth,     gd.parameters->num_bins + 1);
  PUParray(p, gd.groundwater_front_theta, gd.parameters->num_bins + 1);
  PUParray(p, gd.groundwater_front_depth, gd.parameters->num_bins + 1);
  p | gd.initial_water_content;
  p | gd.yes_groundwater;
}

// Create a garto_parameters struct and initialize it.
//
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// parameters          - A pointer passed by reference which will be assigned
//                       to point to the newly allocated struct or NULL if
//                       there is an error.
// num_bins            - The maximum number of bins.  One based indexing is
//                       used.
// conductivity        - Saturation hydraulic conductivity in meters per second
//                       [m/s].
// porosity            - Soil porosity, unitless.
// residual_saturation - Soil residual saturation, unitless.
// van_genutchen       - Flag to indicate if the Van Genutchen parameters are
//                       passed.  If true, vg_alpha and vg_n are converted to
//                       Brooks-Corey paramters bc_lambda and bc_psib.  If
//                       false, bc_lambda and bc_psib are used directly.
// vg_alpha            - Van Genutchen parameter in one over meters [1/m].
// vg_n                - Van Genutchen parameter, unitless.
// bc_lambda           - Brooks-Corey parameter, unitless.
// bc_psib             - Brooks-Corey parameter in meters [m].  Value must be
//                       positive.
int garto_parameters_alloc(garto_parameters** parameters, int num_bins, double conductivity, double porosity, double residual_saturation, int van_genutchen,
                           double vg_alpha, double vg_n, double bc_lambda, double bc_psib);

// Free memory allocated by garto_parameters_alloc.
//
// Parameters:
//
// parameters - A pointer to the garto_parameters struct passed by reference.
//              Will be set to NULL after the memory is deallocated.
void garto_parameters_dealloc(garto_parameters** parameters);

// Create a garto_domain struct and initialize it.  If yes_groundwater is true
// and initialize_to_hydrostatic is true, then the domain is initialized to
// hydrostatic equilibrium with the given value of water_table and
// initial_water_content is ignored.  Otherwise, the domain is initialized to
// have no water other than initial_water_content and water_table is ignored.
//
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// domain                    - A pointer passed by reference which will be
//                             assigned to point to the newly allocated struct
//                             or NULL if there is an error.
// parameters                - A pointer to the garto_parameters struct to use.
// layer_top_depth           - The depth of the top of the garto_domain in
//                             meters [m].
// layer_bottom_depth        - The depth of the bottom of the garto_domain in
//                             meters [m].
// yes_groundwater           - Whether to simulate groundwater.
// initial_water_content     - Initial water content used when either
//                             yes_groundwater or initialize_to_hydrostatic are
//                             false.
// initialize_to_hydrostatic - Whether to initialize the groundwater front to
//                             hydrostatic equilibrium with the given water
//                             table.
// water_table               - The depth in meters of the water table to use to
//                             initialize the groundwater front when
//                             yes_groundwater and initialize_to_hydrostatic
//                             are both true.
int garto_domain_alloc(garto_domain** domain, garto_parameters* parameters, double layer_top_depth, double layer_bottom_depth, int yes_groundwater,
                       double initial_water_content, int initialize_to_hydrostatic, double water_table);

// Free memory allocated by garto_domain_alloc.  Does not free memory allocated
// by garto_parameters_alloc because the garto_parameters struct might be
// shared.  You need to call garto_parameters_dealloc separately.
//
// Parameters:
//
// domain - A pointer to the garto_domain struct passed by reference. Will be
//          set to NULL after the memory is deallocated.
void garto_domain_dealloc(garto_domain** domain);

// Assert if any GARTO domain invariant is violated.
//
// Parameters:
//
// domain - A pointer to the gator_domain struct.
void garto_check_invariant(garto_domain* domain);

// Returns: the total water in the GARTO domain in meters of water.
//
// Parameters:
//
// domain - A pointer to the garto_domain struct.
double garto_total_water_in_domain(garto_domain* domain);

// Step the GARTO simulation forward one timestep.
//
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// domain               - A pointer to the garto_domain struct.
// dt                   - The duration of the timestep in seconds.
// surfacewater_depth   - A scalar passed by reference containing the depth in
//                        meters of the surface water.  Will be updated to the
//                        amount remaining after infiltration.
// water_table          - The depth in meters of the water table.
// groundwater_recharge - A scalar passed by reference containing any
//                        previously accumulated groundwater recharge in meters
//                        of water.  Will be updated for the amount of water
//                        that flowed between the GARTO domain and groundwater.
//                        Positive means water flowed down out of the GARTO domain.
//                        Negative means water flowed up in to the GARTO domain.
int garto_timestep(garto_domain* domain, double dt, double* surfacewater_depth, double water_table, double* groundwater_recharge);

// Arbitrarily add water to the groundwater front of a GARTO domain.  This
// function is used to couple the domain to a separate groundwater simulation.
//
// FIXLATER better description of how and why this water is added.
//
// Parameters:
//
// domain               - A pointer to the garto_domain struct.
// groundwater_recharge - A scalar passed by reference containing the amount of
//                        water to add to the domain in meters of water.  Will
//                        be updated to the amount of water that was not able
//                        to be added to the domain.  If the updated value is
//                        epsilon equal to zero the water was added
//                        successfully and the caller should keep any non-zero
//                        value in groundwater_recharge for next timestep to
//                        maintain mass conservation.  If the updated value is
//                        not epsilon equal to zero there was not enough space
//                        for the water.  This means the domain is entirely
//                        full, and the caller should put the rest of the water
//                        in the surface water and move the water table depth
//                        to the surface.
void garto_add_groundwater(garto_domain* domain, double* groundwater_recharge);

// Arbitrarily remove water from the groundwater front of a GARTO domain.  This
// function is used to couple the domain to a separate groundwater simulation.
//
// garto_take_groundwater takes water starting with the rightmost bin.  It
// lowers the groundwater front as low as 10% of capilary head above the water
// table.  If it can't get enough water from the rightmost bin it proceeds to
// bins to the left until it can get all of the water or every bin is at 10% of
// capillary head above the water table.
//
// FIXLATER better description of why this water is taken.
//
// Parameters:
//
// domain               - A pointer to the garto_domain struct.
// water_table          - The depth in meters of the water table.
// groundwater_recharge - A scalar passed by reference containing the amount of
//                        water to take from the domain in meters of water as a
//                        negative number.  Will be updated to the amount of
//                        water that was not able to be taken from the domain,
//                        again as a negative number.  The caller should keep
//                        any non-zero value in groundwater_recharge for next
//                        timestep to maintain mass conservation.
void garto_take_groundwater(garto_domain* domain, double water_table, double* groundwater_recharge);

// Returns: an estimate of the specific yield.
//
// Parameters:
//
// domain      - A pointer to the garto_domain struct.
// water_table - The depth in meters of the water table.
double garto_specific_yield(garto_domain* domain, double water_table);

// Remove evapotranspiration water from the GARTO domain.
//
// Returns: the amount of evapotranspirated water in meters of water.
//
// Parameters:
//
// domain               - A pointer to the garto_domain struct.
// evaporation_demand   - Surface evaporation demand in meters of water.
// transpiration_demand - Root zone transpiration demand in meters of water.
// root_depth           - Depth of root zone in meters.
double garto_evapotranspiration(garto_domain* domain, double evaporation_demand, double transpiration_demand, double root_depth);

// Map the GARTO domain water content to a different 1D spatial discretization.
// This function is used to couple the domain to a separate evapotranspiration
// simulation.
//
// Parameters:
//
// domain        - A pointer to the garto_domain struct.
// water_content - 1D array of size num_elements.  water_content[jj] will be
//                 filled in with the average water content between
//                 soil_depth_z[jj - 1] and soil_depth_z[jj].  Both
//                 water_content and soil_depth_z are zero-based.
//                 water_content[0] will be filled in with the average water
//                 content between 0 and soil_depth_z[0].
// soil_depth_z  - 1D array of size num_elements containing the depth of the
//                 bottom of each soil layer in meters, positive downward.
// num_elements  - Number of layers in the different 1D spatial discretization
//                 represented by soil_depth_z.
void garto_domain_water_content(garto_domain* domain, double* water_content, double* soil_depth_z, int num_elements);

// Get the equilibrium water content using the Brooks-Corey retention curve for
// the given depth above the water table.
//
// Parameters:
//
// domain                  - A pointer to the garto_domain struct.
// depth_above_water_table - Depth above water table in meters.
double garto_equilibrium_water_content(garto_domain* domain, double depth_above_water_table);

#endif // __GARTO_H__
