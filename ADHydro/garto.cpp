#include "garto.h"

int garto_parameters_alloc(garto_parameters** parameters, int num_bins, double conductivity, double porosity, double residual_saturation, int van_genutchen,
                           double vg_alpha, double vg_n, double bc_lambda, double bc_psib)
{
  int    error = false; // Error flag.
  double m, p;          // Derived parameters.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != parameters))
    {
      CkError("ERROR in garto_parameters_alloc: parameters must not be NULL.\n");
      error = true;
    }
  else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      *parameters = NULL; // Prevent deallocating a random pointer.
    }

#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 < num_bins))
    {
      CkError("ERROR in garto_parameters_alloc: num_bins must be greater than zero.\n");
      error = true;
    }

  if (!(0.0 < conductivity))
    {
      CkError("ERROR in garto_parameters_alloc: conductivity must be greater than zero.\n");
      error = true;
    }

  if (!(0.0 < residual_saturation))
    {
      CkError("ERROR in garto_parameters_alloc: residual_saturation must be greater than zero.\n");
      error = true;
    }

  if (!(residual_saturation < porosity))
    {
      CkError("ERROR in garto_parameters_alloc: porosity must be greater than residual_saturation.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (van_genutchen)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
      if (!(1.0 < vg_n))
        {
          CkError("ERROR in garto_parameters_alloc: vg_n must be greater than one.\n");
          error = true;
        }
      
      if (!(0.0 < vg_alpha))
        {
          CkError("ERROR in garto_parameters_alloc: vg_alpha must be greater than zero.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
      
      // If given Van Genutchen parameters calculate Brooks-Corey parameters.
      if (!error)
        {
          m         = 1.0 - 1.0 / vg_n;
          p         = 1.0 + 2.0 / m;
          bc_lambda = 2.0 / (p - 3.0);
          bc_psib   = (p + 3.0) * (147.8 + 8.1 * p + 0.092 * p * p) / (2.0 * vg_alpha * p * (p - 1.0) * (55.6 + 7.4 * p + p * p));
        }
    }
  else
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
      // FIXME below we are dividing by bc_lambda.  That means at the very least there should be a requirement that it not be zero, but I don't know what the
      // range of valid values for bc_lambda is.  One of the hydrologists on the team needs to answer this for me.
      if (!(0.0 != bc_lambda))
        {
          CkError("ERROR in garto_parameters_alloc: bc_lambda must not be zero.\n");
          error = true;
        }
      
      if (!(0.0 < bc_psib))
        {
          CkError("ERROR in garto_parameters_alloc: bc_psib must be greater than zero.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
      
      // If given Brooks-Corey parameters calculate Van Genutchen parameters.
      if (!error)
        {
          p        = 3.0 + 2.0 / bc_lambda;
          m        = 2.0 / (p - 1.0);
          vg_alpha = (p + 3.0) * (147.8 + 8.1 * p + 0.092 * p * p) / (2.0 * bc_psib * p * (p - 1.0) * (55.6 + 7.4 * p + p * p));
          vg_n     = 1.0 / (1.0 - m);
        }
    }

  // Allocate and initialize the garto_parameters struct.
  if (!error)
    {
      *parameters = new garto_parameters;
      
      (*parameters)->num_bins                = num_bins;
      (*parameters)->theta_r                 = residual_saturation;
      (*parameters)->theta_s                 = porosity;
      (*parameters)->vg_alpha                = vg_alpha;
      (*parameters)->vg_n                    = vg_n;
      (*parameters)->bc_lambda               = bc_lambda;
      (*parameters)->bc_psib                 = bc_psib;
      (*parameters)->saturated_conductivity  = conductivity;
      
      if (van_genutchen)
        {
          (*parameters)->effective_capillary_suction = (1.0 / vg_alpha) * (0.046 * m + 2.07 * m * m + 19.5 * m * m * m) / (1 + 4.7 * m + 16.0 * m * m);
        }
      else
        {
          (*parameters)->effective_capillary_suction = bc_psib * (2.0 + 3.0 * bc_lambda) / (1.0 + 3.0 * bc_lambda);
        }
    }

  if (error)
    {
      garto_parameters_dealloc(parameters);
    }

  return error;
}

void garto_parameters_dealloc(garto_parameters** parameters)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  CkAssert(NULL != parameters);
#endif

  if (NULL != parameters && NULL != *parameters)
    {
      // Deallocate the garto_parameters struct.
      delete *parameters;
      
      *parameters = NULL;
    }
}

int garto_domain_alloc(garto_domain** domain, garto_parameters* parameters, double layer_top_depth, double layer_bottom_depth, int yes_groundwater,
                       double initial_water_content, int initialize_to_hydrostatic, double water_table)
{
  int    error = false;       // Error flag.
  int    ii;                  // Loop counter.
  double delta_water_content; 
  double relative_saturation;
  double suction_head;
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != domain))
    {
      CkError("ERROR in garto_domain_alloc: domain must not be NULL.\n");
      error = true;
    }
  else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      *domain = NULL; // Prevent deallocating a random pointer.
    }

#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != parameters))
    {
      CkError("ERROR in garto_domain_alloc: parameters must not be NULL.\n");
      error = true;
    }

  if (!(0.0 <= layer_top_depth))
    {
      CkError("ERROR in garto_domain_alloc: layer_top_depth must be greater than or equal to zero.\n");
      error = true;
    }
    
  if (!(layer_top_depth < layer_bottom_depth))
    {
      CkError("ERROR in garto_domain_alloc: layer_bottom_depth must be greater than layer_top_depth.\n");
      error = true;
    }

  if (yes_groundwater && initialize_to_hydrostatic)
    {
      if (!(0.0 <= water_table))
        {
          CkError("ERROR in garto_domain_alloc: if yes_groundwater and initialize_to_hydrostatic are true water_table must be greater than or equal to zero.\n");
          error = true;
        }
    }
  else
    {
      if (!(0.0 < initial_water_content))
        {
          CkError("ERROR in garto_domain_alloc: if yes_groundwater or initialize_to_hydrostatic are false initial_water_content must be greater than zero.\n");
          error = true;
        }
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      // Allocate and initialize the garto_domain struct.
      *domain = new garto_domain;

      (*domain)->parameters              = parameters;
      (*domain)->top_depth               = layer_top_depth;
      (*domain)->bottom_depth            = layer_bottom_depth;
      (*domain)->d_theta                 = NULL;
      (*domain)->surface_front_theta     = NULL;
      (*domain)->surface_front_depth     = NULL;
      (*domain)->groundwater_front_theta = NULL;
      (*domain)->groundwater_front_depth = NULL;
      (*domain)->yes_groundwater         = yes_groundwater;

      if (yes_groundwater && initialize_to_hydrostatic)
        {
          // Calculate surface initial_water_content using water_table.
          if ((water_table - (*domain)->top_depth) > parameters->bc_psib)
            {
              initial_water_content = parameters->theta_r + (parameters->theta_s - parameters->theta_r) *
                  pow((water_table - (*domain)->top_depth) / parameters->bc_psib, -parameters->bc_lambda);
            }
          else
            {
              initial_water_content = parameters->theta_s;
            }
        }

      if (initial_water_content >= parameters->theta_r)
        {
          (*domain)->initial_water_content = initial_water_content;
        }
      else
        {
          CkError("WARNING in garto_domain_alloc: initial_water_content less than the residual water content, using the residual water content instead.\n");

          (*domain)->initial_water_content = parameters->theta_r;
        }

      // Allocate and initialize d_theta.
      (*domain)->d_theta = new double[parameters->num_bins + 1];

      for (ii = 1; ii <= parameters->num_bins; ii++)
        {
          (*domain)->d_theta[ii] = 0.0;
        }

      // Allocate and initialize surface_front_theta.
      (*domain)->surface_front_theta = new double[parameters->num_bins + 1];

      // (*domain)->surface_front_theta[0] will be used in the simulation.
      for (ii = 0; ii <= parameters->num_bins; ii++)
        {
          (*domain)->surface_front_theta[ii] = (*domain)->initial_water_content;
        }

      // Allocate and initialize surface_front_depth.
      (*domain)->surface_front_depth = new double[parameters->num_bins + 1];

      // (*domain)->surface_front_depth[0] will be used in the simulation.
      (*domain)->surface_front_depth[0] = layer_bottom_depth;

      for (ii = 1; ii <= parameters->num_bins; ii++)
        {
          (*domain)->surface_front_depth[ii] = layer_top_depth;
        }

      // Allocate and initialize groundwater_front_theta.
      (*domain)->groundwater_front_theta = new double[parameters->num_bins + 1];
      delta_water_content                = (parameters->theta_s - (*domain)->initial_water_content) / parameters->num_bins;

      // (*domain)->groundwater_front_theta[0] will be used in the simulation.
      for (ii = 0; ii < parameters->num_bins; ii++)
        {
          (*domain)->groundwater_front_theta[ii] = (*domain)->initial_water_content + ii * delta_water_content;
        }

      // The last groundwater front bin should be exactly theta_s.
      (*domain)->groundwater_front_theta[parameters->num_bins] = parameters->theta_s;

      // Allocate and initialize groundwater_front_depth.
      (*domain)->groundwater_front_depth = new double[parameters->num_bins + 1];
      
      if (yes_groundwater && initialize_to_hydrostatic)
        {
          // Initialize groundwater_front_depth to hydrostatic.
          for (ii = 1; ii <= parameters->num_bins; ii++)
            {
              relative_saturation = ((*domain)->groundwater_front_theta[ii] - parameters->theta_r) / (parameters->theta_s - parameters->theta_r);
              suction_head        = parameters->bc_psib * pow(relative_saturation, -1.0 / parameters->bc_lambda);

              (*domain)->groundwater_front_depth[ii] = water_table - suction_head;

              if ((*domain)->groundwater_front_depth[ii] < layer_top_depth)
                {
                  (*domain)->groundwater_front_depth[ii] = layer_top_depth;
                }
              else if ((*domain)->groundwater_front_depth[ii] > layer_bottom_depth)
                {
                  (*domain)->groundwater_front_depth[ii] = layer_bottom_depth;
                }
            }
        }
      else
        {
          // Initialize groundwater_front_depth to empty.
          for (ii = 1; ii <= parameters->num_bins; ii++)
            {
              (*domain)->groundwater_front_depth[ii] = layer_bottom_depth;
            }
        }
    }

  if (error)
    {
      garto_domain_dealloc(domain);
    }

  return error;
}

void garto_domain_dealloc(garto_domain** domain)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  CkAssert(NULL != domain);
#endif

  if (NULL != domain && NULL != *domain)
    {
      delete[] (*domain)->d_theta;
      delete[] (*domain)->surface_front_theta;
      delete[] (*domain)->surface_front_depth;
      delete[] (*domain)->groundwater_front_theta;
      delete[] (*domain)->groundwater_front_depth;
      delete *domain;

      *domain = NULL;
    }
}

void garto_check_invariant(garto_domain* domain)
{
  int ii; // Loop counter.

  CkAssert(NULL != domain);
  
  if (NULL != domain)
    {
      for (ii = 1; ii <= domain->parameters->num_bins; ii++)
         {
           if (epsilonGreater(domain->surface_front_theta[ii], domain->surface_front_theta[0]))
             {
               CkAssert(domain->surface_front_depth[ii] >= domain->top_depth); 
               
               if (domain->yes_groundwater)
                 {
                   CkAssert(domain->surface_front_depth[ii] <= domain->groundwater_front_depth[1]);
                 } 
               else
                 {
                   CkAssert(domain->surface_front_depth[ii] <= domain->bottom_depth); 
                 }
               
               if (ii >= 2)
                 {
                   CkAssert(domain->surface_front_depth[ii - 1] >= domain->surface_front_depth[ii]); 
                 }
             }
         } 
         
      if (domain->yes_groundwater)
        {
          CkAssert(epsilonEqual(domain->surface_front_theta[0], domain->groundwater_front_theta[0]));
          for (ii = 1; ii <= domain->parameters->num_bins && domain->groundwater_front_theta[ii] > domain->groundwater_front_theta[0]; ii++)
             {
               CkAssert(domain->groundwater_front_depth[ii] <= domain->bottom_depth);
               if (ii >= 2)
                 {
                   CkAssert(domain->groundwater_front_depth[ii - 1] <= domain->groundwater_front_depth[ii]); 
                 } 
             }

          // Scan from right to left for the last bin that exists.
          for (ii = domain->parameters->num_bins; ii > 0 && domain->groundwater_front_theta[ii] <= domain->groundwater_front_theta[0]; ii--)
            {
              // no-op
            }

          CkAssert(domain->groundwater_front_theta[ii] == domain->parameters->theta_s);
        }
    }
}

double garto_total_water_in_domain(garto_domain* domain)
{
  int    ii;
  double water = 0.0;

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)  
  if (domain->yes_groundwater)
    {
      CkAssert(epsilonEqual(domain->surface_front_theta[0], domain->groundwater_front_theta[0]));
    }
  
  CkAssert(NULL != domain);
#endif

  if (NULL != domain)
    {
      // Add amount of water in fully saturated. 
      water += (domain->bottom_depth - domain->top_depth) * domain->surface_front_theta[0];
      
      // Water in surface_front and groundwater_front.
      for (ii = 1; ii <= domain->parameters->num_bins; ii++)
         {
           if (domain->surface_front_theta[ii] > domain->surface_front_theta[0])
             {
               water += (domain->surface_front_depth[ii] - domain->top_depth) * (domain->surface_front_theta[ii] - domain->surface_front_theta[ii - 1]);
             }
             
           if (domain->yes_groundwater)
             {
               if (domain->groundwater_front_theta[ii] > domain->groundwater_front_theta[0])
                 {
                   water += (domain->bottom_depth - domain->groundwater_front_depth[ii]) * 
                            (domain->groundwater_front_theta[ii] - domain->groundwater_front_theta[ii - 1]);
                 }
             }
             
         }
    }
  
  return water;
}

/* Limit infiltration_capacity due to hitting groundwater or domain bottom.
 * Parameters:
 * domain                - A pointer to the garto_domain struct.
 * dt                    - Time step used in seconds[s]. 
 * bin                   - Bin number.
 * infiltration_capacity - A scalar passed by reference containing the infiltration_capaicty for current bin with time step dt [m/s]. 
 */
int garto_limit_infiltration_capacity(garto_domain* domain, double dt, int bin, double* infiltration_capacity)
{
  int    error       = false;
  double maximum_gap = 0.0;
  double water_space = 0.0;
  // FIXME later, currently assume surface_front_depth[bin] cant not overshoot domain->bottom_depth.
  
  maximum_gap = domain->bottom_depth - domain->surface_front_depth[bin];
  water_space = maximum_gap * (domain->surface_front_theta[bin] - domain->surface_front_theta[bin - 1]);
  
  if (domain->d_theta[bin] > 0.0)
    {
      water_space += domain->d_theta[bin] * maximum_gap;
    }
    
  if (epsilonGreater(maximum_gap, 0.0) && *infiltration_capacity * dt > water_space)
    {
      *infiltration_capacity = water_space / dt; 
    }
  else if (epsilonEqual(0.0, maximum_gap))
    {
      *infiltration_capacity = 0.0;
    }
  
  return error;
}

/* If surface water front depth of current bin is deeper than its leftside bin, merge these two fronts into one. 
 * Parameters:
 * domain                - A pointer to the garto_domain struct.
 */
int garto_check_surface_front_overshoot(garto_domain* domain)
{
  int    ii, jj;
  int    error       = false;
  double total_water = 0.0;
  
  for (jj = domain->parameters->num_bins; jj >= 2; jj--)
     {
       if (domain->surface_front_depth[jj] >= domain->surface_front_depth[jj - 1] &&
           domain->surface_front_theta[jj] >  domain->surface_front_theta[jj - 1] )
         {
           total_water = (domain->surface_front_theta[jj]     - domain->surface_front_theta[jj - 1]) * 
                         (domain->surface_front_depth[jj]     - domain->top_depth) +
                         (domain->surface_front_theta[jj - 1] - domain->surface_front_theta[jj - 2]) * 
                         (domain->surface_front_depth[jj - 1] - domain->top_depth);
           domain->surface_front_depth[jj - 1] = domain->top_depth + total_water / (domain->surface_front_theta[jj] - domain->surface_front_theta[jj - 2]);
           domain->surface_front_theta[jj - 1] = domain->surface_front_theta[jj];
           domain->surface_front_depth[jj]     = domain->top_depth;
           domain->surface_front_theta[jj]     = domain->surface_front_theta[0];
           domain->d_theta[jj - 1]             = 0.0;
           domain->d_theta[jj]                 = 0.0;
      
           // After mering [jj] and [jj - 1] to [jj - 1], it might need to shift fronts [>= jj + 1] to left.
           for (ii = jj + 1; ii <= domain->parameters->num_bins; ii ++)
              {
                if (domain->surface_front_theta[ii] > domain->surface_front_theta[0])
                  { // Shift to the left.
                    domain->surface_front_theta[ii - 1] = domain->surface_front_theta[ii];
                    domain->surface_front_depth[ii - 1] = domain->surface_front_depth[ii]; 
                    domain->d_theta[ii - 1]             = domain->d_theta[ii];
                    domain->surface_front_theta[ii]     = domain->surface_front_theta[0];
                    domain->surface_front_depth[ii]     = domain->top_depth;
                    domain->d_theta[ii]                 = 0.0;
                  }
              }
           
            jj++;   
         } 
     }
  
  return error;
}

/* Find dry depth using Green-Ampt equation with backward finite difference in time.
 * Parameters:
 * domain                - A pointer to the garto_domain struct.
 * dt                    - Time step in seconds[s].
 */
double garto_find_dry_depth(garto_domain* domain, double dt)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  CkAssert(domain->parameters->theta_s > domain->surface_front_theta[0]);
#endif
  
  double tau       = dt * domain->parameters->saturated_conductivity / (domain->parameters->theta_s - domain->surface_front_theta[0]);
  double dry_depth = 0.5 * (tau + sqrt(tau * tau + 4.0 * tau * domain->parameters->effective_capillary_suction));
  
  return dry_depth;
}

/* Calculate conductiviy using Brooks-Corey equaiton, given water content.
 * K = Ks * pow ( Se, 3 + 2 / lambda), Se = (water_content - theta_r) / (theta_s - theta_r).
 * Parameters:
 * domain                - A pointer to the garto_domain struct.
 * water_content         - Water_content, unitless.
 */
double conductivity_brooks_corey(garto_domain* domain, double water_content)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  CkAssert((water_content >= domain->parameters->theta_r && water_content <= domain->parameters->theta_s) ||
         epsilonEqual(water_content, domain->parameters->theta_s)  || epsilonEqual(water_content, domain->parameters->theta_r));
#endif
 
  double relative_water_content = (water_content - domain->parameters->theta_r) / (domain->parameters->theta_s - domain->parameters->theta_r);
  double conductivity           = domain->parameters->saturated_conductivity * pow(relative_water_content, 3.0 + 2.0 / domain->parameters->bc_lambda);
  
  return conductivity;
}

/* Calculate pressure head using Brooks-Corey equaiton, given water content.
 * h = psi_b * (Se)^(-1 / lambda), Se = (water_content - theta_r) / (theta_s - theta_r).
 * Parameters:
 * domain                - A pointer to the garto_domain struct.
 * water_content         - Water_content, unitless.
 */
double pressure_head_brooks_corey(garto_domain* domain, double water_content)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  CkAssert((water_content >= domain->parameters->theta_r && water_content <= domain->parameters->theta_s) ||
          epsilonEqual(water_content, domain->parameters->theta_s) || epsilonEqual(water_content, domain->parameters->theta_r));
#endif

  double relative_water_content = (water_content - domain->parameters->theta_r) / (domain->parameters->theta_s - domain->parameters->theta_r);
  double pressure_head          = domain->parameters->bc_psib * pow(relative_water_content, -1.0 /  domain->parameters->bc_lambda);
  
  return pressure_head;
}

/* Return value of capillary drive between water_content_1 and water_content_2 using Brooks-Corey function.
 * Parameters:
 * domain                - A pointer to the garto_domain struct.
 * water_content_1(_2)   - Water_content at the left (right), unitless. 
*/
double capillary_drive_brooks_corey(garto_domain* domain, double water_content_1, double water_content_2)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  CkAssert((domain->parameters->theta_r <= water_content_1 && water_content_1 < water_content_2 && water_content_2 <= domain->parameters->theta_s) ||
          epsilonEqual(water_content_2, domain->parameters->theta_s) || epsilonEqual(water_content_1, domain->parameters->theta_r));
#endif
  
  double capillary_drive;
  double relative_water_content_1 = (water_content_1 - domain->parameters->theta_r) / (domain->parameters->theta_s - domain->parameters->theta_r);
  double relative_water_content_2 = (water_content_2 - domain->parameters->theta_r) / (domain->parameters->theta_s - domain->parameters->theta_r);
  
  if (epsilonLess(water_content_2, domain->parameters->theta_s))
    {
      capillary_drive = domain->parameters->bc_psib * (pow(relative_water_content_2, 3.0 + 1.0 / domain->parameters->bc_lambda) - 
                        pow(relative_water_content_1, 3.0 + 1.0 / domain->parameters->bc_lambda)) / (3.0 * domain->parameters->bc_lambda + 1.0);
    }
  else
    {
      capillary_drive = domain->parameters->bc_psib *(3.0 * domain->parameters->bc_lambda + 2.0 - 
                        pow(relative_water_content_1, 3.0 + 1.0 / domain->parameters->bc_lambda)) / (3.0 * domain->parameters->bc_lambda + 1.0);
    }
  
  return capillary_drive;
}


/* Create a bin to the right of current bin, in case of current bin is shrinking and a rainfall pulse with intensity greater than saturatd conductivity.
 * Parameters:
 * domain                - A pointer to the garto_domain struct.
 * dt                    - Time step used in seconds[s]. 
 * surfacewater_depth    - A scalar passed by reference, containing surface water ponded depth in meters[m].
 * bin                   - Bin number.
 */
int garto_create_right_bin(garto_domain* domain, double dt, double* surfacewater_depth, int bin)
{
  int    error                 = false;
  double dry_depth             = 0.0;  
  double conductivity          = 0.0;
  double infiltration_capacity = 0.0;
  
  if (domain->surface_front_theta[bin] >= domain->parameters->theta_s)
    {
      error = true;
    }
 
  dry_depth    = garto_find_dry_depth(domain, dt);  
  conductivity = conductivity_brooks_corey(domain, domain->surface_front_theta[bin]);
  
  // Method 1, d_theta calculated using GAR equation.
  domain->d_theta[bin + 1] = dt * (*surfacewater_depth / dt - 2.0 * conductivity) / dry_depth;
  if (domain->d_theta[bin + 1] < 0.0)
    { // If d_theta < 0.0 when creating new front, set d_theta = theta_s - theta.
      domain->d_theta[bin + 1] = domain->parameters->theta_s - domain->surface_front_theta[bin]; 
    }
    
  // UNUSED
  // Method 2, the newly created front is saturated, d_theta = theta_s - theta[bin].
  // domain->d_theta[bin + 1] = domain->parameters->theta_s - domain->surface_front_theta[bin];
  
  // Create bin only when the newly calculated d_theta[bin + 1] is positive.
  if (epsilonGreater(domain->d_theta[bin + 1], 0.0))
    {
      // Calculate theta[bin + 1];
      if (domain->surface_front_theta[bin] + domain->d_theta[bin + 1] <= domain->parameters->theta_s)
        {
          domain->surface_front_theta[bin + 1] = domain->surface_front_theta[bin] + domain->d_theta[bin + 1];
        }
      else
        {
          domain->surface_front_theta[bin + 1] = domain->parameters->theta_s;
        }
      
      // Calculate front_depth[bin + 1].
      infiltration_capacity = domain->parameters->saturated_conductivity * (1.0 + domain->parameters->effective_capillary_suction / dry_depth);
      
      // Limit the infiltrate distance as it hit groundwater or domain bottom.
      garto_limit_infiltration_capacity(domain, dt, bin, &infiltration_capacity);
      
      if (*surfacewater_depth > infiltration_capacity * dt)
        {
          domain->surface_front_depth[bin + 1] = domain->top_depth + 
                                                 infiltration_capacity * dt / (domain->surface_front_theta[bin + 1] - domain->surface_front_theta[bin]);
          *surfacewater_depth                 -= infiltration_capacity * dt;
        }
      else
        {
          domain->surface_front_depth[bin + 1] = domain->top_depth + 
                                                 *surfacewater_depth / (domain->surface_front_theta[bin + 1] - domain->surface_front_theta[bin]);
          *surfacewater_depth                  = 0.0;
        }               
    } // End of (domain->d_theta[bin + 1] > 0.0) .
   
  return error;
}

/* Find last bin has water in surface front.
 * Parameters:
 * domain                - A pointer to the garto_domain struct.
 */
int find_surface_last_bin(garto_domain* domain)
{
  int ii, last_bin = 0;
  
  for (ii = domain->parameters->num_bins; ii >= 1; ii--)
     {
       if (epsilonGreater(domain->surface_front_theta[ii], domain->surface_front_theta[0]))
         {
           last_bin = ii;
           break;
         }
     }
  
  return last_bin;
}

/* Find last bin has water in groundwater front.  Will return 0 if the domain
 * is completely full and there are no used groundwater bins.
 * Parameters:
 * domain                - A pointer to the garto_domain struct.
 */
int find_groundwater_last_bin(garto_domain* domain)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  CkAssert(domain->yes_groundwater);
#endif  
  int ii, last_bin = 0;
  
  for (ii = domain->parameters->num_bins; ii >= 1; ii--)
     {
       if (domain->groundwater_front_theta[ii] > domain->groundwater_front_theta[0]  &&
           epsilonEqual(domain->groundwater_front_theta[ii], domain->parameters->theta_s))
         {
           last_bin = ii;
           break;
         }
     }
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE) 
  CkAssert(last_bin >= 0);
#endif
  return last_bin;
}

/* Advance wetting front in a bin (either has right bin or saturated) using Talbot-Ogden equation.
 * Parameters:
 * domain                - A pointer to the garto_domain struct.
 * dt                    - Time step used in seconds[s]. 
 * surfacewater_depth    - A scalar passed by reference, containing surface water ponded depth in meters[m].
 * bin                   - Bin number.
 */
int t_o_advance_front(garto_domain* domain, double dt, double* surfacewater_depth, int bin, double surfacewater_depth_old)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE) 
  CkAssert((domain->surface_front_theta[bin + 1] > domain->surface_front_theta[bin]) || 
         epsilonEqual(domain->surface_front_theta[bin], domain->parameters->theta_s));
#endif 
  int    error                = false;  
  int    last_bin             = find_surface_last_bin(domain);
  double infiltrate_distance  = 0.0;
  double conductivity_1       = 0.0;
  double conductivity_2       = 0.0;
  double capillary_drive      = 0.0;
  double water_demand         = 0.0;
  double water_avail          = 0.0;
  double dry_depth            = garto_find_dry_depth(domain, dt);
  double depth_use            = (domain->surface_front_depth[bin] - domain->top_depth);
  
  if (depth_use < dry_depth)
    {
      infiltrate_distance = dry_depth;
    }
  else
    { 
      // T-O infiltration. Original T-O equation.
      /*
      conductivity_1      = conductivity_brooks_corey(domain, domain->surface_front_theta[0]);
      conductivity_2      = conductivity_brooks_corey(domain, domain->surface_front_theta[last_bin]); 
      capillary_drive     = capillary_drive_brooks_corey(domain, domain->surface_front_theta[0], domain->surface_front_theta[last_bin]);
      infiltrate_distance = dt * (conductivity_2 - conductivity_1) / (domain->surface_front_theta[last_bin] - domain->surface_front_theta[0]) * 
                           (1.0 + (capillary_drive + surfacewater_depth_old) / depth_use);
           
      */
      // Method 2, this works better than the original T-O equation, as it transit from using Redistribute equation to T-O equation.  
      conductivity_1      = conductivity_brooks_corey(domain, domain->surface_front_theta[bin - 1]);
      conductivity_2      = conductivity_brooks_corey(domain, domain->surface_front_theta[bin]); 
      capillary_drive     = capillary_drive_brooks_corey(domain, domain->surface_front_theta[0], domain->surface_front_theta[last_bin]);
      infiltrate_distance = dt * (conductivity_2 - conductivity_1) / (domain->surface_front_theta[bin] - domain->surface_front_theta[bin - 1]) * 
                            (1.0 + (capillary_drive + surfacewater_depth_old) / depth_use);
  
      if (infiltrate_distance < 0.0)
        {
          error = true;
        }
    }
  
  // Limit infiltrate distance.
  if (infiltrate_distance > domain->bottom_depth - domain->surface_front_depth[bin])
    {
      infiltrate_distance = domain->bottom_depth - domain->surface_front_depth[bin];
    }
  
  // Advance front, taking surfacewater_depth.
  water_demand             = infiltrate_distance * (domain->surface_front_theta[bin] - domain->surface_front_theta[bin - 1]);
  if (*surfacewater_depth >= water_demand)
    {
      domain->surface_front_depth[bin] += infiltrate_distance;
      *surfacewater_depth              -= water_demand;
      water_demand                      = 0.0;
    }
  else
    {
      water_demand                     -= *surfacewater_depth;
      domain->surface_front_depth[bin] += *surfacewater_depth / (domain->surface_front_theta[bin] - domain->surface_front_theta[bin - 1]);
      *surfacewater_depth               = 0.0;
    }
    
  // If there is still water_demand, take water from the right side bins.
  while (0.0 < water_demand && bin < last_bin)
    {
      water_avail = (domain->surface_front_theta[last_bin] - domain->surface_front_theta[last_bin - 1]) * 
                           (domain->surface_front_depth[last_bin] - domain->top_depth);
                            
      if (water_avail >= water_demand)
        { // The last_bin has enough water.         
          // Original method, take water and reduce bin depth.
          domain->surface_front_depth[last_bin] -= water_demand / (domain->surface_front_theta[last_bin] - domain->surface_front_theta[last_bin - 1]);

          // UNUSED
          // New method, take water and reduce water content.
          //domain->surface_front_theta[last_bin] -= water_demand / (domain->surface_front_depth[last_bin] - domain->top_depth);   
      
          domain->surface_front_depth[bin]      += water_demand / (domain->surface_front_theta[bin]      - domain->surface_front_theta[bin - 1]);
          water_demand                           = 0.0;          
        }
      else
        { // The last_bin doesn't have enough water, take all that have, go to next.
          water_demand                          -= water_avail;
          domain->surface_front_depth[bin]      += water_avail / (domain->surface_front_theta[bin] - domain->surface_front_theta[bin - 1]);
          domain->surface_front_depth[last_bin]  = domain->top_depth;
          domain->surface_front_theta[last_bin]  = domain->surface_front_theta[0];
          domain->d_theta[last_bin]              = 0.0;
          last_bin--;
        }
    }
     
  return error;
}

/* Advance wetting front in a bin using Green-Ampt with redistribution equation.
 * Parameters:
 * domain                - A pointer to the garto_domain struct.
 * dt                    - Time step used in seconds[s]. 
 * surfacewater_depth    - A scalar passed by reference, containing surface water ponded depth in meters[m].
 * groundwater_recharge  - A scalar passed by reference, containing groundwater recharge in meters[m].
 * bin                   - Bin number.
 */
int garto_infiltrate_redistribute(garto_domain* domain, double dt, double* surfacewater_depth, double* groundwater_recharge, int bin)
{
  int    error                 = false;
  double infiltration_capacity = 0.0; // Meters of surface water per second [m/s].
  double dry_depth             = 0.0;  
  double conductivity          = 0.0;
  double conductivity_2        = 0.0;
  double capillary_drive       = 0.0;
  double factor                = 1.0;
  double bin_initial_water     = 0.0;
  
  // Step 1 , calculate d_theta / dt. 
  if (epsilonEqual(domain->top_depth, domain->surface_front_depth[bin]))
    {  
      dry_depth             = garto_find_dry_depth(domain, dt);
      conductivity          = conductivity_brooks_corey(domain, domain->surface_front_theta[bin - 1]);
      infiltration_capacity = domain->parameters->saturated_conductivity * (1.0 + domain->parameters->effective_capillary_suction / dry_depth);
      domain->d_theta[bin]  = dt * (*surfacewater_depth / dt - 1.0 * conductivity) / dry_depth;
    }
  else
    {
      //double conductivity_1  = conductivity_brooks_corey(domain, domain->surface_front_theta[bin - 1]);
      conductivity_2  = conductivity_brooks_corey(domain, domain->surface_front_theta[bin]); 
      capillary_drive = capillary_drive_brooks_corey(domain, domain->surface_front_theta[bin - 1], domain->surface_front_theta[bin]);
               
      // FIXME later, if use capillary_drive calculated above, it is too small, give oscillation in infiltration rate.
    //infiltration_capacity = domain->parameters->saturated_conductivity * (1.0 + capillary_drive / (domain->surface_front_depth[bin] - domain->top_depth));
      infiltration_capacity = domain->parameters->saturated_conductivity * (1.0 + domain->parameters->effective_capillary_suction /
                                                                                 (domain->surface_front_depth[bin] - domain->top_depth));
      
      if (0.0 < *surfacewater_depth)
        {
          factor                = 1.0;
          domain->d_theta[bin]  = dt * (*surfacewater_depth / dt - conductivity_2 - 
                                        (factor * domain->parameters->saturated_conductivity * capillary_drive) / 
                                        (domain->surface_front_depth[bin] - domain->top_depth) ) /  (domain->surface_front_depth[bin] - domain->top_depth);
        }
      else
        {
          factor                = 1.7;
          domain->d_theta[bin]  = dt * ( - conductivity_2 - (factor * domain->parameters->saturated_conductivity * capillary_drive) / 
                                        (domain->surface_front_depth[bin] - domain->top_depth) ) /  (domain->surface_front_depth[bin] - domain->top_depth);
        }
    }
  
  // Step 2, advance water content theta.
  // FIXME later, if it's close to saturated and theta increasing, make it saturated?
  // if (domain->surface_front_theta[bin] > 0.98 * domain->parameters->theta_s && domain->d_theta[bin] > 0.0)
     //{
      //    domain->d_theta[bin] = domain->parameters->theta_s - domain->surface_front_theta[bin];
     //}
       
  bin_initial_water = (domain->surface_front_depth[bin] - domain->top_depth)* (domain->surface_front_theta[bin] - domain->surface_front_theta[bin - 1]);
  if (domain->surface_front_theta[bin] + domain->d_theta[bin] <= domain->parameters->theta_s &&
      domain->surface_front_theta[bin] + domain->d_theta[bin] >= domain->surface_front_theta[bin - 1])
    { // theta[bin], between theta[bin - 1] and theta_s.
      domain->surface_front_theta[bin] += domain->d_theta[bin];
    } 
  else if (domain->surface_front_theta[bin] + domain->d_theta[bin] < domain->surface_front_theta[bin - 1])
    { // theta[bin] less than theta[bin - 1], use theta[bin - 1].
      domain->surface_front_theta[bin] = domain->surface_front_theta[bin - 1];
    }
  else if (domain->surface_front_theta[bin] + domain->d_theta[bin] > domain->parameters->theta_s)
    { // theta[bin] greater than theta_s, use theta_s.
      domain->surface_front_theta[bin] = domain->parameters->theta_s;
    }
  
  // Step 3, advance front depth.
  if (domain->surface_front_theta[bin] > domain->surface_front_theta[bin - 1])
    { // Newly calculated theta[bin] > theta[bin - 1].
      // Limit the infiltrate distance as it hit groundwater or domain bottom.
      // garto_limit_infiltration_capacity(domain, dt, bin, &infiltration_capacity);
        
      if (*surfacewater_depth > infiltration_capacity * dt)
        {
          domain->surface_front_depth[bin] = domain->top_depth + (infiltration_capacity * dt + bin_initial_water) / 
                                             (domain->surface_front_theta[bin] - domain->surface_front_theta[bin - 1]);
          *surfacewater_depth             -= infiltration_capacity * dt;
        }
      else
        {
          domain->surface_front_depth[bin] = domain->top_depth + (*surfacewater_depth + bin_initial_water) / 
                                             (domain->surface_front_theta[bin] - domain->surface_front_theta[bin - 1]);
          *surfacewater_depth              = 0.0;
        }
    }
  else if (epsilonEqual(domain->surface_front_theta[bin], domain->surface_front_theta[bin - 1]))
    { // Newly calculated theta[bin] <= theta[bin - 1]. 
      if (2 <= bin)
        { // Not first bin, put initial water and if any infilatrated water into left bin, then remove this bin.
          if (*surfacewater_depth > infiltration_capacity * dt)
            {
              domain->surface_front_depth[bin - 1] += (infiltration_capacity * dt + bin_initial_water) / 
                                                      (domain->surface_front_theta[bin - 1] - domain->surface_front_theta[bin - 2]);
              *surfacewater_depth                  -= infiltration_capacity * dt;
            }
          else
            {
              domain->surface_front_depth[bin - 1] += (*surfacewater_depth  + bin_initial_water) / 
                                                      (domain->surface_front_theta[bin - 1] - domain->surface_front_theta[bin - 2]);
              *surfacewater_depth                   = 0.0;
            }
        }
      else if (1 == bin)
        { // First bin, put initial water and if any infiltrated water into groundwater, recharge, then remove this bin.
          if (*surfacewater_depth > infiltration_capacity * dt)
            {
              *groundwater_recharge += (infiltration_capacity * dt + bin_initial_water);
              *surfacewater_depth   -= infiltration_capacity * dt;
            }
          else
            {
              *groundwater_recharge += (*surfacewater_depth  + bin_initial_water);
              *surfacewater_depth    = 0.0;
            }
        }
        
      domain->surface_front_depth[bin]     = domain->top_depth;
      domain->surface_front_theta[bin]     = domain->surface_front_theta[0];
      domain->d_theta[bin]                 = 0.0;
    }
  
  return error;
}

/* Process infiltration and redistribution step of the simulation.
 * Parameters:
 * domain                 - A pointer to the garto_domain struct.
 * dt                     - Time step used in seconds[s]. 
 * surfacewater_depth     - A scalar passed by reference, containing surface water ponded depth in meters[m].
 * groundwater_recharge   - A scalar passed by reference, containing recharge to groundwater in meters[m] (Note: This is not used in this function).
 * surfacewater_depth_old - A scalar use to determine original rainfall rate.
 */
int garto_surface_front_infiltrate(garto_domain* domain, double dt, double* surfacewater_depth, double* groundwater_recharge, double surfacewater_depth_old)
{
  int jj, error = false;
  for (jj = 1; jj <= domain->parameters->num_bins && epsilonLess(domain->surface_front_theta[0], domain->parameters->theta_s); jj++)
     {
       if (epsilonLess(domain->surface_front_theta[jj], domain->parameters->theta_s))
         { // domain->surface_front_theta[jj] < domain->parameters->theta_s.  This bin is not saturated.
           if (jj < domain->parameters->num_bins)
             { // jj < domain->parameters->num_bins.  This is not the last bin.
               if (domain->surface_front_theta[jj] > domain->surface_front_theta[0] && 
                   domain->surface_front_theta[jj] < domain->surface_front_theta[jj + 1])
                 { // Not the last bin, theta[j] has water, and theta[j + 1] has water, use T-O advance.
                   domain->d_theta[jj] = 0; 
                   // Advance front using T-O equation.
                   error = t_o_advance_front(domain, dt, surfacewater_depth, jj, surfacewater_depth_old);
                 }
               else
                 { // Not the last allowable bin, but the rightmost bin, either create front or redistribute.
                   // The rightmost bin means it is either the righmost bin that has water, or bin 1 if all bins are empty.
                   // We break out of the loop in this case so we do not process dry bins to the right of the rightmost bin.
                   if (surfacewater_depth_old >= domain->parameters->saturated_conductivity * dt && domain->d_theta[jj] < 0.0 && 
                        domain->surface_front_theta[jj] > domain->surface_front_theta[0])
                     { // Create front to the right.
                       error = garto_create_right_bin(domain, dt, surfacewater_depth, jj);
                     }
                   else
                     { // Redistriute or rising.
                       error = garto_infiltrate_redistribute(domain, dt, surfacewater_depth, groundwater_recharge, jj);
                     }
                    
                    break;
                 }
             }
           else
             { // jj = domain->parameters->num_bins, and domain->surface_front_theta[jj] < domain->parameters->theta_s.
               // Redistriute or rising.
               error = garto_infiltrate_redistribute(domain, dt, surfacewater_depth, groundwater_recharge, jj);
               break;
             }
         }
       else
         { // domain->surface_front_theta[jj] = domain->parameters->theta_s
           if (surfacewater_depth_old >= domain->parameters->saturated_conductivity * dt)
             { // Fully saturated front with rainfall_depth >= K_s * dt, advance using GA equation.
               error = t_o_advance_front(domain, dt, surfacewater_depth, jj, surfacewater_depth_old);
             }
           else
             { // Fully saturated front with rainfall_depth < K_s * dt, redistribution.
               error = garto_infiltrate_redistribute(domain, dt, surfacewater_depth, groundwater_recharge, jj);
             }
           break;
         }
     }// End of element loop.
  
  // Check if any right front is deeper than left front.
  error = garto_check_surface_front_overshoot(domain);
     
  return error;
}

/* Merge groundwater_front[bin] and groundwater_front[bin - 1].
 * Parameters:
 * domain                - A pointer to the garto_domain struct.
 * bin                   - Bin number.
 */
int garto_merge_groundwater_front(garto_domain* domain, int bin)
{
  int    ii;
  int    error             = false;
  double water_in_two_bins = 0.0;
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)   
  CkAssert(bin >= 2);
#endif  
  if (bin >= 2)
    {
      water_in_two_bins                        = (domain->bottom_depth - domain->groundwater_front_depth[bin - 1]) * 
                                                 (domain->groundwater_front_theta[bin - 1] - domain->groundwater_front_theta[bin - 2]) + 
                                                 (domain->bottom_depth - domain->groundwater_front_depth[bin]) * 
                                                 (domain->groundwater_front_theta[bin] - domain->groundwater_front_theta[bin - 1]); 
      domain->groundwater_front_depth[bin - 1] = domain->bottom_depth - 
                                                 water_in_two_bins / (domain->groundwater_front_theta[bin] - domain->groundwater_front_theta[bin - 2]);
      domain->groundwater_front_theta[bin - 1] = domain->groundwater_front_theta[bin];
      domain->groundwater_front_depth[bin]     = domain->bottom_depth;
      domain->groundwater_front_theta[bin]     = domain->groundwater_front_theta[0];   
      
      // After mering [bin] and [bin - 1] to [bin - 1], it might need to shift fronts [>= bin +1] to left.
      for (ii = bin + 1; ii <= domain->parameters->num_bins; ii ++)
         {
           if (domain->groundwater_front_theta[ii] > domain->groundwater_front_theta[0])
             { // Shift to the left.
               domain->groundwater_front_theta[ii - 1] = domain->groundwater_front_theta[ii];
               domain->groundwater_front_depth[ii - 1] = domain->groundwater_front_depth[ii]; 
               domain->groundwater_front_theta[ii]     = domain->groundwater_front_theta[0];
               domain->groundwater_front_depth[ii]     = domain->bottom_depth;
             }
         }                    
    }
  
  return error;
}

/* Check if it need to create one new groundwater front. If there is no ponded surface water, no surface water front, 
 * and hydrostatic surface water content is less than current water content, create groundwater front.
 * Parameters:
 * domain                - A pointer to the garto_domain struct.
 * surfacewater_depth    - A scalar passed by reference, containing surface water ponded depth in meters[m].
 * water_table           - The depth of the water table in meters[m].
 */
int garto_create_groundwater_front(garto_domain* domain, double* surfacewater_depth, double water_table)
{
  int    ii;
  int    error                     = false;
  int    has_surfacewater_front    = false;
  int    last_bin                  = 0;
  int    incre                     = 0; 
  int    num_divide                = 0;
  double bin_width                 = 0.0;
  double water_content_hydrostatic = 0.0;
  
  for (ii = 1; ii <= domain->parameters->num_bins; ii++)
     {
       if (domain->surface_front_theta[ii] > domain->surface_front_theta[0] &&
           domain->surface_front_depth[ii] > domain->top_depth)
         {
           has_surfacewater_front = true;
           break;
         }
     }
     
  if ((water_table - domain->top_depth) > domain->parameters->bc_psib)
    {
      water_content_hydrostatic = domain->parameters->theta_r + (domain->parameters->theta_s - domain->parameters->theta_r) * 
                                  pow((water_table - domain->top_depth) / domain->parameters->bc_psib, -domain->parameters->bc_lambda);
    }
  else
    {
      water_content_hydrostatic = domain->parameters->theta_s;
    }
  
  // FIXME later, this might create to many fronts when water table droping,use     water_content_hydrostatic < 0.95 * domain->groundwater_front_theta[0]
  if (false == has_surfacewater_front && epsilonEqual(0.0, *surfacewater_depth) && water_content_hydrostatic < 0.95 * domain->groundwater_front_theta[0])
    { // No surface water, no surfacewater_front, and calculated new_theta_i less than current theta_i, create a new groundwater front. 
      
      // It already has domain->parameters->num_bins bins, merge last two groundwater fronts, before create groundwater front.
      if (epsilonGreater(domain->groundwater_front_theta[domain->parameters->num_bins], domain->groundwater_front_theta[0]))
        {
          garto_merge_groundwater_front(domain, domain->parameters->num_bins);
        }
      
      // Find last_bin (because merger above, last_bin <= domain->parameters->num_bins - 1 ).
      last_bin = 0;
      for (ii = domain->parameters->num_bins - 1; ii >= 1; ii--)
         {
           if (epsilonGreater(domain->groundwater_front_theta[ii], domain->groundwater_front_theta[0]))
             {
               last_bin = ii;
               break;
             }
         }
       
      // Shift bins. 
      for (ii = last_bin; ii >= 1; ii--)
         {
           domain->groundwater_front_theta[ii + 1] = domain->groundwater_front_theta[ii];
           domain->groundwater_front_depth[ii + 1] = domain->groundwater_front_depth[ii];
         }
      domain->groundwater_front_theta[1] = domain->groundwater_front_theta[0];
      domain->groundwater_front_depth[1] = domain->top_depth;
      domain->groundwater_front_theta[0] = water_content_hydrostatic;
      
      for (ii = last_bin + 2; ii <= domain->parameters->num_bins; ii++)
         {
           domain->groundwater_front_theta[ii] = domain->groundwater_front_theta[0];
           domain->groundwater_front_depth[ii] = domain->bottom_depth;
         }
      
      // change surface front bins.
      domain->surface_front_theta[0] = water_content_hydrostatic;
      for (ii = 1; ii <= domain->parameters->num_bins; ii++)
         {
           domain->surface_front_theta[ii] = domain->surface_front_theta[0];
           domain->surface_front_depth[ii] = domain->top_depth;
         }
      
      // If number of groundwater bins < number of bins,  evenly dived groundwater_bin[1].
      last_bin     = find_groundwater_last_bin(domain);
      if (last_bin < domain->parameters->num_bins)
        {
          for (ii = last_bin; ii >= 2; ii--)
             { 
               incre                                       = domain->parameters->num_bins - last_bin;
               domain->groundwater_front_theta[ii + incre] = domain->groundwater_front_theta[ii];
               domain->groundwater_front_depth[ii + incre] = domain->groundwater_front_depth[ii];
             }
          num_divide = domain->parameters->num_bins - last_bin + 1;
          bin_width  = (domain->groundwater_front_theta[1] - domain->groundwater_front_theta[0]) / num_divide;
          for (ii = 1; ii <= num_divide; ii++)
             {
               domain->groundwater_front_theta[ii] = domain->groundwater_front_theta[0] + ii * bin_width;
               domain->groundwater_front_depth[ii] = domain->groundwater_front_depth[1];
             }
        }
    } // End of create new groundwater front.
    
  return error;
}

/* Process the groundwater step of the simulation.
 * Parameters:
 * domain                - A pointer to the garto_domain struct.
 * dt                    - Time step used in seconds[s]. 
 * surfacewater_depth    - A scalar passed by reference, containing surface water ponded depth in meters[m].
 * groundwater_recharge  - A scalar passed by reference, containing recharge to groundwater in meters[m].
 * water_table           - The depth of the water table in meters[m].
 */
int garto_groundwater(garto_domain* domain, double dt, double* surfacewater_depth, double* groundwater_recharge, double water_table)
{ 
  int    ii;
  int    last_bin;
  int    error           = false;
  double length          = 0.0;
  double capillary_drive = 0.0; 
  double conductivity    = 0.0;
  double conductivity_0  = 0.0;
  double distance        = 0.0;
  // Check if it need to create one new groundwater front.
  garto_create_groundwater_front(domain, surfacewater_depth, water_table);
  
  // Move groundwater fronts.    
  for (ii = 1; ii <= domain->parameters->num_bins && epsilonGreater(domain->groundwater_front_theta[ii], domain->groundwater_front_theta[0]); ii++)
     {
       capillary_drive = pressure_head_brooks_corey(domain, domain->groundwater_front_theta[ii]); 
       conductivity    = conductivity_brooks_corey( domain, domain->groundwater_front_theta[ii]);
       conductivity_0  = conductivity_brooks_corey( domain, domain->groundwater_front_theta[0]);
       distance        = 0.0;
       
       if (water_table > domain->groundwater_front_depth[ii])
         { // Groundwater_front above water table.
           length     = water_table - domain->groundwater_front_depth[ii];
           if (length > capillary_drive)
             { // Groundwater_front above hydrostatic, move downward.
               distance      = dt * (conductivity - conductivity_0) * (1.0 - capillary_drive / length) / 
                                (domain->groundwater_front_theta[ii] - domain->groundwater_front_theta[0]);
                                
               // Can't pass hydrostatic.
               if (distance > length - capillary_drive)
                 {
                   distance = length - capillary_drive;
                 }

               // Can't pass domain bottom.
               if (domain->groundwater_front_depth[ii] + distance >= domain->bottom_depth)
                 {
                   distance = domain->bottom_depth - domain->groundwater_front_depth[ii];
                 }
             }
           else if (length < capillary_drive)
             { // Groundwater_front below hydrostatic, move upward.
               // FIXME later, it seems to use conductivity = K[last_bin] is better in case of rising water table.
               last_bin     = find_groundwater_last_bin(domain);
               conductivity = conductivity_brooks_corey( domain, domain->groundwater_front_theta[last_bin]);
               distance     = dt * (conductivity - conductivity_0) * (1.0 - capillary_drive / length) / 
                                  (domain->groundwater_front_theta[ii] - domain->groundwater_front_theta[0]);

               // Can't pass hydrostatic.
               if (distance < length - capillary_drive)
                 {
                   distance = length - capillary_drive;
                 }

               // Can't pass domain top.
               if (domain->groundwater_front_depth[ii] + distance <= domain->top_depth)
                 {
                   //distance = domain->top_depth - domain->groundwater_front_depth[ii];
                 }
             }
         }
       else
         { // Groundwater_front below water table.
           distance = water_table - domain->groundwater_front_depth[ii] - 0.1 * capillary_drive;
           // Can't pass domain top.
           if (domain->groundwater_front_depth[ii] + distance <= domain->top_depth)
             {
               distance = domain->top_depth - domain->groundwater_front_depth[ii];
             }
         }

       domain->groundwater_front_depth[ii] += distance;
       *groundwater_recharge               += distance * (domain->groundwater_front_theta[ii] - domain->groundwater_front_theta[ii - 1]); 
     }
  
  // Check if right bin is shallower than left bin.
  for (ii = domain->parameters->num_bins ; ii >= 2; ii--)
     {
       if (!epsilonGreater(domain->groundwater_front_depth[ii], domain->groundwater_front_depth[ii - 1]) &&
            epsilonGreater(domain->groundwater_front_theta[ii], domain->groundwater_front_theta[0]))
         {
           garto_merge_groundwater_front(domain, ii);
           ii++; // Need to re-check the bin on the right in case it is shallower than the newly merged bin.
         }
     }
     
  return error;
}

/* Check if surface water bin hit groundwater or pass domain depth at the end of each time step.
 * Parameters:
 * domain                - A pointer to the garto_domain struct.
 * groundwater_recharge  - A scalar passed by reference, containing recharge to groundwater in meters[m].
 */
int garto_merge_surface_groundwater_overlapped(garto_domain* domain, double* groundwater_recharge)
{ 
  int error = false;
  int ii, jj;
  int merged = false;
  
  if (domain->yes_groundwater)
    { 
      // Check surface_front hit groundwater.
      for (ii = 1; ii <= domain->parameters->num_bins && domain->surface_front_theta[ii] > domain->surface_front_theta[0]; ii++)
         {
           for (jj = 1; jj <= domain->parameters->num_bins && domain->groundwater_front_theta[jj] > domain->groundwater_front_theta[0]; jj++)
              {
                if ((!epsilonLess(domain->surface_front_depth[ii], domain->groundwater_front_depth[jj])) &&
                    domain->surface_front_theta[ii - 1] < domain->groundwater_front_theta[jj] &&
                    domain->surface_front_theta[ii]     > domain->groundwater_front_theta[jj - 1])
                  {
                    merged = true;
                    if (domain->surface_front_theta[ii - 1] <= domain->groundwater_front_theta[jj - 1])
                      {
                        if (domain->surface_front_theta[ii] <= domain->groundwater_front_theta[jj])
                          {
                            *groundwater_recharge += (domain->surface_front_depth[ii] - domain->groundwater_front_depth[jj]) * 
                                                     (domain->surface_front_theta[ii] - domain->groundwater_front_theta[jj - 1]);
                            domain->surface_front_theta[0]     = domain->surface_front_theta[ii];
                            domain->groundwater_front_theta[0] = domain->surface_front_theta[ii];
                          }
                        else
                          {
                            *groundwater_recharge += (domain->surface_front_depth[ii] - domain->groundwater_front_depth[jj]) * 
                                                     (domain->groundwater_front_theta[jj] - domain->groundwater_front_theta[jj - 1]);
                            domain->surface_front_theta[0]     = domain->groundwater_front_theta[jj];
                            domain->groundwater_front_theta[0] = domain->groundwater_front_theta[jj];
                          }
                      }
                    else 
                      {
                        if (domain->surface_front_theta[ii] <= domain->groundwater_front_theta[jj])
                          {
                            *groundwater_recharge += (domain->surface_front_depth[ii] - domain->groundwater_front_depth[jj]) * 
                                                     (domain->surface_front_theta[ii] - domain->surface_front_theta[ii - 1]);
                            domain->surface_front_theta[0]     = domain->surface_front_theta[ii];
                            domain->groundwater_front_theta[0] = domain->surface_front_theta[ii];
                          }
                        else
                          {
                            *groundwater_recharge += (domain->surface_front_depth[ii] - domain->groundwater_front_depth[jj]) * 
                                                     (domain->groundwater_front_theta[jj] - domain->surface_front_theta[ii - 1]);
                            domain->surface_front_theta[0]     = domain->groundwater_front_theta[jj];
                            domain->groundwater_front_theta[0] = domain->groundwater_front_theta[jj];
                          }
                      }
                  }
              }
         }
       
       // Check groundwater hit domain top.
       for (ii = 1; ii <= domain->parameters->num_bins && domain->groundwater_front_theta[ii] > domain->groundwater_front_theta[0]; ii++)
          {
            if (domain->groundwater_front_depth[ii] <= domain->top_depth)
              {
                merged = true;
                *groundwater_recharge += (domain->top_depth - domain->groundwater_front_depth[ii]) * 
                                                     (domain->groundwater_front_theta[ii] - domain->groundwater_front_theta[ii - 1]);
                domain->surface_front_theta[0]     = domain->groundwater_front_theta[ii];
                domain->groundwater_front_theta[0] = domain->groundwater_front_theta[ii];
              } 
          }
       
       // Shift bins if merged is true.
       if (true == merged)
         {
           // Shift bins to the left.
           jj = 1;
           for (ii = 1; ii <= domain->parameters->num_bins; ii++)
             {
               if (domain->surface_front_theta[ii] > domain->surface_front_theta[0])
                 {
                   domain->surface_front_theta[jj] = domain->surface_front_theta[ii];
                   domain->surface_front_depth[jj] = domain->surface_front_depth[ii];
                   jj++;  
                 }
             }
           for (ii = jj; ii <= domain->parameters->num_bins; ii++)
             {
               domain->surface_front_theta[ii] = domain->surface_front_theta[0];
               domain->surface_front_depth[ii] = domain->top_depth;
             }
       
           jj = 1;
           for (ii = 1; ii <= domain->parameters->num_bins; ii++)
             {
               if (domain->groundwater_front_theta[ii] > domain->groundwater_front_theta[0])
                 {
                   domain->groundwater_front_theta[jj] = domain->groundwater_front_theta[ii];
                   domain->groundwater_front_depth[jj] = domain->groundwater_front_depth[ii];
                   jj++;
                 }
             }
           for (ii = jj; ii <= domain->parameters->num_bins; ii++)
             {
               domain->groundwater_front_theta[ii] = domain->groundwater_front_theta[0];
               domain->groundwater_front_depth[ii] = domain->bottom_depth;
             }
  
         }
    }
  else
    { // Yes_groundwater is false.
      for (ii = 1; ii <= domain->parameters->num_bins; ii++)
         {
           if (domain->surface_front_theta[ii] > domain->surface_front_theta[0] &&
               (epsilonEqual(domain->surface_front_depth[ii], domain->bottom_depth) || epsilonGreater(domain->surface_front_depth[ii], domain->bottom_depth)))
             {
               merged = true;
               *groundwater_recharge += (domain->surface_front_depth[ii] - domain->bottom_depth) * 
                                        (domain->surface_front_theta[ii] - domain->surface_front_theta[ii - 1]);
               domain->surface_front_theta[0]     = domain->surface_front_theta[ii];
             }
         }
      
      if (true == merged)
        {
          jj = 1;
          for (ii = 1; ii <= domain->parameters->num_bins; ii++)
             {
               if (domain->surface_front_theta[ii] > domain->surface_front_theta[0])
                 {
                   domain->surface_front_theta[jj] = domain->surface_front_theta[ii];
                   domain->surface_front_depth[jj] = domain->surface_front_depth[ii];
                   jj++;  
                 }
             }
           for (ii = jj; ii <= domain->parameters->num_bins; ii++)
              {
                domain->surface_front_theta[ii] = domain->surface_front_theta[0];
                domain->surface_front_depth[ii] = domain->top_depth;
              } 
        }
    }
  
  return error;
}

/* Process infiltration into the completely saturated domain.
 * Parameters:
 * domain                - A pointer to the garto_domain struct.
 * dt                    - Time step used in seconds[s]. 
 * surfacewater_depth    - A scalar passed by reference, containing surface water ponded depth in meters[m].
 * groundwater_recharge  - A scalar passed by reference, containing recharge to groundwater in meters[m].
 * water_table           - The depth of the water table in meters[m].
 */
void garto_saturated_bin(garto_domain* domain, double dt, double* surfacewater_depth, double* groundwater_recharge, double water_table)
{
  double conductivity = conductivity_brooks_corey(domain, domain->surface_front_theta[0]);
  double water_demand;
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  if (domain->yes_groundwater)
    {
      CkAssert(epsilonEqual(domain->surface_front_theta[0], domain->groundwater_front_theta[0]));
    }
#endif
  if (domain->yes_groundwater)
    {
      water_demand = conductivity * dt * water_table / (domain->bottom_depth - domain-> top_depth); // FIXME, add 08/13/14.
    }
  else
    {
      water_demand = conductivity * dt; 
    } 
  if (*surfacewater_depth   >= water_demand)
    {
      *surfacewater_depth   -= water_demand;
      *groundwater_recharge += water_demand;
    }
  else
    {
      *groundwater_recharge += *surfacewater_depth;
      *surfacewater_depth    = 0.0;
    }
  
  return;
}

int garto_timestep(garto_domain* domain, double dt, double* surfacewater_depth, double water_table, double* groundwater_recharge)
{
  double surfacewater_depth_old = *surfacewater_depth;
  int    error                   = false;
  int    No_flow_lower_bc        = false;   // No flow lower boundary condition is for certain testing problems, put inside garto_timestep().
  
  // Flow through saturated bin.
  garto_saturated_bin(domain, dt, surfacewater_depth, groundwater_recharge, water_table);
  
  // Surface front.
  error = garto_surface_front_infiltrate(domain, dt, surfacewater_depth, groundwater_recharge, surfacewater_depth_old);
   
  // FIXME, later, To simulate tests with no flow boundary condition, deactive garto_groundwater(), call garto_add_groundwater() instead.
  if (false == No_flow_lower_bc)
    {
      if (!error && domain->yes_groundwater)
        { 
          // Groundwater front.
          error = garto_groundwater(domain, dt, surfacewater_depth, groundwater_recharge, water_table);
        }
    }
  else
    { // TURE = No_flow_lower_bc.
      if (!error && domain->yes_groundwater)
        {
          garto_add_groundwater(domain, groundwater_recharge);
        }
    }

  if (!error)
    {
      // Check if surface water bin hit groundwater or pass domain depth.
      garto_merge_surface_groundwater_overlapped(domain, groundwater_recharge);
    }
  
  return error;
}

void garto_add_groundwater(garto_domain* domain, double* groundwater_recharge)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  CkAssert(NULL != domain && NULL != groundwater_recharge && 0.0 <= *groundwater_recharge);
#endif
  
  int    last_groundwater_bin = find_groundwater_last_bin(domain);
  int    ii                   = last_groundwater_bin;
  int    jj, last_surface_bin;
  double space;
  
  while (epsilonLess(0.0, *groundwater_recharge) && ii >= 2)
    {
      // Availabe space in meters of water.
      space = (domain->groundwater_front_theta[ii] - domain->groundwater_front_theta[ii - 1]) * 
              (domain->groundwater_front_depth[ii] - domain->groundwater_front_depth[ii - 1]);
       if (space > *groundwater_recharge)
         {
           domain->groundwater_front_depth[ii] -= *groundwater_recharge / (domain->groundwater_front_theta[ii] - 
                                                                           domain->groundwater_front_theta[ii - 1]);
           *groundwater_recharge                = 0.0;         
         }
       else
         {
           domain->groundwater_front_depth[ii] = domain->groundwater_front_depth[ii - 1];
           garto_merge_groundwater_front(domain, ii);
           ii--;
           *groundwater_recharge              -= space;
         }
    }
  
  if (epsilonLess(0.0, *groundwater_recharge))
    {
      if (domain->surface_front_theta[0] < domain->surface_front_theta[1])
        { // Has surface front bins.
          space = (domain->groundwater_front_depth[1] - domain->surface_front_depth[1]) * 
                  (domain->parameters->theta_s        - domain->surface_front_theta[0]);
          if (space > *groundwater_recharge)
            {
              domain->groundwater_front_depth[ii] -= *groundwater_recharge / (domain->groundwater_front_theta[1] - domain->groundwater_front_theta[0]);
              *groundwater_recharge                = 0.0;
            }
          else
            {
              domain->groundwater_front_depth[ii] = domain->surface_front_depth[1];
              *groundwater_recharge              -= space; 
            }
          
          jj               = 2;
          last_surface_bin = find_surface_last_bin(domain);
          while (epsilonLess(0.0, *groundwater_recharge) && jj <= last_surface_bin)
            {
              space = (domain->surface_front_depth[jj - 1] - domain->surface_front_depth[jj]) *
                      (domain->surface_front_theta[jj]     - domain->surface_front_theta[jj - 1]);
              if (space > *groundwater_recharge)
                {
                  domain->surface_front_depth[jj] += *groundwater_recharge / (domain->surface_front_theta[jj] - domain->surface_front_theta[jj - 1]);
                  *groundwater_recharge            = 0.0;
                }
              else
                {
                  domain->surface_front_depth[jj] = domain->surface_front_depth[jj - 1];
                  *groundwater_recharge          -= space;
                  jj++; 
                }
            }
          
          if (epsilonLess(0.0, *groundwater_recharge) && domain->surface_front_theta[last_surface_bin] < domain->parameters->theta_s)
            {
              space = (domain->surface_front_depth[last_surface_bin] - domain->top_depth) * 
                      (domain->parameters->theta_s - domain->surface_front_theta[last_surface_bin]);
              if (last_surface_bin < domain->parameters->num_bins)
                { // Last_surface_bin is less than num_bins, create a bin to the right.
                  if (space > *groundwater_recharge)
                    {
                      domain->surface_front_theta[last_surface_bin + 1]  = domain->parameters->theta_s;
                      domain->surface_front_depth[last_surface_bin + 1] += *groundwater_recharge / 
                                                                          (domain->parameters->theta_s - domain->surface_front_theta[last_surface_bin]);
                      *groundwater_recharge                              = 0.0;
                    }
                  else
                    {
                      domain->surface_front_theta[last_surface_bin + 1]  = domain->parameters->theta_s;
                      domain->surface_front_depth[last_surface_bin + 1] += domain->surface_front_depth[last_surface_bin];
                      *groundwater_recharge                             -= space;
                    }
                }
              else if (last_surface_bin == domain->parameters->num_bins)
                { // Last_surface_bin equals num_bins, increase theta.
                  if (space > *groundwater_recharge)
                    {
                      domain->surface_front_theta[last_surface_bin] += *groundwater_recharge / (domain->surface_front_depth[last_surface_bin] - 
                                                                                                domain->top_depth);
                      *groundwater_recharge                          = 0.0;
                    }
                  else
                    {
                      domain->surface_front_theta[last_surface_bin]  = domain->parameters->theta_s;
                      *groundwater_recharge                         -= space;
                    }
                }
              
            }
        }
      else if (domain->surface_front_theta[0] < domain->parameters->theta_s)
        { // No surface front bins and groundwater not completey full.
          space = (domain->groundwater_front_depth[1] - domain->top_depth) * 
                  (domain->parameters->theta_s        - domain->surface_front_theta[0]);
          if (space > *groundwater_recharge)
            {
              domain->groundwater_front_depth[ii] -= *groundwater_recharge / (domain->groundwater_front_theta[1] - domain->groundwater_front_theta[0]);
              *groundwater_recharge                = 0.0;
            }
          else
            {
              domain->groundwater_front_depth[ii] = domain->top_depth;
              *groundwater_recharge              -= space;
            }
        }
    }
  
   // Check if surface water bin hit groundwater or pass domain depth.
   garto_merge_surface_groundwater_overlapped(domain, groundwater_recharge);
}

void garto_take_groundwater(garto_domain* domain, double water_table, double* groundwater_recharge)
{ 
  int    ii;
  int    last_bin      = find_groundwater_last_bin(domain);
  double maximum_depth = 0.0;
  double minimum_depth = 0.0;
  double space         = 0.0;

  // Take groundwater by lowering the depth of bins last_bin through 1.
  for (ii = last_bin; ii >= 1 && epsilonGreater(0.0, *groundwater_recharge); ii--)
    {
      maximum_depth = water_table - 0.1 * pressure_head_brooks_corey(domain, domain->groundwater_front_theta[ii]);

      if (maximum_depth > domain->bottom_depth)
        {
          maximum_depth = domain->bottom_depth;
        }

      if (maximum_depth > domain->groundwater_front_depth[ii])
        {
          space = (maximum_depth - domain->groundwater_front_depth[ii]) * 
                  (domain->groundwater_front_theta[ii] - domain->groundwater_front_theta[ii -1]);
          
          if (space > -*groundwater_recharge)
            {
              domain->groundwater_front_depth[ii] -= *groundwater_recharge / (domain->groundwater_front_theta[ii] - domain->groundwater_front_theta[ii -1]);
              *groundwater_recharge                = 0.0;
            }
          else
            {
              domain->groundwater_front_depth[ii]  = maximum_depth;
              *groundwater_recharge               += space;
            }
        }
    }
  
  // Take groundwater by moving to the left the theta of bin 0.
  if (epsilonGreater(0.0, *groundwater_recharge))
    {
      if (domain->surface_front_theta[0] < domain->parameters->theta_s)
        {
          maximum_depth = domain->groundwater_front_depth[1];
        }
      else
        {
          maximum_depth = domain->bottom_depth;
        }

      if (domain->surface_front_theta[0] < domain->surface_front_theta[1])
        {
          minimum_depth = domain->surface_front_depth[1];
        }
      else
        {
          minimum_depth = domain->top_depth;
        }

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(maximum_depth > minimum_depth);
#endif

      space = (maximum_depth - minimum_depth) * (domain->surface_front_theta[0] - domain->parameters->theta_r);

      if (space > -*groundwater_recharge)
        {
          domain->surface_front_theta[0] += *groundwater_recharge / (maximum_depth - minimum_depth);
          *groundwater_recharge           = 0.0;
        }
      else
        {
          domain->surface_front_theta[0] = domain->parameters->theta_r;
          *groundwater_recharge         += space;
        }
      
      // Update empty bins after changing theta[0].
      for (ii = 1; ii <= domain->parameters->num_bins; ii++)
         {
           if (domain->surface_front_theta[ii]     <= domain->groundwater_front_theta[0])
             {
               domain->surface_front_theta[ii]      = domain->surface_front_theta[0];
             }
           
           if (domain->groundwater_front_theta[ii] <= domain->groundwater_front_theta[0])
             {
               domain->groundwater_front_theta[ii]  = domain->surface_front_theta[0];
             }
         }

      if (domain->groundwater_front_theta[0] >= domain->parameters->theta_s)
        {
          domain->groundwater_front_theta[1] = domain->parameters->theta_s;
          domain->groundwater_front_depth[1] = domain->bottom_depth;
        }

      domain->groundwater_front_theta[0] = domain->surface_front_theta[0];
    }
}

double garto_specific_yield(garto_domain* domain, double water_table)
{
  double specific_yield = 0.0;
  
  if (domain->yes_groundwater && domain->groundwater_front_theta[0] < domain->parameters->theta_s)
    {
      specific_yield = domain->parameters->theta_s - domain->groundwater_front_theta[0];
    }
  else
    {
      specific_yield = (domain->parameters->theta_s - domain->parameters->theta_r) * (1.0 - 
                       pow(domain->parameters->bc_psib / (domain->parameters->bc_psib + water_table + 0.01), domain->parameters->bc_lambda));
    }
  
  if (0.1 > specific_yield)
    {
      specific_yield = 0.1;
    }
  
  if (specific_yield < domain->parameters->theta_r)
    {
      specific_yield = domain->parameters->theta_r;
    }
  
  if (specific_yield > domain->parameters->theta_s)
    {
      specific_yield = domain->parameters->theta_s;
    }
     
  return specific_yield;
}

double garto_evapotranspiration(garto_domain* domain, double evaporation_demand, double transpiration_demand, double root_depth)
{
  int    ii;                         // Loop counter.
  double bin_water            = 0.0; // Amount of water in unit of depth in a bin.
  double delta_theta          = 0.0; // Change of water content due to transpiration, unitless.
  double evaporate_depth      = 0.0; // Maximum evaporate depth (in meters), when there is no surface front.
  double evaporated_water     = 0.0; // Amount of water evaporated in meters of water.
  
  if (root_depth > domain->bottom_depth)
    {
      root_depth = domain->bottom_depth;
    }
      
  // Step 1, Satisfy evaporation demand from surface front water, starting from right most bin to left.
  if (0.0 < evaporation_demand)
    {
      for (ii = domain->parameters->num_bins; ii >= 1 && evaporation_demand > 0.0; ii--)
         {
           if (epsilonGreater(domain->surface_front_theta[ii], domain->surface_front_theta[0]))
             { // There is water in bin ii.
               bin_water = (domain->surface_front_theta[ii] - domain->surface_front_theta[ii - 1]) * (domain->surface_front_depth[ii] - domain->top_depth);
               if (bin_water > evaporation_demand)
                 { // Evaporation_demand can be satisfy.
                   domain->surface_front_theta[ii] -= evaporation_demand / (domain->surface_front_depth[ii] - domain->top_depth);
                   evaporated_water                += evaporation_demand;
                   evaporation_demand               = 0.0;
                   break;
                 }
               else
                 { // Take all water in the bin.
                   evaporated_water               += bin_water;
                   evaporation_demand             -= bin_water;
                   domain->surface_front_theta[ii] = domain->surface_front_theta[0];
                   domain->surface_front_depth[ii] = domain->top_depth;
                 }
             }
         }
    }// End of step 1, evaporation from surface front water.
  
  // Step 2, When there is no surface front bins, satisfy evaporation demand from fully saturated domain.
  if (domain->yes_groundwater && domain->surface_front_theta[0] < domain->parameters->theta_s)
    {
      evaporate_depth = domain->groundwater_front_depth[1];
    }
  else
    {
      evaporate_depth = domain->bottom_depth;
    }
  
  if (evaporation_demand > 0.0 && domain->surface_front_theta[0] >= domain->surface_front_theta[1] && 
                                  domain->surface_front_theta[0] >  domain->parameters->theta_r)
    {
      bin_water = (domain->surface_front_theta[0] - domain->parameters->theta_r) * (evaporate_depth - domain->top_depth);
      
      if (bin_water > evaporation_demand)
        { // evaporation_demand can be satisfied.
          domain->surface_front_theta[0]    -= evaporation_demand / (evaporate_depth - domain->top_depth);
          evaporated_water                  += evaporation_demand;
          evaporation_demand                 = 0.0;
        }
      else
        { // Take all water in the saturated domain up to the depth of evaporate_depth.
          domain->surface_front_theta[0]     = domain->parameters->theta_r;
          evaporated_water                  += bin_water;
          evaporation_demand                -= bin_water;
        }
      
      // Update empty bins after changing theta[0].
      for (ii = 1; ii <= domain->parameters->num_bins; ii++)
         {
           if (domain->surface_front_theta[ii]     <= domain->groundwater_front_theta[0])
             {
               domain->surface_front_theta[ii]      = domain->surface_front_theta[0];
             }
           
           if (domain->groundwater_front_theta[ii] <= domain->groundwater_front_theta[0])
             {
               domain->groundwater_front_theta[ii]  = domain->surface_front_theta[0];
             }
         }

      if (domain->groundwater_front_theta[0] >= domain->parameters->theta_s)
        {
          domain->groundwater_front_theta[1] = domain->parameters->theta_s;
          domain->groundwater_front_depth[1] = domain->bottom_depth;
        }

      domain->groundwater_front_theta[0] = domain->surface_front_theta[0];
    }// End of step 2, evaporation from fully saturated domain.
  
  // Calculate change of water content due to transpiration (assume uniform through root depth), unitless, delta_theta will be used through steps 3-5.
  if (0.0 < transpiration_demand && root_depth > domain->top_depth)
    { // There is transpiration demand and root depth.
      delta_theta = transpiration_demand / (root_depth - domain->top_depth);
    }
  else
    { // There is no transpiration demand or no root depth.
      return evaporated_water;
    }
  
  // Step 3, Satisfy transpiration demand from surface front water.
  for (ii = domain->parameters->num_bins; ii >= 1; ii--)
     { 
       if (domain->surface_front_depth[ii] > root_depth &&  
           (ii == domain->parameters->num_bins || domain->surface_front_theta[ii + 1] <= domain->surface_front_theta[0]))
         { // The rightmost surface water front depth deeper than root depth, satisfy demand instead of delta_theta.
           bin_water = (domain->surface_front_theta[ii] - domain->surface_front_theta[ii - 1]) * (domain->surface_front_depth[ii] - domain->top_depth);
           
           if (bin_water > transpiration_demand)
             { // Transpiration_demand can be satisfy.
               domain->surface_front_theta[ii] -= transpiration_demand / (domain->surface_front_depth[ii] - domain->top_depth);
               evaporated_water                += transpiration_demand;
               transpiration_demand             = 0.0;
               break;
             }
           else
             { // Take all water in the bin.
               evaporated_water               += bin_water;
               transpiration_demand           -= bin_water;
               domain->surface_front_theta[ii] = domain->surface_front_theta[0];
               domain->surface_front_depth[ii] = domain->top_depth;
             }
         }
       else if (epsilonGreater(domain->surface_front_theta[ii], domain->surface_front_theta[0]) && domain->surface_front_depth[ii] <= root_depth)
         { // There is water in surface front bin, and water depth is less than root depth.
           if (domain->surface_front_theta[ii] - domain->surface_front_theta[ii - 1] > delta_theta)
             { // Water in the bin is wide enough for root uptake by delta_theta.
               if (ii == domain->parameters->num_bins || domain->surface_front_theta[ii + 1] <= domain->surface_front_theta[0])
                 { // Last surface front.
                   domain->surface_front_theta[ii] -= delta_theta;
                   evaporated_water                += delta_theta * (domain->surface_front_depth[ii] - domain->top_depth);
                   transpiration_demand            -= delta_theta * (domain->surface_front_depth[ii] - domain->top_depth);
                 }
               else
                 { // Not last surface front.
                   domain->surface_front_theta[ii] -= delta_theta;
                   evaporated_water                += delta_theta * (domain->surface_front_depth[ii] - domain->surface_front_depth[ii + 1]);
                   transpiration_demand            -= delta_theta * (domain->surface_front_depth[ii] - domain->surface_front_depth[ii + 1]);
                 }
             }
           else
             { // Water in the bin is not wide enough for root uptake by delta_theta.
               if (ii == domain->parameters->num_bins || domain->surface_front_theta[ii + 1] <= domain->surface_front_theta[0])
                 { // Last surface front, take all the water.
                   evaporated_water                += (domain->surface_front_theta[ii] - domain->surface_front_theta[ii - 1]) * 
                                                      (domain->surface_front_depth[ii] - domain->top_depth);
                   transpiration_demand            -= (domain->surface_front_theta[ii] - domain->surface_front_theta[ii - 1]) * 
                                                      (domain->surface_front_depth[ii] - domain->top_depth);
                   domain->surface_front_theta[ii]  = domain->surface_front_theta[0];
                   domain->surface_front_depth[ii]  = domain->top_depth;
                 } 
               else
                 { // Not last surfcace fornt, decrease depth to right bin.
                   evaporated_water                += (domain->surface_front_theta[ii] - domain->surface_front_theta[ii - 1]) * 
                                                      (domain->surface_front_depth[ii] - domain->surface_front_depth[ii + 1]);
                   transpiration_demand            -= (domain->surface_front_theta[ii] - domain->surface_front_theta[ii - 1]) * 
                                                      (domain->surface_front_depth[ii] - domain->surface_front_depth[ii + 1]);
                   domain->surface_front_depth[ii]  = domain->surface_front_depth[ii + 1];
                 }
             }
         }
     }// End of step 3, root zone transpiration from surface front water.
  
  // Step 4, satisfy transpiration demand from fully saturated domain, decrease delta_theta from theta[0].
  // FIXME If there is not surface front, and root depth is less then domain->groundwater_front_depth[1], not considered here.
  if (domain->surface_front_theta[0] > domain->parameters->theta_r && root_depth >= evaporate_depth)
    {
      if (domain->surface_front_theta[0] - domain->parameters->theta_r > delta_theta)
        {
          domain->surface_front_theta[0]    -= delta_theta;
          evaporated_water                  += delta_theta * (evaporate_depth - domain->surface_front_depth[1]);
          transpiration_demand              -= delta_theta * (evaporate_depth - domain->surface_front_depth[1]);
        }
      else
        {
          evaporated_water                  += (domain->surface_front_theta[0] - domain->parameters->theta_r) *
                                               (evaporate_depth - domain->surface_front_depth[1]);
          transpiration_demand              -= (domain->surface_front_theta[0] - domain->parameters->theta_r) * 
                                               (evaporate_depth - domain->surface_front_depth[1]);
          domain->surface_front_theta[0]     = domain->parameters->theta_r;
        }
      
      // Update empty bins after changing theta[0]. 
      for (ii = 1; ii <= domain->parameters->num_bins; ii++)
         {
           if (domain->surface_front_theta[ii]     <= domain->groundwater_front_theta[0])
             {
               domain->surface_front_theta[ii]      = domain->surface_front_theta[0];
             }
           
           if (domain->groundwater_front_theta[ii] <= domain->groundwater_front_theta[0])
             {
               domain->groundwater_front_theta[ii]  = domain->surface_front_theta[0];
             }
         }

      if (domain->groundwater_front_theta[0] >= domain->parameters->theta_s)
        {
          domain->groundwater_front_theta[1] = domain->parameters->theta_s;
          domain->groundwater_front_depth[1] = domain->bottom_depth;
        }

      domain->groundwater_front_theta[0] = domain->surface_front_theta[0];    
    } // End of step 4, root zone transpiration from fully saturated domain.
   
  // Step 5, satisfy transpiration demand from groundwater front bins within root zone depth.
  if (domain->yes_groundwater)
    {
      for (ii = 1; ii <= domain->parameters->num_bins; ii++)
         {  
           if (ii < domain->parameters->num_bins  &&  domain->groundwater_front_depth[ii + 1] < root_depth &&
               epsilonGreater(domain->groundwater_front_theta[ii + 1], domain->groundwater_front_theta[0]))
             { // Groundwater front not the last one, within the root zone (its right bin groundwater front less than root depth).
               if (domain->groundwater_front_theta[ii] - domain->groundwater_front_theta[ii - 1] > delta_theta)
                 { // Water in the bin is wide enough for root uptake by delta_theta.
                   domain->groundwater_front_theta[ii] -= delta_theta;
                   evaporated_water                    += delta_theta * (domain->groundwater_front_depth[ii + 1] - domain->groundwater_front_depth[ii]);
                   transpiration_demand                -= delta_theta * (domain->groundwater_front_depth[ii + 1] - domain->groundwater_front_depth[ii]);
                 }
               else
                 { // Water in the bin is not wide enough for root uptake by delta_theta.
                   evaporated_water                    += (domain->groundwater_front_theta[ii] - domain->groundwater_front_theta[ii - 1]) *
                                                          (domain->groundwater_front_depth[ii + 1] - domain->groundwater_front_depth[ii]);
                   transpiration_demand                -= (domain->groundwater_front_theta[ii] - domain->groundwater_front_theta[ii - 1]) *
                                                          (domain->groundwater_front_depth[ii + 1] - domain->groundwater_front_depth[ii]);
                   domain->groundwater_front_depth[ii]  = domain->groundwater_front_depth[ii + 1];
                 }
             }
           else if (epsilonGreater(domain->groundwater_front_theta[ii], domain->groundwater_front_theta[0]) &&
                    (domain->groundwater_front_theta[ii + 1] <= domain->groundwater_front_theta[0] || ii == domain->parameters->num_bins) &&
                    domain->groundwater_front_depth[ii]     <  root_depth)
             { // Last groundwater front, within root zone.
               if (root_depth >= domain->bottom_depth)
                 { // Do nothing.
                   // FIXME, later, root depth is deeper than domain depth, do nothig for the last bin to keep its theta = theta_s.
                   // Following code reduce theta for the last bin.
                   /*
                   if (domain->groundwater_front_theta[ii] - domain->groundwater_front_theta[ii - 1] > delta_theta)
                     { // Water in the bin is wide enough for root uptake by delta_theta.
                       
                       domain->groundwater_front_theta[ii] -= delta_theta;
                       evaporated_water                    += delta_theta * (root_depth - domain->groundwater_front_depth[ii]);
                       transpiration_demand                -= delta_theta * (root_depth - domain->groundwater_front_depth[ii]);
                       
                     }
                   else
                    { // Water in the bin is not wide enough for root uptake by delta_theta, remove all the water in the bin.
                      evaporated_water                    += (domain->groundwater_front_theta[ii] - domain->groundwater_front_theta[ii - 1]) *
                                                             (root_depth - domain->groundwater_front_depth[ii]);
                      transpiration_demand                -= (domain->groundwater_front_theta[ii] - domain->groundwater_front_theta[ii - 1]) *
                                                             (root_depth - domain->groundwater_front_depth[ii]);
                      domain->groundwater_front_depth[ii]  = domain->bottom_depth;
                      domain->groundwater_front_theta[ii]  = domain->groundwater_front_theta[0];
                    }
                   */ 
                 }
               else
                 { // root_depth < domain->bottom_depth.
                   if (domain->groundwater_front_theta[ii] - domain->groundwater_front_theta[ii - 1] > delta_theta)
                     {  
                       if (ii < domain->parameters->num_bins)
                         { // Create a bin to the right.
                           domain->groundwater_front_theta[ii + 1] = domain->groundwater_front_theta[ii];
                           domain->groundwater_front_depth[ii + 1] = root_depth;
                           domain->groundwater_front_theta[ii] -= delta_theta;
                           evaporated_water                    += delta_theta * (root_depth - domain->groundwater_front_depth[ii]);
                           transpiration_demand                -= delta_theta * (root_depth - domain->groundwater_front_depth[ii]);
                           break;
                         }
                       else if (ii == domain->parameters->num_bins)
                         {
                           // Do nothing.
                         } 
                     }
                   else
                     { 
                       evaporated_water                    += (domain->groundwater_front_theta[ii] - domain->groundwater_front_theta[ii - 1]) *
                                                              (root_depth - domain->groundwater_front_depth[ii]);
                       transpiration_demand                -= (domain->groundwater_front_theta[ii] - domain->groundwater_front_theta[ii - 1]) *
                                                              (root_depth - domain->groundwater_front_depth[ii]);
                       domain->groundwater_front_depth[ii]  = root_depth;
                     }
                 }
             }
         } // End of groundwater front bin loop.
    } // End of step5, root zone transpiration from groundwater front water.

  return evaporated_water;
}// End of garto_evapotranspiration().

void garto_domain_water_content(garto_domain* domain, double* water_content, double* soil_depth_z, int num_elements)
{
  int    ii, jj;                                            // Loop counter.
  int    jj_start     = 0;                                  // Loop start index for loop over soil_depth_z.
  double water_amount = 0.0;                                // Amount of water in each layer in meter.
  double base_theta   = domain->groundwater_front_theta[0]; // When processing groundwater this is the water content that is full over the entire height of a
                                                            // soil layer.  Accumulated water in the soil layer needs to be added to this water content to find
                                                            // the average water content over the layer.  This variable is only used for groundwater because
                                                            // surfacewater is processed right to left so the value is always the bin one to the left of the
                                                            // final bin processed for that layer, which is known when the value is needed.  Groundwater,
                                                            // however, is processed left to right so the value is the bin one to the left of the first bin
                                                            // processed so we need to remember it.
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  CkAssert(NULL != domain);
#endif
  // Do a simultaneous walk down the surfacewater bins in ii and soil_depth_z layers in jj_start and jj.
  for (ii = domain->parameters->num_bins; ii >= 1; ii--)
     {
       if (epsilonGreater(domain->surface_front_theta[ii], domain->surface_front_theta[0]))
         {
           for (jj = jj_start; jj < num_elements; jj++)
              {
                if (soil_depth_z[jj] <= domain->surface_front_depth[ii])
                  {
                    if (0 == jj)
                      {
                        water_amount     += (domain->surface_front_theta[ii] - domain->surface_front_theta[ii - 1]) * 
                                            (soil_depth_z[jj] - domain->top_depth);
                        water_content[jj] = domain->surface_front_theta[ii - 1] + water_amount / (soil_depth_z[jj] - domain->top_depth);
                      }
                    else 
                      {
                        water_amount     += (domain->surface_front_theta[ii] - domain->surface_front_theta[ii - 1]) * 
                                            (soil_depth_z[jj] - soil_depth_z[jj - 1]);
                        water_content[jj] = domain->surface_front_theta[ii - 1] + water_amount / (soil_depth_z[jj] - soil_depth_z[jj - 1]);          
                      }
                    water_amount          = 0.0;
                  }
                else
                  {
                    if (0 == jj)
                      {
                        water_amount += (domain->surface_front_theta[ii] - domain->surface_front_theta[ii - 1]) * 
                                        (domain->surface_front_depth[ii] - domain->top_depth);
                      }
                    else 
                      {
                        water_amount += (domain->surface_front_theta[ii] - domain->surface_front_theta[ii - 1]) * 
                                        (domain->surface_front_depth[ii] - soil_depth_z[jj - 1]);
                      }
                    jj_start          = jj;
                    break;
                  }
              }
         }
     }
  
  if (domain->yes_groundwater)
    { // TRUE = domain->yes_groundwater.  Do a simultaneous walk down the groundwater bins in ii and soil_depth_z layers in jj_start and jj.
      for (ii = 1; ii <= domain->parameters->num_bins; ii++)
         {
           if (epsilonGreater(domain->groundwater_front_theta[ii], domain->groundwater_front_theta[0]))
             {
               for (jj = jj_start; jj < num_elements; jj++)
                  {
                    if (soil_depth_z[jj] <= domain->groundwater_front_depth[ii])
                      {
                        // Groundwater bin ii is below the bottom of soil layer jj.  That means the soil layer can't intersect any more groundwater so it is
                        // done and its water content can be calculated.
                        if (0 == jj)
                          {
                            water_content[jj] = base_theta + water_amount / (soil_depth_z[jj] - domain->top_depth);
                          }
                        else
                          {
                            water_content[jj] = base_theta + water_amount / (soil_depth_z[jj] - soil_depth_z[jj - 1]);
                          }
                        water_amount = 0.0;
                        base_theta   = domain->groundwater_front_theta[ii - 1];
                      }
                    else
                      {
                        // Groundwater bin ii is above the bottom of soil layer jj.  Accumulate the water content of groundwater bin ii in the soil layer.
                        // All soil layers above this one are done, but this one might intersect additional groundwater bins to the right so set it to start
                        // with this soil layer and go on to the next groundwater bin
                        water_amount         += (domain->groundwater_front_theta[ii] - domain->groundwater_front_theta[ii - 1]) * 
                                                (soil_depth_z[jj] - domain->groundwater_front_depth[ii]);
                        jj_start              = jj;
                        break;
                      }
                  }
             }
         }
      for (jj = jj_start; jj < num_elements; jj++)
         {
           water_content[jj] = domain->parameters->theta_s;
         }
    }
  else
    { // FALSE = domain->yes_groundwater, no groundwater front, set water content to saturated bin.
      for (jj = jj_start; jj < num_elements; jj++)
         {
           if (0 == jj)
             {
               water_content[jj] = domain->surface_front_theta[0] + water_amount / (soil_depth_z[jj] - domain->top_depth);
             }
           else
             {
               water_content[jj] = domain->surface_front_theta[0] + water_amount / (soil_depth_z[jj] - soil_depth_z[jj - 1]);
             }
           water_amount = 0.0;
         }
    }
} // End of get_gar_domain_water_content().

double garto_equilibrium_water_content(garto_domain* domain, double depth_above_water_table)
{
  double equilibrium_water_content;
  if (depth_above_water_table  <= domain->parameters->bc_psib)
    { // Below air entry head, saturated.
      equilibrium_water_content = domain->parameters->theta_s;
    }
  else
    { // Using Brooks-Corey retention curve.
      equilibrium_water_content = domain->parameters->theta_r + (domain->parameters->theta_s - domain->parameters->theta_r) * 
                                  pow(depth_above_water_table / domain->parameters->bc_psib,  -domain->parameters->bc_lambda);
    }
  
  return equilibrium_water_content;
}
