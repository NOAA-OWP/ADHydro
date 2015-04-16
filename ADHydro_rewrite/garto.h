#ifndef __GARTO_H__
#define __GARTO_H__

/* A garto_parameters struct stores constant soil parameters for Green-Ampt with Redistribution (GARTO) domain.
 * Note: In GARTO model, the Brooks-Corey soil model is taken by default, because the capillary head across the
 * wetting front between any water content G(theta_1, theta_2) is available. While the analytic equation from
 * van Genutchen is not available, the vg_alpha and vg_n are transformed to bc_lambda and bc_psib.
 */
typedef struct
{
  int    num_bins;                    // The maximum number of bins can be used.
  double theta_r;                     // Residual water content, unitless.
  double theta_s;                     // Saturation water content, unitless.
  double vg_alpha;                    // Van Genutchen parameter in one over meters [1/m].
  double vg_n;                        // Van Genutchen parameter, unitless.
  double bc_lambda;                   // Brook-Corey parameter, unitless.
  double bc_psib;                     // Brook-Corey parameter in meters [m].
  double saturated_conductivity;      // Saturated hydraulic conducivity in meters per second [m/s].
  double effective_capillary_suction; // Effective capillary suction head used in Green-Ampt model, in meters [m].
} garto_parameters;

/* A garto_domain struct stores all of the state of a single GARTO domain.
 * Note: groundwater_front_theta, and groundwater_front_depth are allocated even yes_groundwater is FALSE.
 */
typedef struct
{
  garto_parameters* parameters;              // Constant soil paramters.
  double            top_depth;               // The dpeth of the top of the garto_domain in meters [m].
  double            bottom_depth;            // The dpeth of the bottom of the garto_domain in meters [m].
  double*           d_theta;                 // 1D array of veloctiy of surface wetting front water content change in each bin, unitless.
  double*           surface_front_theta;     // 1D array of surface front water content in each bin, unitless.
  double*           surface_front_depth;     // 1D array of depth of surface water wetting front in each bin, in meters[m].
  double*           groundwater_front_theta; // 1D array of groundwater front water content in each bin, unitless.
  double*           groundwater_front_depth; // 1D array of depth of groundwater wetting front in each bin, in meters[m].
  double            initial_water_content;   // Initial water content, used only if yes_groundwater if FALSE.
  int               yes_groundwater;         // Whether to simulate groundwater.
} garto_domain;

/* Create a garto_parameters struct and initialize it.
 * Return TRUE if there is an error, FALSE otherwise.
 *
 * Parameters:
 *
 * parameters          - A pointer passed by reference which will be assigned to point to the newly allocated struct or NULL if there is an error.
 * num_bins            - The maximum number of bins.  One based indexing is used.
 * conductivity        - Saturation hydraulic conductivity in meters per second [m/s].
 * porosity            - Soil porosity as a unitless fraction.
 * residual saturation - Soil residual saturation as a unitless fraction.
 * van_genutchen       - Flag to indicate if the Van Genutchen parameters are passed.
 *                       If TRUE, vg_alpha and vg_n are converted to Brook-Corey paramters bc_lambda and bc_psib.
 *                       If FALSE, the Brook-Corey function is used instead.
 * vg_alpha            - Van Genutchen parameter in one over meters[1/m].
 * vg_n                - Van Genutchen parameter, unitless.
 * bc_lambda           - Brook-Corey parameter, unitless.
 * bc_psib             - Brook-Corey parameter in meters[m].
 */
int garto_parameters_alloc(garto_parameters** parameters, int num_bins, double conductivity, double porosity, double residual_saturation,
                           int van_genutchen, double vg_alpha, double vg_n, double bc_lambda, double bc_psib);

/* Free memory allocated by garto_parameters_alloc.
 *
 * Parameters:
 *
 * parameters - A pointer to the garto_parameters struct passed by reference. Will be set to NULL after the memory is deallocated.
 */
void garto_parameters_dealloc(garto_parameters** parameters);

/* Create a garto_domain struct and initialize it.
 * Return TRUE if there is an error, FALSE otherwise.
 * If yes_groundwater is TRUE and initialize_to_hydrostatic is TRUE, then the domain is initialized to hydrostatic equilibrium with the given water table.
 * Else, the domain is initialized to have no water other than initial_water_content.
 *
 * Parameters:
 *
 * domain                    - A pointer passed by reference which will be assigned to point to the newly allocated struct or NULL if there is an error.
 * parameters                - A pointer to the garto_parameters struct.
 * layer_top_depth           - The depth of the top    of the garto_domain in meters[m].
 * layer_bottom_depth        - The depth of the bottom of the garto_domain in meters[m].
 * yes_groundwater           - Whether to simulate groundwater.
 * initial_water_content     - Initial water content used when yes_groundwater or initialize_to_hydrostatic is FALSE.
 * initialize_to_hydrostatic - Whether to initialize the groundwater front to hydrostatic equilibrium with the given water table.
 * water_table               - The depth in meters of the water table to use to initialize the groundwater front.
 */
int garto_domain_alloc(garto_domain** domain, garto_parameters* parameters, double layer_top_depth, double layer_bottom_depth, int yes_groundwater,
                       double initial_water_content, int initialize_to_hydrostatic, double water_table);

/* Free memory allocated for the GARTO domain including memory allocated by garto_domain_alloc except memory allocated by garto_parameters_alloc
 * because the garto_parameters struct might be shared.  You need to call garto_parameters_dealloc separately.
 *
 * Parameters:
 *
 * domain - A pointer to the t_o_domain struct passed by reference. Will be set to NULL after the memory is deallocated.
 */
void garto_domain_dealloc(garto_domain** domain);

/* Return the total water in the GARTO domain in meters of water.
 * Parameters:
 *
 * domain - A pointer to the garto_domain struct.
 */
double garto_total_water_in_domain(garto_domain* domain);

/* Step the Green-Ampt with Redistribution simulation forward one timestep.
 * Return TRUE if there is an error, FALSE otherwise.
 *
 * Parameters:
 *
 * domain               - A pointer to the garto_domain struct.
 * dt                   - The duration of the timestep in seconds.
 * surfacewater_depth   - A scalar passed by reference containing the depth in meters of the surface water.
 *                         Will be updated for the amount of infiltration.
 * water_table          - The depth in meters of the water table.
 * groundwater_recharge - A scalar passed by reference containing any previously accumulated groundwater recharge in meters of water.
 *                        Will be updated for the amount of water that flowed between the GARTO domain and groundwater.
 *                        Positive means water flowed down out of the GARTO domain.
 *                        Negative means water flowed up in to the GARTO domain.
 */
int garto_timestep(garto_domain* domain, double dt, double* surfacewater_depth, double water_table, double* groundwater_recharge);

/* Assert if any GARTO domain invariant is violated.
 * Parameters:
 *
 * domain - A pointer to the gator_domain struct.
 */
void garto_check_invariant(garto_domain* domain);

/* Arbitrarily add water to the groundwater front of a GARTO domain.
 * This function is used to couple the domain to a separate groundwater simulation.
 * Parameters:
 *
 * domain               - A pointer to the t_o_domain struct.
 * groundwater_recharge - A scalar passed by reference containing the amount of water to add to the domain in meters of water.
 *                        Will be updated to the amount of water that was not able to be added to the domain.
 *                        If the updated value is epsilon equal to zero all of the water was added successfully.
 *                        If the updated value is not epsilon equal to zero the caller is responsible for putting
 *                        that much water somewhere else to maintain mass conservation.
 *                        If it failed to add all of the water that means the domain is entirely full and you can
 *                        move the water table to the surface and put the rest  of the water in the surface water.    
 */
void garto_add_groundwater(garto_domain* domain, double* groundwater_recharge);

/* Arbitrarily remove water from the groundwater front of a GARTO domain.
 * This function is used to couple the domain to a separate groundwater simulation.
 *
 * garto_take_groundwater takes water starting with the rightmost bin.
 * It lowers the groundwater front as low as 10% of capilary head above the water table.
 * If it can't get enough water from the rightmost bin it proceeds to bins to
 * the left until it can get all of the water or every bin is at 10% of capillary head above the water table.
 * Parameters:
 *
 * domain               - A pointer to the t_o_domain struct.
 * water_table          - The depth in meters of the water table.
 * groundwater_recharge - A scalar passed by reference containing the amount of water to take from the domain in meters of water as a negative number.
 *                        Will be updated to the amount of water that was not able to be taken from the domain.
 */
void garto_take_groundwater(garto_domain* domain, double water_table, double* groundwater_recharge);

/* Return an estimate of the specific yield.
 *
 * Parameters:
 *
 * domain      - A pointer to the t_o_domain struct.
 * water_table - The depth in meters of the water table.
 */
double garto_specific_yield(garto_domain* domain, double water_table);

/* Evapotranspiration process in GARTO domain for one time step. Added 2014/07/17.
 * Return the amount of evapotranspirated water in [meters].
 *
 * Parameters:
 *
 * domain               - A pointer to the garto_domain struct.
 * evaporation_demand   - Potential evaporation demand in meters of water.
 * transpiration_demand - Potential root zone transpiration demand in meters of water.
 * root_depth           - Depth of root zone in [meters].
 *
 * Note, soil evaporation rate can be obtained from NOAHMP output variable ESOIL() in unit of [mm / second], (variable name EDIR() in origin NOAH code).
 *          transpiration rate can be obtained from NOAHMP output variable ETRAN() in unit of [mm / second].
 */
double garto_evapotranspiration(garto_domain* domain, double evaporation_demand, double transpiration_demand, double root_depth);

/* Map gar domain into water content in 1D space discretization, water_content[num_elements] and effective_porosity are output values.
 *
 * Parameters:
 * domain             - A pointer to the garto_domain struct.
 * num_elements       - Number of element in 1d soil column, zero-based.
 * water_content      - A pointer to 1d array of size num_elements contains water content of each element.
 * soil_depth_z       - A pointer to 1d array of size num_elements contains depth of each element's lower bound, in unit of [meters], positive downward.
 */
int get_garto_domain_water_content(garto_domain* domain, double* water_content, double* soil_depth_z, int num_elements);

#endif // __GARTO_H__
