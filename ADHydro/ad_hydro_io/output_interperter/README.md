ADHydro Output Options

In general, output from distributed-parameter hydrological modeling is a time series.   These time series may consist of an absolute time expressed as a double precision Julian date (%16.8lf) followed by instantaneous scalars at specific points, temporal averages at specific points, areal averages within specified polygons. 

The objective of the ADHydro output module is maximum flexibility in terms of the ability of the user to specify output. This way, the code does not require modification to change the output. Any state variable or computed flux should be writeable anywhere, at any time, on a conditional basis. The conditionals would include:

1. Geographical region, inside a polygon. <Px1,Py2,Px2,Py2,….,PxN,PyN>, specified in a clockwise direction, so that “inside” the polygon is to the right of the direction of drawing.
2. Time period <Julian date beginning, Julian date end>
3. When a state variable or calculated flux value includes a specified threshold.  
4. Within a specified distance of channels.
5. Within channels only (hydrographs).

	5a. At confluences involving channels greater than a specified order. Any time two channels of order N or greater join, write their flow time series, either instantaneous or time-averaged discharge, at specified temporal resolution (5 minutes, hourly, daily).

	5b. At specific locations on channels (i.e. stream gauge locations). The highest stream order near a particular geographical coordinate, within some distance in meters (e.g. 200).

Conditionals 1-4 shall be additive. All of them should be changeable during a simulation, necessitating a dynamic output specification. We will want to add some internal functions to create output conditionals automatically during a simulation. For example, when the product of velocity and depth in an overland flow exceeds 0.25 m2/s.

We want to have the ability to output either instantaneous or cumulative state variables or fluxes. In reality, cumulative fluxes will be more useful than cumulative state variables.

The output routine shall be optimized to minimize the impact of output on run times.

Some interface will be required? Some list of state variables and fluxes that user picks/chooses from?  

Map output viewable with paraview. How should we view time-series output?

Shall we create an entirely new output file for this?
