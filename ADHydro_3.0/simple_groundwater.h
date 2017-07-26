#ifndef __SIMPLE_GROUNDWATER_H__
#define __SIMPLE_GROUNDWATER_H__

#include "all.h"

// A SimpleGroundwater is an extremely low fidelity model of a groundwater capillary fringe.  In the model, there is a shallowest depth that is saturated.
// The model assumes there is a triangle of water above that depth and a rectangle of water below that depth.
//
//          0 |\          .
//            | \         .
//            |  \        .
//            |   \       .
//            |    \      .
//            |     \     . water = 1/2 * porosity * saturation depth
//            |      \    .
//            |       \   .
//            |        \  .
//            |         \ .
// saturation |          \.
//      depth |___________.
//            |           |
//            |           | water = porosity * (thickness - saturation depth)
//            |           |
//  thickness |-----------|
//            0        porosity
//
// The saturation depth can be below the bottom of the modeled layer, in which case there is only a triangle of water.
// In this case, the water content at the layer bottom will be less than saturated.
// A consequnce of this model is that water can only go to zero if saturation depth goes to infinity.
//
//          0 |\          .
//            | \         .
//            |  \        . water = 1/2 * water content at layer bottom * thickness
//            |   \       .
//            |    \      .
//  thickness |-----\     . water content at layer bottom = porosity * thickness / saturation depth
//            |      \    .
//            |       \   .
//            |        \  . This part of the triangle below the bottom of the modeled layer is not included in water in the layer.
//            |         \ .
// saturation |          \.
//      depth |___________.
//            0        porosity
//
// The model actually only stores the quantity of water in the layer and derives everything else.
// The capillary fringe is in hydrostatic equilibrium when the saturation depth is equal to the depth of the water table minus psiB.
// The timestep simulation moves the quantity of water towards equilibrium putting water in to or taking water from recharge.
// The timestep simulation also performs trivial infiltration moving water from the surface directly to recharge without simulating an infiltration front.
// The timestep simulation ensures that no flow exceeds conductivity either from the surface or to/from recharge.
class SimpleGroundwater
{
public:
    
    // Constructor.  All parameters directly initialize member variables except for:
    //
    // Parameters:
    //
    // waterTableDepth - (m) If this value is not NAN and is greater than or equal to zero, the water parameter is ignored, and the water member variable is initialized to be in
    //                   hydrostatic equilibrium with this water table.  Otherwise, this parameter is ignored and the water member variable is initialized to the water parameter.
    inline SimpleGroundwater(double thickness = 1.0, double conductivity = 1.0, double porosity = 1.0, double psiB = 0.0, double water = 0.0, double waterTableDepth = NAN) :
        thickness(thickness), conductivity(conductivity), porosity(porosity), psiB(psiB), water((0.0 <= waterTableDepth) ? waterFromSaturationDepth(std::max(waterTableDepth - psiB, 0.0)) : water)
    {
        if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
        {
            if (checkInvariant())
            {
                ADHYDRO_EXIT(-1);
            }
        }
    }
    
#ifdef __CHARMC__
    // Charm++ pack/unpack method.
    //
    // Parameters:
    //
    // p - Pack/unpack processing object.
    inline void pup(PUP::er &p)
    {
        p | thickness;
        p | conductivity;
        p | porosity;
        p | psiB;
        p | water;
    }
#endif // __CHARMC__
    
    // Check invariant conditions on data.
    //
    // Returns: true if the invariant is violated, false otherwise.
    bool checkInvariant() const;
    
    // Process one timestep of 1D infiltration and vadose zone water movement.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // groundwaterRecharge - (m) Temporary holding bucket for water that is moving to/from the vadose zone expressed as a thickness of water.  Can be negative for a water deficit.
    //                       This will be updated based on how much water infiltrates and moves within the vadose zone.  May be increased or decreased by up to conductivity * dt.
    // surfaceWater        - (m) Depth of ponded surface water.  This will be updated based on how much water infiltrates.  May be decreased by up to conductivity * dt.
    // waterTableDepth     - (m) Depth of the water table from the top of the modeled layer expressed as a positive number.
    // dt                  - (s) Duration of the timestep.
    bool doTimestep(double& groundwaterRecharge, double& surfaceWater, double waterTableDepth, double dt);
    
    // Add or remove recharge water to or from the water in the layer.
    // If recharge is positive, and it will all fit in water, add it all and set recharge to zero.  If recharge is positive, but not all will fit, add as much as can fit and set recharge to the excess.
    // If recharge is negative, remove it all from water and set recharge to zero.  There should always be enough water in the capillary fringe to cover a negative recharge.  If not, the timestep is too long.
    // If it ever happens that there is not enough water to cover a negative recharge, take all the water there is, set recharge to zero, and record the deficit in waterCreated.
    // At the end, recharge cannot be negative, and can only be positive if water is full.
    //
    // Parameters:
    //
    // recharge     - (m) A thickness of water that is left over in a recharge bucket after accounting for all flows.  Can be negative for a water deficit.  Will be updated for the amount of water moved.
    // waterCreated - (m) Water created or destroyed for exceptional and possibly erroneous circumstances.  Positive means water was created.  Negative means water was destroyed.
    //                Will be updated if water needs to be created to fill a negative recharge.
    void addOrRemoveWater(double& recharge, double& waterCreated);
    
    // Returns: true if the layer is completely full of water, false otherwise.
    inline bool isFull() const
    {
        return (porosity * thickness == water);
    }
    
    // Returns: (m/s) The conductivity at a given depth.  We assume that conductivity is linear with water content.  Exit on error.
    //
    // Parameters:
    //
    // depth - (m) The depth from the top of the modeled layer expressed as a positive number.
    inline double conductivityAtDepth(double depth) const
    {
        // Don't error check parameter because it's a simple pass-through to waterContentAtDepth and it will be checked inside that method.
        return conductivity * (waterContentAtDepth(depth) / porosity); // Grouping is designed to guarantee multiplication by a fraction less than or equal to one
                                                                       // to prevent roundoff error from returning more than conductivity.
    }
    
    // Returns: (m^3/m^3) The water content at a given depth.  Exit on error.
    //
    // Parameters:
    //
    // depth - (m) The depth from the top of the modeled layer expressed as a positive number.
    double waterContentAtDepth(double depth) const;
    
    // Returns: (m) The quantity of water above a given depth.  Exit on error.
    //
    // Parameters:
    //
    // depth - (m) The depth from the top of the modeled layer expressed as a positive number.
    double waterAboveDepth(double depth) const;
    
    // Returns: the value of thickness.
    inline double getThickness() const
    {
        return thickness;
    }
    
    // Returns: the value of conductivity.
    inline double getConductivity() const
    {
        return conductivity;
    }
    
    // Returns: the value of porosity.
    inline double getPorosity() const
    {
        return porosity;
    }
    
private:
    
    // Returns: (m) The quantity of water the modeled layer would have if the saturation depth were the given value.
    //
    // Parameters:
    //
    // saturationDepth - (m) The shallowest depth from the top of the modeled layer that is saturated expressed as a positive number.
    double waterFromSaturationDepth(double saturationDepth) const;
    
    // Returns: (m) The saturation depth the modeled layer would have expressed as a positive number if the quantity of water were the given value.
    //
    // Parameters:
    //
    // waterQuantity - (m) The qunatity of water in the layer.
    double saturationDepthFromWater(double waterQuantity) const;
    
    // Immutable attributes of the layer.
    double thickness;    // (m) The thickness of the modeled layer.
    double conductivity; // (m/s) The hydraulic conductivity of the matrix.
    double porosity;     // (m^3/m^3) The porosity of the matrix.
    double psiB;         // (m) The bubbling pressure of the matrix.
    
    // Mutable state of the layer.
    double water; // (m) The quantity of water in the layer.
};

#endif // __SIMPLE_GROUNDWATER_H__
