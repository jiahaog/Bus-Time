import json

bus_stop_list = json.loads(open('../out/bus_stop_data.json').read())['data']




def print_all_descriptions():
    for bus_stop in bus_stop_list:
        if bus_stop['d']:
            print bus_stop['d']

def print_all_roads():
    for bus_stop in bus_stop_list:
        if bus_stop['r']:
            print bus_stop['r']

if __name__ == '__main__':
    # print_all_descriptions()
    print_all_roads()