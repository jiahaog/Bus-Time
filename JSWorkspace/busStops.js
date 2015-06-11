/**
 * Created by JiaHao on 11/6/15.
 */

var dataSet = require('./dataSet.js');
var geolib = require('geolib');

DATA_SET_KEYS = {
    data: 'data',
    stopId: 'n',
    location: 'l',
    description: 'd',
    road: 'r'
};

CLOSEST_BUS_STOP_KEYS = {
    description: 'description',
    distance: 'distance',
    stopId: 'stopId',
    road: 'road'
};

NEARBY_THRESHOLD = 500; // 500m
MAX_NEARBY = 5;

/**
 * Iterates through the data store and finds the nearby bus stops that fall within the threshold
 * @param {Array} currentLocation [latitude, longitude]
 * @returns {Array} array of bus stops that is ordered from closest (index 0) to furthest (index array.length)
 */
function getNearbyBusStops(currentLocation) {

    // find nearby bus stops that fall within the nearby threshold
    var busStopsNearby = [];
    for (var i = 0; i < dataSet.length; i++) {
        var busStop = dataSet[i];

        var busStopLocation = busStop[DATA_SET_KEYS.location];

        var distance = distanceFrom(currentLocation, busStopLocation);

        if (distance < NEARBY_THRESHOLD) {
            var nearbyBusStop = {};
            nearbyBusStop[CLOSEST_BUS_STOP_KEYS.description] = busStop[DATA_SET_KEYS.description];
            nearbyBusStop[CLOSEST_BUS_STOP_KEYS.distance] = distance; // in metres
            nearbyBusStop[CLOSEST_BUS_STOP_KEYS.stopId] = busStop[DATA_SET_KEYS.stopId];
            nearbyBusStop[CLOSEST_BUS_STOP_KEYS.road] = busStop[DATA_SET_KEYS.road];

            busStopsNearby.push(nearbyBusStop);
        }
    }

    busStopsNearby.sort(function (a, b) {

        var distanceA = a[CLOSEST_BUS_STOP_KEYS.distance];
        var distanceB = b[CLOSEST_BUS_STOP_KEYS.distance];
        return distanceA - distanceB;
    });

    return busStopsNearby.slice(0, MAX_NEARBY); // don't return more than the max nearby bus stops
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

    var nearby = getNearbyBusStops(gmapsPoint);

    var firstDescription = nearby[0][CLOSEST_BUS_STOP_KEYS.description];

    console.log(firstDescription);
}


module.exports = {
    CLOSEST_BUS_STOP_KEYS: CLOSEST_BUS_STOP_KEYS,
    testDistance: testDistance,
    getNearbyBusStops: getNearbyBusStops
};


if (require.main === module) {
    testDistance()

}