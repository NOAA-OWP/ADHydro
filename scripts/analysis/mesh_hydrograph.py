#!/usr/bin/env python
"""
Note, on Mt Moran, need to load 
"""

import os
import netCDF4
import pandas as pd
import matplotlib

import matplotlib.pyplot as plot
import argparse 
#import sys
#input_dir = sys.argv[1]

parser = argparse.ArgumentParser(description="Generate a hydrograph from a given channel element and ADHydro simulation output.")
parser.add_argument('inputDirectory', help='Directory containing ADHydro simulation output, specifically state.nc.  By default, outputs will also be saved in this directory.')
parser.add_argument('-o','--output', help='Optional output directory.')
parser.add_argument('-r','--reservoirs', help='A list of ADHydro reservoir link numbers to produce a volume change and hydrograph plot for.', nargs='+', type=int, default=[])
parser.add_argument('-c','--channels', help='A list of ADHydro channel link numbers to produce a hydrograph for.', nargs='+', type=int, default=[])

args = parser.parse_args()

input_dir = os.path.abspath(args.inputDirectory)
output_dir = os.path.abspath(args.output) if args.output else input_dir

geometry = os.path.join(input_dir, 'geometry.nc')
state = os.path.join(input_dir, 'state.nc')
parameter = os.path.join(input_dir, 'parameter.nc')

down_stream = 'channelChannelNeighborsDownstream'
neighbors = 'channelChannelNeighbors'
flow = 'channelSurfacewaterChannelNeighborsFlowRate'
length = 'channelElementLength'
width = 'channelBaseWidth'
sSlope = 'channelSideSlope'
depth = 'channelSurfacewaterDepth'

state_ncf = netCDF4.Dataset(state)
parameter_ncf = netCDF4.Dataset(parameter)
geometry_ncf = netCDF4.Dataset(geometry)

flow_ncf = state_ncf.variables[flow]
flow_panel = pd.Panel(flow_ncf[:])

def neighborFlow(s):
    #s should have only one valid link number??????
    #Should only be one neighbor downstream...
    #Look up flow to that element.
    neighbor_index = s.name
    link = s.index[0]
    #print "HERE"
    #print neighbor_index
    #print link
    #neighbor_index now contains the index into the flow table that
    #defines the flow from the link to the  neighbor 
    #print "FLOW from {} to {} for each instance:".format(s.index[0], s.iloc[0])
    flow = [flow_panel[x].loc[link].loc[neighbor_index] for x in flow_panel]
    #print flow
    return pd.Series(flow, name = link)

def graphs(s,link,out):
    if out:
        if s.name == None:
            title = "Flow out of {}".format(link)
        else:
            title = "Flow from {} to {}".format(link, s.name)
    else:
        title = "Flow from {} to {}".format(s.name, link)
    #print title
    #print s
    fig = plot.figure()
    ax = s.plot()
    ax.set_xlabel('Instance')
    ax.set_ylabel('Flow ($m^3/s$)')
    ax.set_title(title)
    plot.savefig(os.path.join(output_dir, '{}_hydrograph.pdf'.format(link)))

def oldhydrographIN(link):
    ds = geometry_ncf.variables[down_stream]
    n = geometry_ncf.variables[neighbors]

    neighbor_df = pd.DataFrame(n[0]).replace(-1, pd.np.nan)
    down_stream_df = pd.DataFrame(ds[0])
    #Mask for UPSTREAM neighbors
    up_stream_mask = (down_stream_df==0)
    #Mask for DOWNSTREAM neighbors
    down_stream_mask = (down_stream_df==1)
    #Get the set of upstream and downstream neighbors
    upstream_df = neighbor_df[up_stream_mask]
    downstream_df = neighbor_df[down_stream_mask]

    upstream_neighbors = upstream_df.loc[link].dropna()
    #print "UPSTREAM: "
    #print upstream_neighbors
    #print "DOWNSTREAM:"
    downstream_neighbors = downstream_df.loc[link].dropna()
    #print downstream_neighbors
    #print "NEIGHBORS OF UPSTREAM:"
    df = neighbor_df.loc[upstream_neighbors]
    #Slice upstream neighbors neighbor list to get only links to link
    df = df[ df == link ]
    #print df
    #Figure out the flow from each neighbor to link
    #print flow_panel
    flows = df.apply(neighborFlow, axis=1)
    flows.apply(graphs,axis=1, args=(link,False))
    #print "FLOW to {} from all neighbors:".format(link)
    #print flows
    #print "Cummulative Flow:"
    #print flows.sum()

def hydrographOUT(link, plot=False):
    ds = geometry_ncf.variables[down_stream]
    n = geometry_ncf.variables[neighbors]
    myNeighbors = pd.Series(n[0][link], name=link).replace(-1, pd.np.nan).dropna()
    myDown_stream = pd.Series(ds[0][link], name=link)
    #print myDown_stream
    #Mask for DOWNSTREAM neighbors
    down_stream_mask = (myDown_stream == 1)
    #print down_stream_mask
    #FOR TESTING#down_stream_mask.loc[3] = True
    #Get the set of upstream and downstream neighbors
    myDown_stream = pd.DataFrame(myNeighbors[down_stream_mask])
    #print "DOWNSTREAM:"
    #print myDown_stream
    #Figure out the flow from link to each neighbor
    #print flow_panel
    flows = myDown_stream.apply(neighborFlow, axis=1)
    flows['toLink'] = myDown_stream.apply(lambda x: myNeighbors.loc[x.index]).astype(pd.np.int)
    flows.set_index('toLink', inplace=True)
    #print "OUTFLOWS\n", flows
    #if plot:
    #    flows.apply(graphs,axis=1, args=(link,True))
    print "FLOW from {} to all neighbors:".format(link)
    print flows
    print "Cummulative Flow:"
    print flows.sum()
    if plot:
        graphs(flows.sum(), link, True)
    return flows.sum()

def fromLink(s, neighbors):
    print neighbors
    print neighbors.loc[s.index]

def hydrographIN(link):
    ds = geometry_ncf.variables[down_stream]
    n = geometry_ncf.variables[neighbors]

    myNeighbors = pd.Series(n[0][link], name=link).replace(-1, pd.np.nan).dropna()
    myUp_stream = pd.Series(ds[0][link], name=link)
    #Mask for UPSTREAM neighbors
    up_stream_mask = (myUp_stream==0)
    #Get the set of upstream neighbors
    myUp_stream = pd.Series(myNeighbors[up_stream_mask])
    print myUp_stream.values
    usNN = pd.DataFrame([pd.Series(n[0][int(l)], name = int(l)) for l in myUp_stream.values])
    #TODO/FIXME messed up around here somewhere
    print usNN
    usNN = usNN [ usNN == link ]
    df = pd.DataFrame()
    for name, data in usNN.T.iteritems():
        print name
        data = data.dropna()
        s = pd.DataFrame([data.index[0]], index = [data.index[0]], columns = [int(name)])
        print s
        df = df.append(s)
        
    print df
    #Figure out the flow from each neighbor to link
    #print flow_panel
    flows = df.apply(neighborFlow, axis=1)
    print flows
    flows['fromLink'] = flows.apply(lambda x: myNeighbors.loc[x.name], axis = 1).astype(pd.np.int)
    print flows
    flows.set_index('fromLink', inplace=True)
    #flows.apply(graphs,axis=1, args=(link,False))
    print "FLOW to {} from all neighbors:".format(link)
    print flows
    print "Cummulative Flow:"
    print flows.sum()
     


def resVolume(link):
    l = geometry_ncf.variables[length][0][link]
    #print l
    w = parameter_ncf.variables[width][0][link]
    #print w
    s = parameter_ncf.variables[sSlope][0][link]
    #print s
    #TODO/FIXME incorporate side slope, though for it is always 0 for reservoirs
    #Get depth at each instance for the link of interest.  Must transpose since NCD4 stores instances on major axis then elementNumbers
    depth_series = pd.DataFrame(state_ncf.variables[depth][:]).T.loc[link] #TODO SLICE NETCDF4 data for link, avoid reading all depths
    volume = depth_series*l*w
    
    fig = plot.figure()
    fig.subplots_adjust(hspace=0.5 )
    ax1 = fig.add_subplot(211)
    ax2 = fig.add_subplot(212)
    ax1.set_title("Volume of reservoir {}".format(link))
    ax1.set_ylabel("$m^3$")
    ax1.set_xlabel("Instance")

    ax2.set_title("Flow out of {}".format(link))
    ax2.set_ylabel("$m^3/s$")
    ax2.set_xlabel("Instance")
    out = hydrographOUT(link, plot=False)
 
    out = out.replace(0, pd.np.nan).dropna()
    volume[volume <= 0.000001] = pd.np.nan
    volume = volume.dropna()
    volume.plot(ax=ax1)
    out.plot(ax=ax2)
    plot.savefig(os.path.join(output_dir, 'res_{}_volume.pdf'.format(link)))
    out.to_csv(os.path.join(output_dir, '{}_release.csv'.format(link)))
    volume.to_csv(os.path.join(output_dir, '{}_volume.csv'.format(link)))
    
if __name__=='__main__':
    
    if not args.reservoirs and not args.channels:
        parser.error('No links provided, please provide either channel links with -c <list> and/or reservoir links with -r <list>')
    #TODO add plot flag to command line
    #link1 = 67
    #link2 = 743
    #resVolume(67)
    #resVolume(204)
    """
    import sys
    res = False
    if sys.argv[2] == '-r':
        res = True
    start = 2
    if res:
        start = 3
    for l in sys.argv[start:]:
        print "Creating Hydrograph for {}".format(l)
        if res:
                resVolume(int(l))
        else:
            hydrographOUT(int(l),True)
    """
    for r in args.reservoirs:
        print "Creating Hydrograph and Volume change plot for {}".format(r)
        resVolume(r)

    for c in args.channels:
        print "Creating Hydrograph for {}".format(c)
        hydrographOUT(c, True)
    #hydrographOUT(744, True)
    #hydrographOUT(21026, True)
    #hydrographOUT(21179, True)
    #hydrographOUT(1145, True)
    plot.show()
