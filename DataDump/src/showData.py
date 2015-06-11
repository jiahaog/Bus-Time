import json

bus_stop_list = json.loads(open('../out/bus_stop_data.json').read())['data']



lengths = []

for bus_stop in bus_stop_list:
    if bus_stop['d']:
        print len(bus_stop['d'])
        lengths.append(len(bus_stop['d']))

print lengths
print max(lengths)