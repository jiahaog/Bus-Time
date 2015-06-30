/**
 * Created by JiaHao on 18/6/15.
 */

var pebbleHelpers = require('./../pebbleHelpers');
var busStops = require('./../process_data/busStops');
var recordParser = require('./../process_data/recordParser');
var constants = require('./../constants/constants');
var recordCache = require('./../model/recordCache');

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
 * Transforms the bus arrival time, from '10m|Arr.|-' to '10|a|-'
 * For use with sending the service details to the watch, as the details requires special formats of string
 * @param {string} inp
 * @param {string} [append] if an argument is provided here, if the inp string contains a number, the append string will
 *                          be appended to the tail
 * @returns {string}
 */
function transformArrivalsForServiceDetails(inp, append) {
    const NUMBER_REGEX = /\d+/;

    if (inp === 'Arr.') {
        return 'a';
    } else if (inp === '-') {
        return '-'
    } else {
        var result = inp.match(NUMBER_REGEX).toString();
        if (append) {
            return result + append;
        } else {
            return result;
        }
    }
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

                var nextBusArrivalTimeString = transformArrivalsForServiceDetails(serviceDetails[constants.RESPONSE_KEYS.nextBus][constants.RESPONSE_KEYS.estimatedArrival]);
                var subsequentBusArrivalTimeString = transformArrivalsForServiceDetails(serviceDetails[constants.RESPONSE_KEYS.subsequentBus][constants.RESPONSE_KEYS.estimatedArrival],' min');

                messageString =
                    stopId + constants.MESSAGE_DELIMITER +
                    serviceDetails[constants.RESPONSE_KEYS.serviceNo] + constants.MESSAGE_DELIMITER +
                    nextBusArrivalTimeString + constants.MESSAGE_DELIMITER +
                    serviceDetails[constants.RESPONSE_KEYS.nextBus][constants.RESPONSE_KEYS.load] + constants.MESSAGE_DELIMITER +
                    subsequentBusArrivalTimeString + constants.MESSAGE_DELIMITER +
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

module.exports = {
    sendBusStopList: sendBusStopList,
    sendErrorCode: sendErrorCode,
    sendServicesList: sendServicesList,
    sendServiceDetails: sendServiceDetails,
    sendNotificationStatus: sendNotificationStatus
};