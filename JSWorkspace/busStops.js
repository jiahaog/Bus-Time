/**
 * Created by JiaHao on 11/6/15.
 */


var dataSet = require('./dataSet.js');
var geolib = require('geolib');
var pebbleHelpers = require('./pebbleHelpers');

DATA_SET_KEYS = {
    data: 'data',
    stopId: 'n',
    location: 'l',
    description: 'd'
};

CLOSEST_BUS_STOP_KEYS = {
    description: 'description',
    distance: 'distance',
    stopId: 'stopId'
};

/**
 * Iterates through the data store and finds the closest bus stop
 * @param {array} location [latitude, longitude]
 */
function getClosest(currentLocation) {


    var closestDistance = Number.MAX_VALUE;
    var closestBusStop = null;

    for (var i = 0; i < dataSet.length; i++) {


        var busStop = dataSet[i];

        var busStopLocation = busStop[DATA_SET_KEYS.location];

        var distance = distanceFrom(currentLocation, busStopLocation);
        if (distance < closestDistance) {
            closestDistance = distance;
            closestBusStop = busStop
        }
    }

    const result = {};
    result[CLOSEST_BUS_STOP_KEYS.description] = closestBusStop[DATA_SET_KEYS.description];
    result[CLOSEST_BUS_STOP_KEYS.distance] = closestDistance; // in metres
    result[CLOSEST_BUS_STOP_KEYS.stopId] = closestBusStop[DATA_SET_KEYS.stopId];

    return result

}

/**
 * Calculates distance from two points [latitude, longitude]
 * @param point1
 * @param point2
 * @returns {*}
 */
function distanceFrom(point1, point2) {

    var point1LatLong = {
        latitude: point1[0],
        longitude: point1[1]
    };

    var point2LatLong = {
        latitude: point2[0],
        longitude: point2[1]
    };

    return geolib.getDistance(point1LatLong, point2LatLong)
}


/**
 * REMOVE THIS
 * test function to get the description of the closest bus stop
 */
function testDistance() {
    // point taken from google places OPP TROPICANA CONDO
    gmapsPoint = [
        1.340414,
        103.961279
    ];

    //var dist = distanceFrom(dataPoint, gmapsPoint);
    //console.log(dist); // 5

    var closest = getClosest(gmapsPoint);
    console.log(closest[CLOSEST_BUS_STOP_KEYS.description]);
}


module.exports = {
    testDistance: testDistance
};


if (require.main === module) {
    testDistance()

}