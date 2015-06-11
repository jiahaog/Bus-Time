
# use http://ogre.adc4gis.com to convert the data downloaded from
# http://www.mytransport.sg/content/mytransport/home/dataMall.html bus stop location SHP

# Target SRS: WGS84
# Somehow latitude and longitude are reversed in output, so we have to reverse it when reducing


import json
from pprint import pprint



REDUCED_DUMP_PATH = '../out/bus_stop_data.json'


def read_file_and_parse():
    dump = open('../out/staticBusStops.geojson').read()

    return json.loads(dump)


# reduces it to a list
def reduce_geojson(json_dict):

    features = json_dict['features']

    result = []

    for bus_stop in features:
        location = bus_stop['geometry']['coordinates']  # lat and longitude is swapped
        locationLatLong = location[::-1]

        description = bus_stop['properties']['LOC_DESC']
        stop_number = bus_stop['properties']['BUS_STOP_N']

        bus_stop_obj = {
            'l': locationLatLong,
            'd': description,
            'n': stop_number
        }

        result.append(bus_stop_obj)

    return result

def write_reduced_dump_to_file(reduced_list):

    data_set = {
        'data': reduced_list
    }

    with open(REDUCED_DUMP_PATH, 'w') as myFile:
        # serialise with json
        json.dump(data_set, myFile)


def main():

    json_dict = read_file_and_parse()

    reducedList = reduce_geojson(json_dict)
    write_reduced_dump_to_file(reducedList)

if __name__ == '__main__':
    main()