Notes

-   Snow on canopy and rain on canopy can be calculated from mass
    conservation with other quantities. In order to do this, the ADHydro
    team had to expose qrain as an output variable of noahmp_sflx, which
    was previously only an internal variable.
-   During testing I found one instance where the canopy completely
    emptied of water and the total outflow ((qsnow + qrain + ecan) \*
    dt) was greater than the water available for those flows (prcp \*
    dt + canliq + canice). It\'s possible that they meant for qsnow,
    qrain, and ecan to be the rates that occur at the beginning of the
    timestep not capped by the amount of water available, but the value
    of sneqv was increased by ((qsnow + qrain) \* dt) so there is an
    inconsistency somewhere. The canopy completely emptying should occur
    infrequently, at most once per storm event. In the one case where I
    have seen this it only created 0.1 micron of water.
-   If canLiq or canIce would be less than 1e-6 mm they are set to zero.
    This is not a bug per-se because it is done on purpose, but it is a
    mass balance violation where water is thrown away and not accounted
    for in any variable. To complicate matters, they are not set to zero
    at the end of the Noah-MP code. I found one instance where canLiq
    was set to zero, and then after that some of canIce was melted and
    added to canLiq so it returned a non-zero value for canLiq even
    though it had been set to zero for being too small.
-   If ist is 2 indicating a waterbody, and the ground temperature is
    above freezing, then any snow that reaches the ground is set to
    zero. It doesn\'t get melted and added to rainfall. It just gets
    thrown away. This is not a bug per-se because it is done on purpose,
    but it is a mass balance violation where water is thrown away and
    not accounted for in any variable.
-   There is a complicated relationship between the variables that
    represent the snow layer(s): sneqv, sneqvo, snowh, isnow, snice,
    snliq, zsnso, ficeold.
-   sneqv always equals the total water equivalent of all snow layers.
    For mass balance, just look at sneqv as the total water in the snow
    pack and ignore the other variables.
-   sneqvo is usually sneqv at the beginning of the last timestep with
    one exception. When the multi-layer snow simulation is turned off
    and snow is melting, the melt gets taken out of sneqv before its
    beginning-of-timestep value gets saved in sneqvo. The amount of melt
    is reported in ponding, so in this case sneqvo plus ponding is sneqv
    at the beginning of the last timestep
-   snowh always equals the total geometric height of all snow layers.
-   If sneqv would be less than 0.001 mm or snowh would be less than
    1e-6 m at the end of the timestep they are both set to zero. This is
    not a bug per-se because it is done on purpose, but it is a mass
    balance violation where water is thrown away and not accounted for
    in any variable.
-   If sneqv would be greater than 2000 mm at the end of the timestep it
    is capped at 2000 mm, and the excess is considered to be glacier
    flow that leaves the simulation domain. This glacier flow is added
    directly to runsub. It is not added to smc, qsnbot, ponding, or any
    other variable.
-   There is a multi-layer simulation that is only turned on when snowh
    gets larger than 2.5 cm.
-   When the multi-layer simulation is off, isnow is zero and snice,
    snliq, zsnso, and ficeold are ignored even if there is snow in sneqv
    and snowh.
-   When the multi-layer simulation is on, isnow is the number of layers
    as a negative number. That is, isnow is -1 for one layer, -2 for two
    layers, etc. It was done this way because isnow is used as an array
    index in the fortran code where negative indices are snow layers and
    non-negative indices are soil layers.
-   When the multi-layer simulation is on, snice, snliq, zsnso, and
    ficeold store the values of the layer(s), and the total of snice and
    snliq is consistent with sneqv, and zsnso is consistent with snowh.
-   ficeold, like sneqvo, stores the value from the beginning of the
    last timestep.
-   If the multi-layer snow simulation is turned on at the end of the
    timestep, then all of the rain below canopy becomes rain on snow.
    Otherwise, it all becomes rain on ground.
-   The evaporation for edir is taken first from snow up to the amount
    of snow present at the beginning of the timestep, and then the rest
    from the ground. For example, if sneqv is 0.05 at the beginning of
    the timestep, and edir \* dt is 0.06, and all other effects combine
    to add 0.1 to the snow layer(s) then sneqv will be 0.1 at the end of
    the timestep, evaporation from snow will be 0.05, and evaporation
    from ground will be 0.01. NOAHMP_GLACIER is different in this
    respect. It does precipitation before evaporation. I believe it also
    does snow melt before evaporation, although I haven\'t checked this.
-   The water flowing out of the bottom of the snow layer(s) including
    snowmelt and passed through rainfall is qsnbot \* dt + ponding +
    ponding1 + ponding2. Note, passed through rainfall comes only from
    rain on snow and does not include rain on ground. When the
    multi-layer simulation is on at the end of the timestep, all of the
    flow gets put in qsnbot. When the multi-layer simulation is off at
    the beginning and end of the timestep, all of the flow gets put in
    ponding. When the multi-layer simulation is on at the beginning of
    the timestep and off at the end of the timestep, sneqv takes on the
    total of snice, the water in snliq flows out of the bottom of the
    snow layer(s), and all of the flow gets put in ponding1 or ponding2
    depending on which of two code paths turns off the multi-layer
    simulation. If the water equivalent of snice gets too low the flow
    is put in ponding1. If the height of the snowpack gets too low the
    flow is put in ponding2.
-   There is a complicated relationship between the variables that
    represent saturated soil: wa, wt, zwt.
-   wa is the water in the aquifer. The aquifer is an extra layer below
    the soil layers that is hardcoded to be 25 meters thick and have a
    specific yield of 0.2. Therefore the total water capacity of the
    aquifer is 5000 mm. wa should never be larger than 5000. The water
    in wa is in addition to the water in the soil layers represented by
    smc.
-   wt is wa plus any water within the soil layers that is below the
    water table. smc gives only the average moisture content of each
    layer so wt is an extra variable to give more information about the
    position of the water table within a layer. wt is not extra water in
    addition to wa and smc. It is the sum of all of the water in wa plus
    possibly some of the water in smc. When doing mass balance
    calculations just add wa to smc and ignore wt.
-   zwt is the depth to the water table and must be consistent with wa
    and wt.
-   If the water table is below the bottom of the lowest soil layer then
    wa will be some value between 0 and 5000. wt will be the same as wa.
    zwt will be the depth to the bottom of the lowest soil layer plus
    the empty depth in the aquifer, which is (5000 -- wa) / 0.2.
-   If the water table is above the bottom of the lowest soil layer then
    wa will be 5000. wt will be some value greater than or equal to wa
    and less than or equal to wa + smc. zwt will be the depth to the
    bottom of the lowest soil layer minus the fully saturated depth in
    the soil layer(s) due to the water in wt, which is (wt -- wa) /
    (porosity, which is MAXSMC in the SOILPARM.TBL file).
-   Wslake is also water that is available for evapotranspiration. It
    should be added to smc and wa to calculate mass balance.
