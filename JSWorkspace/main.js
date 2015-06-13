/**
* Created by JiaHao on 8/6/15.
*/

var pebbleHelpers = require('./pebbleHelpers');
var busStops = require('./busStops');
var recordParser = require('./recordParser');
var constants = require('./constants');
var recordCache = require('./recordCache');


var lastBusStopsIDsSent = [];
var lastStopID;

/**
 * Gets the location of the watch and sends nearby bus stops to the watch
 */
function processLocation() {
    pebbleHelpers.getLocation(function (error, position) {
        if (error) {
            console.log('location error (' + error.code + '): ' + error.message);
            sendErrorCode(constants.ERROR_CODES.LOCATION_ERROR);
        } else {

            var positionArray = [position.coords.latitude, position.coords.longitude];

            var nearbyBusStops = busStops.getNearbyBusStops(positionArray);

            // iterates through the nearby bus stops and populates two arrays of descriptions and
            // stop id
            var descriptions = [];
            var stopIds = [];
            for (var i = 0; i < nearbyBusStops.length; i++) {
                var busStop = nearbyBusStops[i];
                descriptions.push(
                    busStop[busStops.CLOSEST_BUS_STOP_KEYS.description] + constants.MESSAGE_DELIMITER +
                    busStop[busStops.CLOSEST_BUS_STOP_KEYS.road] + constants.MESSAGE_DELIMITER +
                    busStop[busStops.CLOSEST_BUS_STOP_KEYS.stopId]);

                stopIds.push(busStop[busStops.CLOSEST_BUS_STOP_KEYS.stopId]);
            }

            // sends the message to the watch
            pebbleHelpers.sendMessageStream(
                constants.APP_MESSAGE_KEYS.KEY_BUS_STOP_LIST_START,
                constants.APP_MESSAGE_KEYS.KEY_BUS_STOP_LIST_VALUE,
                constants.APP_MESSAGE_KEYS.KEY_BUS_STOP_LIST_END,
                descriptions
            );

            // save the last stop ids sent to the watch, so that when the user chooses the item on the menu,
            // the index will be sent back and we can get the corresponding stopId
            lastBusStopsIDsSent = stopIds;
        }
    })
}


/**
 * @param {int} code
 */
function sendErrorCode(code) {

    var dictionaryMessage = {};
    dictionaryMessage[constants.APP_MESSAGE_KEYS.KEY_ERROR] = code;

    pebbleHelpers.sendMessage(dictionaryMessage, function (error) {
        if (error) {
            console.log('Error sending connection error message! REASON:' + error);
        } else {
            // callback
        }
    });

}

var busTimings = {

    // gets a list of services at the bus stop
    sendServicesList: function (stopId) {
        recordCache.getBusTimings(stopId, undefined, function (error, record) {
            if (error) {
                console.log('Error getting bus timings');
                sendErrorCode(constants.ERROR_CODES.NETWORK_ERROR);
            } else {

                const serviceList = recordParser.parseForServicesList(record);

                if (serviceList.length > 0) {
                    pebbleHelpers.sendMessageStream(
                        constants.APP_MESSAGE_KEYS.KEY_BUS_SERVICE_LIST_START,
                        constants.APP_MESSAGE_KEYS.KEY_BUS_SERVICE_LIST_VALUE,
                        constants.APP_MESSAGE_KEYS.KEY_BUS_SERVICE_LIST_END,
                        serviceList
                    );
                } else {
                    sendErrorCode(constants.ERROR_CODES.NO_SERVICES_OPERATIONAL);
                }



            }
        });
    },

    // gets the details for the service
    sendServiceDetails: function (stopId, serviceNo) {
        recordCache.getBusTimings(stopId, serviceNo, function(error, record) {
            if (error) {
                console.log('Error getting bus timings');
                sendErrorCode(constants.ERROR_CODES.NETWORK_ERROR);
            } else {
                //console.log(JSON.stringify(record));
                const serviceDetails = recordParser.parseForServiceDetails(record, serviceNo);

                var messageString;
                if (serviceDetails) {
                    messageString =
                        'Service ' + serviceDetails[constants.RESPONSE_KEYS.serviceNo] + '\n' +
                        'Next Bus:\n' +
                        serviceDetails[constants.RESPONSE_KEYS.nextBus][constants.RESPONSE_KEYS.estimatedArrival] + '\n' +
                        serviceDetails[constants.RESPONSE_KEYS.nextBus][constants.RESPONSE_KEYS.load] + '\n' +
                        'Subsequent Bus:\n' +
                        serviceDetails[constants.RESPONSE_KEYS.subsequentBus][constants.RESPONSE_KEYS.estimatedArrival] + '\n' +
                        serviceDetails[constants.RESPONSE_KEYS.subsequentBus][constants.RESPONSE_KEYS.load];
                } else {
                    messageString = 'Service not found'
                }

                var dictionaryMessage = {};
                dictionaryMessage[constants.APP_MESSAGE_KEYS.KEY_BUS_SERVICE_DETAILS_VALUE] = messageString;

                pebbleHelpers.sendMessage(dictionaryMessage, function (error) {
                    if (error) {
                        console.log('Error sending message!' + error);
                    } else {
                        // callback
                    }
                });
            }
        })
    }
};

// when the app is launched get the location and send nearby bus stops to the watch
pebbleHelpers.addEventListener.onReady(function () {
    // restore bus data

    recordCache.restoreCache();

    processLocation();
    //setTimeout(function () {
    //    pebbleHelpers.sendNotification('message title', 'message text');
    //}, 10000);

});

pebbleHelpers.addEventListener.onAppMessage(function (event) {
    processReceivedMessage(event);
});

function processReceivedMessage(event) {

    const payload = event[constants.MISC_KEYS.payload];

    for (var key in payload) {
        if (payload.hasOwnProperty(key)) {
            // value is an int
            var value = payload[key];

            if (key === constants.APP_MESSAGE_KEYS.KEY_BUS_SERVICE_LIST_START) {
                var stopId = lastBusStopsIDsSent[value];
                console.log('Received request for service list for stopID: ' + stopId);
                lastStopID = stopId;
                busTimings.sendServicesList(stopId);

            } else if (key === constants.APP_MESSAGE_KEYS.KEY_BUS_SERVICE_DETAILS_START) {
                console.log('Received request for service: ' + value);
                busTimings.sendServiceDetails(lastStopID, value);
            }
        }
    }
}
