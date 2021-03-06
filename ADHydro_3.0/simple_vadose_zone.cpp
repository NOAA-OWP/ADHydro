#include "simple_vadose_zone.h"

bool SimpleVadoseZone::checkInvariant() const
{
    bool error = false; // Error flag.
    
    if (!(0.0 < thickness))
    {
        ADHYDRO_ERROR("ERROR in SimpleGroundwater::checkInvariant: thickness must be greater than zero.\n");
        error = true;
    }
    
    if (!(0.0 < conductivity))
    {
        ADHYDRO_ERROR("ERROR in SimpleGroundwater::checkInvariant: conductivity must be greater than zero.\n");
        error = true;
    }
    
    if (!(0.0 < porosity && porosity <= 1.0))
    {
        ADHYDRO_ERROR("ERROR in SimpleGroundwater::checkInvariant: porosity must be greater than zero and less than or equal to one.\n");
        error = true;
    }
    
    if (!(0.0 <= psiB))
    {
        ADHYDRO_ERROR("ERROR in SimpleGroundwater::checkInvariant: psiB must be greater than or equal to zero.\n");
        error = true;
    }
    
    if (!(0.0 <= water && water <= porosity * thickness))
    {
        ADHYDRO_ERROR("ERROR in SimpleGroundwater::checkInvariant: water must be greater than or equal to zero and less than or equal to porosity * thickness.\n");
        error = true;
    }
    
    return error;
}

bool SimpleVadoseZone::doTimestep(double& groundwaterRecharge, double& surfaceWater, double waterTableDepth, double dt)
{
    bool   error = false;    // Error flag.
    double newRecharge;      // (m) Water that will be added to or removed from groundwaterRecharge.
    double equilibriumWater; // (m) The quantity of water the layer would have if the capillary fringe were in hydrostatic equilibrium with waterTableDepth.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        if (!(0.0 <= surfaceWater))
        {
            ADHYDRO_ERROR("ERROR in SimpleGroundwater::doTimestep: surfaceWater must be greater than or equal to zero.\n");
            error = true;
        }
        
        if (!(0.0 <= waterTableDepth))
        {
            ADHYDRO_ERROR("ERROR in SimpleGroundwater::doTimestep: waterTableDepth must be greater than or equal to zero.\n");
            error = true;
        }
        
        if (!(0.0 < dt))
        {
            ADHYDRO_ERROR("ERROR in SimpleGroundwater::doTimestep: dt must be greater than zero.\n");
            error = true;
        }
    }
    
    if (!error)
    {
        // Do infiltration.
        if (conductivity * dt < surfaceWater)
        {
            newRecharge   = conductivity * dt;
            surfaceWater -= conductivity * dt;
        }
        else
        {
            newRecharge   = surfaceWater;
            surfaceWater  = 0.0;
        }
        
        // Do vadoseZone.
        equilibriumWater = waterFromSaturationDepth(std::max(waterTableDepth - psiB, 0.0));
        
        if (water > equilibriumWater && water - equilibriumWater > conductivity * dt - newRecharge)
        {
                // Water movement is limited by conductivity.
                water       -= conductivity * dt - newRecharge;
                newRecharge  = conductivity * dt;
        }
        else if (water < equilibriumWater && equilibriumWater - water > conductivity * dt + newRecharge)
        {
                // Water movement is limited by conductivity.
                water       +=  conductivity * dt + newRecharge;
                newRecharge  = -conductivity * dt;
        }
        else
        {
                // Water will reach equilibrium.
                newRecharge -= equilibriumWater - water;
                water        = equilibriumWater;
        }
        
        groundwaterRecharge += newRecharge;
    }
    
    return error;
}

void SimpleVadoseZone::addOrRemoveWater(double& recharge, double& waterCreated)
{
    if (0.0 < recharge)
    {
        if (water + recharge <= porosity * thickness)
        {
            // All of the water will fit.
            water   += recharge;
            recharge = 0.0;
        }
        else
        {
            // Not all of the water will fit.
            recharge = (water + recharge) - (porosity * thickness); // Grouping is designed to prevent recharge from going negative due to roundoff error.
            water    = porosity * thickness;
        }
    }
    else if (0.0 > recharge)
    {
        if (water + recharge >= 0.0)
        {
            // There is enough water to cover recharge.
            water   += recharge;
            recharge = 0.0;
        }
        else
        {
            // There is not enough water to cover recharge.
            waterCreated -= water + recharge; // Subtract a negative water deficit to make positive created water.
            water         = 0.0;
            recharge      = 0.0;
        }
    }
}

double SimpleVadoseZone::waterContentAtDepth(double depth) const
{
    double waterContent;                                      // (m^3/m^3) Return value.
    double saturationDepth = saturationDepthFromWater(water); // (m) The current saturation depth.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        if (!(0.0 <= depth && depth <= thickness))
        {
            ADHYDRO_ERROR("ERROR in SimpleGroundwater::waterContentAtDepth: depth must be greater than or equal to zero and less than or equal to thickness.\n");
            ADHYDRO_EXIT(-1);
        }
    }
    
    if (depth >= saturationDepth)
    {
        waterContent = porosity;
    }
    else
    {
        waterContent = porosity * (depth / saturationDepth); // Grouping is designed to guarantee multiplication by a fraction less than one
                                                             // to prevent roundoff error from returning more than porosity.
    }
    
    return waterContent;
}

double SimpleVadoseZone::waterAboveDepth(double depth) const
{
    double waterAbove;                                        // (m) Return value.
    double saturationDepth = saturationDepthFromWater(water); // (m) The current saturation depth.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        if (!(0.0 <= depth && depth <= thickness))
        {
            ADHYDRO_ERROR("ERROR in SimpleGroundwater::waterAboveDepth: depth must be greater than or equal to zero and less than or equal to thickness.\n");
            ADHYDRO_EXIT(-1);
        }
    }
    
    if (depth >= saturationDepth)
    {
        waterAbove = water - (thickness - depth) * porosity;
    }
    else
    {
        waterAbove = 0.5 * waterContentAtDepth(depth) * depth;
    }
    
    return waterAbove;
}

double SimpleVadoseZone::specificYield(double waterTableDepth) const
{
    double yield;                                    // return value.
    double saturationDepth = waterTableDepth - psiB; // (m) The shallowest depth from the top of the modeled layer that would be saturated if the soil
                                                     // moisture were in equilibrium with waterTableDepth.  No need to protect against a negative value.
    
    if (saturationDepth <= thickness)
    {
        yield = 0.5 * porosity;
    }
    else // (saturationDepth > thickness)
    {
        yield = std::max(0.5 * thickness * thickness / (saturationDepth * saturationDepth), 0.01) * porosity;
    }
    
    return yield;
}

double SimpleVadoseZone::waterFromSaturationDepth(double saturationDepth) const
{
    double waterQuantity; // (m) Return value.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
    {
        ADHYDRO_ASSERT(0.0 <= saturationDepth);
    }
    
    if (saturationDepth < thickness)
    {
        waterQuantity = porosity * (thickness - (0.5 * saturationDepth));
    }
    else // (saturationDepth >= thickness)
    {
        waterQuantity = 0.5 * porosity * thickness * (thickness / saturationDepth); // Grouping is designed to guarantee multiplication by a fraction less than or equal to one
                                                                                    // to prevent roundoff error from returning more than 0.5 * porosity * thickness.
    }
    
    return waterQuantity;
}

double SimpleVadoseZone::saturationDepthFromWater(double waterQuantity) const
{
    double saturationDepth; // (m) Return value.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
    {
        ADHYDRO_ASSERT(0.0 <= waterQuantity && waterQuantity <= porosity * thickness);
    }
    
    if (waterQuantity > 0.5 * porosity * thickness) // (saturationDepth < thickness)
    {
        saturationDepth = (porosity * thickness - waterQuantity) / (0.5 * porosity); // Grouping is designed to prevent roundoff error from returning a negative value.
    }
    else // (saturationDepth >= thickness)
    {
        saturationDepth = thickness * ((0.5 * porosity * thickness) / waterQuantity); // Grouping is designed to guarantee multiplication by a number greater than or equal to one
                                                                                      // to prevent roundoff error from returning less than thickness.
    }
    
    return saturationDepth;
}
