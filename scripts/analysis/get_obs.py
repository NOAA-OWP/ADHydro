#!/usr/bin/env python
import requests
import pandas as pd
import os

stations = ["02146211",
            "0214627970",
            "02146285",
            "02146300",
            "02146315",
            "02146348",
            "02146381",
            "02146409",
            "0214642825",
            "0214643820",
            "0214645022",
            "0214645075",
            "0214645080",
            "02146470",
            "02146507",
            "02146530",
            "0214655255",
            "02146562",
            "0214657975",
            "02146600",
            "02146700",
            "02146750",
            "0214676115",
            "0214678175",
            "02146800"]

tz_map = {'EST':'EST'}

def get_nwis_data(station, start_date, end_date):
    service = 'iv' 
    parameter = '00060'
    header = {
        'Accept-encoding': 'gzip',
        'max-age': '120'
    }
    na = None
    values = {
        # specify version of nwis json. Based on WaterML1.1
        # json,1.1 works; json%2C works; json1.1 DOES NOT WORK
        'format': 'json,1.1',
        'sites': [station],
        'stateCd': na,
        'countyCd': na,
        'bBox': na,
        'parameterCd': parameter,
        'period': na,
        'startDT': start_date,
        'endDT': end_date
    }
    
    url = 'http://waterservices.usgs.gov/nwis/'
    url = url + service + '/?'
    response = requests.get(url, params=values, headers=header)
    series = response.json()['value']['timeSeries'][0]
    meta_data = series['sourceInfo']
    time_zone_info = meta_data['timeZoneInfo']
    tz_code = time_zone_info['defaultTimeZone']['zoneAbbreviation']
    #print(time_zone_info)
    data = series['values'][0]['value']
    df = pd.DataFrame(data).drop('qualifiers',axis=1)
    df['dateTime'] = pd.to_datetime(df['dateTime'], utc=True)
    df.set_index('dateTime', inplace=True)
    #print(df.index)
    #print(df.head())
    #df = df.tz_localize(tz_map[tz_code])
    #print(df.head())
    df = df.tz_convert('UTC')
    #print(df.head())
    #Convert from cfs to cms
    df['value'] = df['value'].astype(float)
    df['Discharge_cms'] = df['value'] * 0.028316846592   
    output = os.path.join('./{}'.format(df.index[0].year), 'usgs_{}_observed_cms'.format(station))
    df[['Discharge_cms']].to_csv(output)
#For time zone conversions, to ensure complete coverate, grab a day on either end of the simulation range
dates = ('2008-07-15', '2008-08-31')
dates = ('2011-07-29', '2011-08-09')
dates = ('2015-12-17', '2016-01-03')
for station in stations:
    try:
        get_nwis_data(station, *dates)
    except:
        print("Error for station: {}".format(station))
#data_string = 'USGS:{}:00060:00000'
#df = hf.NWIS(stations[0], 'iv', *dates).get_data().df()
#print(df[data_string.format(stations[0])].head())
