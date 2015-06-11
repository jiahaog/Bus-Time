

# use http://ogre.adc4gis.com to convert the data downloaded from
# http://www.mytransport.sg/content/mytransport/home/dataMall.html bus stop location SHP

# Target SRS: WGS84
# Somehow latitude and longitude are reversed in output, so we have to reverse it when reducing


import json
from pprint import pprint



DESIRED_DATA_PATH = '../out/bus_stop_data.json'
REDUCED_DUMP_PATH = '../out/reduced_dump_data.json'  # path of data reduced from repeatedly calling the api

reducedDump = json.loads(open(REDUCED_DUMP_PATH).read())

def read_file_and_parse():
    dump = open('../out/staticBusStops.geojson').read()

    return json.loads(dump)


# reduces it to a list
def reduce_geojson(json_dict, excludeNil=True):

    features = json_dict['features']

    result = []

    for bus_stop in features:
        location = bus_stop['geometry']['coordinates']  # lat and longitude is swapped
        locationLatLong = location[::-1]

        description = bus_stop['properties']['LOC_DESC']
        stop_number = bus_stop['properties']['BUS_STOP_N']

        # means we skip over stop numbers which are nil
        if excludeNil:
            if stop_number == 'NIL':
                print 'skipping'
                continue

        # if the description is not valid, query the reduced dump to try and get a better description
        if not description:

            description = get_description_from_reduced_dump(stop_number)

        # now we check if the description is valid, and run title() on it
        if description:
            description = description.title()


        # size of keys are reduced, to reduce the file size
        bus_stop_obj = {'l': locationLatLong,
                        'd': description,
                        'n': stop_number,
                        'r': get_description_from_reduced_dump(stop_number)}

        result.append(bus_stop_obj)

    return result


def write_to_file(reduced_list):

    data_set = {
        'data': reduced_list
    }

    with open(DESIRED_DATA_PATH, 'w') as myFile:
        # serialise with json
        json.dump(data_set, myFile)


# helper methods to easily get keys and values from the reduced dump

def get_value_from_reduced_dump(stopId, key):

    # some stop ids will have unknown keys, will just return null


    dataList = reducedDump['data']

    for busStop in dataList:
        if str(stopId) == busStop['Code']:
            return busStop[key];

    print str(stopId) + ' stopid not found for key ' + key
    return None


def get_description_from_reduced_dump(stopId):
    return get_value_from_reduced_dump(stopId, 'Description')

def get_road_from_reduced_dump(stopId):
    return get_value_from_reduced_dump(stopId, 'Road')





def main():

    json_dict = read_file_and_parse()

    reducedList = reduce_geojson(json_dict)
    write_to_file(reducedList)

if __name__ == '__main__':
    main()
    # print get_value_from_reduced_dump(46561, 'Description')

