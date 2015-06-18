/**
 * Created by JiaHao on 8/6/15.
 */

var pebbleHelpers = require('./pebbleHelpers');
var busStops = require('./busStops');
var recordParser = require('./recordParser');
var constants = require('./constants');
var recordCache = require('./recordCache');
var busServiceNotifier = require('./busServiceNotifier');

var lastStopID; // hold the last bus stop id so we know which bus stop to query for arrivals todo get rid of this
var lastAppMessageTime;
var watchBusStopIntervalId;
var watchBusServiceIntervalId;

if (constants.RELEASE_MODE) {
    var REFRESH_INTERVAL = 60 * 1000; // 1 min
} else {
    REFRESH_INTERVAL = 10 * 1000; // 10 sec
}

//const REFRESH_TIMEOUT = 10*60*1000; // 10 min
const REFRESH_TIMEOUT = 15 * 60 * 1000; // 10 min

/**
 * Gets the location of the watch and sends nearby bus stops to the watch
 */
function sendBusStopList() {
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

/**
 * @callback sentAppMessageCallback
 * @param error
 */

/**
 * Sends a list of the services available at the current bus stop
 * @param stopId
 * @param {sentAppMessageCallback} callback
 */
function sendServicesList(stopId, callback) {
    recordCache.getBusTimings(stopId, undefined, true, function (error, record) {
        if (error) {
            console.log('Error getting bus timings');
            sendErrorCode(constants.ERROR_CODES.NETWORK_ERROR);
            callback(error);
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
                callback('No Services Operational');
            }
        }
    });
}

/**
 *
 * @param stopId
 * @param serviceNo
 * @param {sentAppMessageCallback} callback
 */
function sendServiceDetails(stopId, serviceNo, callback) {
    recordCache.getBusTimings(stopId, serviceNo, true, function (error, record) {
        if (error) {
            console.log('Error getting bus timings');
            sendErrorCode(constants.ERROR_CODES.NETWORK_ERROR);
            callback(error);
        } else {
            //console.log(JSON.stringify(record));
            const serviceDetails = recordParser.parseForServiceDetails(record, serviceNo);

            var messageString;
            if (serviceDetails) {
                messageString =
                    stopId + constants.MESSAGE_DELIMITER +
                    serviceDetails[constants.RESPONSE_KEYS.serviceNo] + constants.MESSAGE_DELIMITER +
                    serviceDetails[constants.RESPONSE_KEYS.nextBus][constants.RESPONSE_KEYS.estimatedArrival] + constants.MESSAGE_DELIMITER +
                    serviceDetails[constants.RESPONSE_KEYS.nextBus][constants.RESPONSE_KEYS.load] + constants.MESSAGE_DELIMITER +
                    serviceDetails[constants.RESPONSE_KEYS.subsequentBus][constants.RESPONSE_KEYS.estimatedArrival] + constants.MESSAGE_DELIMITER +
                    serviceDetails[constants.RESPONSE_KEYS.subsequentBus][constants.RESPONSE_KEYS.load];
            } else {
                messageString = 'Service details for service ' + serviceNo + ' not found!';
                console.log(messageString);
                callback(messageString);
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


/**
 *
 * @param {boolean} started
 * @param {string} stopId
 * @param {string} serviceNo
 */
function sendNotificationStatus(started, stopId, serviceNo) {

    const startedString = started ? '1' : '0';
    var concatenatedMessage = startedString + constants.MESSAGE_DELIMITER +
        stopId + constants.MESSAGE_DELIMITER +
        serviceNo;

    var dictionaryMessage = {};
    dictionaryMessage[constants.APP_MESSAGE_KEYS.KEY_BUS_NOTIFICATION] = concatenatedMessage;
    pebbleHelpers.sendMessage(dictionaryMessage, function (error) {
        if (error) {
            console.log('Error sending notification status to pebble');
        }
    });
}

function watchBusStop(stopId) {

    function sendAndManageServicesList(stopId) {
        // if app has not timed out
        if (!checkIfAppTimeout()) {
            sendServicesList(stopId, function (error) {

                if (error) {
                    // if the interval has been set
                    if (watchBusStopIntervalId) {
                        console.log('Clearing interval for bus stop');
                        clearInterval(watchBusStopIntervalId);
                    }
                }
            });
        }
    }

    if (watchBusStopIntervalId) {
        clearInterval(watchBusStopIntervalId);
    }

    console.log('Watching bus stop id: ' + stopId);

    lastStopID = stopId;

    sendAndManageServicesList(stopId);
    watchBusStopIntervalId = setInterval(function () {
        console.log('Updating services list');
        sendAndManageServicesList(stopId);
    }, REFRESH_INTERVAL);
}

function watchBusServiceDetails(stopId, serviceNo) {

    function sendAndManageServiceDetails(stopId, serviceNo) {
        // if app has not timed out
        if (!checkIfAppTimeout()) {
            sendServiceDetails(stopId, serviceNo, function (error) {
                if (error) {
                    // if the interval has been set
                    if (watchBusStopIntervalId) {
                        console.log('Clearing interval for bus service details');
                        clearInterval(watchBusServiceIntervalId);
                    }
                }
            });
        }
    }

    if (watchBusServiceIntervalId) {
        clearInterval(watchBusServiceIntervalId);
    }

    console.log('Watching bus service: ' + stopId + ', ' + serviceNo);

    sendAndManageServiceDetails(stopId, serviceNo);

    watchBusServiceIntervalId = setInterval(function () {
        console.log('Updating service details');
        sendAndManageServiceDetails(stopId, serviceNo);
    }, REFRESH_INTERVAL);

}

/**
 * Checks if the last app message is within the REFRESH_TIMEOUT, and clears intervals if so
 */
function checkIfAppTimeout() {

    const currentTime = Date.now();

    if (currentTime - lastAppMessageTime > REFRESH_TIMEOUT) {
        clearInterval(watchBusStopIntervalId);
        clearInterval(watchBusServiceIntervalId);
        sendErrorCode(constants.ERROR_CODES.APP_TIMEOUT);

        console.log('App timeout. Cleared poll intervals');
        return true;
    } else {
        return false;
    }
}

function processReceivedMessage(event) {

    const payload = event[constants.MISC_KEYS.payload];


    for (var key in payload) {
        if (payload.hasOwnProperty(key)) {
            // value is an int
            var value = payload[key];

            if (key === constants.APP_MESSAGE_KEYS.KEY_BUS_SERVICE_LIST_START) {
                lastAppMessageTime = Date.now();
                watchBusStop(value);

            } else if (key === constants.APP_MESSAGE_KEYS.KEY_BUS_SERVICE_DETAILS_START) {
                // enter the services details page from services list

                lastAppMessageTime = Date.now();
                console.log('Received request for service: ' + value);

                // stop watching the bus services list
                clearInterval(watchBusStopIntervalId);

                // watch the service details
                watchBusServiceDetails(lastStopID, value);

            } else if (key === constants.APP_MESSAGE_KEYS.KEY_BUS_SERVICE_DETAILS_END) {
                // going back to services list from details window

                lastAppMessageTime = Date.now();

                // stop watching the service details
                clearInterval(watchBusServiceIntervalId);

                // start watching the bus services list
                watchBusStop(lastStopID);

            } else if (key === constants.APP_MESSAGE_KEYS.KEY_BUS_SERVICE_LIST_END) {
                // going back to bus stop list from bus services list
                lastAppMessageTime = Date.now();
                // stop watching the bus services list
                clearInterval(watchBusStopIntervalId);
            } else if (key === constants.APP_MESSAGE_KEYS.KEY_BUS_NOTIFICATION) {
                // handle notification

                // message format {set_or_cancel_notification}|{stop_id}|{service_no}
                var splitDetails = value.split(constants.MESSAGE_DELIMITER);
                var startNotification = parseInt(splitDetails[0]);  // parseint here so we can do a if (setorcancel)
                var stopId = splitDetails[1];
                var serviceNo = splitDetails[2];

                if (startNotification) {
                    busServiceNotifier.startNotification(stopId, serviceNo);
                } else {
                    busServiceNotifier.stopNotification(stopId, serviceNo);
                }
            }
        }
    }
}

// when the app is launched get the location and send nearby bus stops to the watch
pebbleHelpers.addEventListener.onReady(function () {
    // restore bus data
    console.log('=== B U S  T I M E ===');
    recordCache.restoreCache();

    sendBusStopList();

});

pebbleHelpers.addEventListener.onAppMessage(function (event) {
    processReceivedMessage(event);
});