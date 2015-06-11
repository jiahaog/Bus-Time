__author__ = 'JiaHao'

import json
from pprint import pprint

# desired fields:
DUMP_KEYS = {
    'responseArray': 'busStops',
    'data': 'd',
    'busStopCode': 'Code',
    'description': 'Description',
    'road': 'Road'
}

DESIRED_KEYS = [DUMP_KEYS['busStopCode'], DUMP_KEYS['description'], DUMP_KEYS['road']]

REDUCED_DUMP_PATH = '../out/reduced_dump_data.json'

def read_file_and_parse():
    dump = open('../out/prettyfiedDump.json').read()

    return json.loads(dump)



def get_response_list(json_dict):

    return json_dict['busStops']

def iterate_and_reduce(response_list):

    result = []
    for response in response_list:
        data = response[DUMP_KEYS['data']]

        for busStop in data:
            reducedBusStop = {}

            for key in DESIRED_KEYS:
                reducedBusStop[key] = busStop[key]

            result.append(reducedBusStop)

    return result


def write_reduced_dump_to_file(reduced_list):

    data_set = {
        'data': reduced_list
    }

    with open(REDUCED_DUMP_PATH, 'w') as myFile:
        # serialise with json
        json.dump(data_set, myFile)


def get_bus_stop_keyword(reduced_list):
    result = []
    for bus_stop in reduced_list:
        keyword = bus_stop[DUMP_KEYS['description']] + " " + bus_stop[DUMP_KEYS['road']]

        result.append(keyword)
    return result

# Returns number of duplicate descriptions
def search_for_duplicates(keyword_list):

    unique_descriptions = []
    for keyword in keyword_list:

        if keyword not in unique_descriptions:
            unique_descriptions.append(keyword)
        else:
            print keyword

    return len(keyword_list) - len(unique_descriptions)


def main():

    parsed = read_file_and_parse()

    response_list = get_response_list(parsed)

    reduced_list = iterate_and_reduce(response_list)

    write_reduced_dump_to_file(reduced_list)


if __name__ == '__main__':
    main()