/**
 * Created by JiaHao on 14/6/15.
 */

var pebbleHelpers = require('./pebbleHelpers');
var constants = require('./constants');



/**
 * Translates the absolute arrival timing of the bus to relative time from the current moment
 *
 * Assumes mobile device time is set to the Singapore Standard Time
 *
 * This function is used in two situations:
 *  to check if the record is valid,
 *  and to format a string to be sent to the watch
 *
 *  Therefore, it is possible for the time to be negative and the result to be null, in the second case where the records are not updated
 *  on the server side
 *
 *
 * @param arrivalString utc date string
 * @returns {string} e.g. '1m 20s', null if negative, '-' if the data received from myTransport is null
 */
function getTimeToArrival(arrivalString) {
    if (!arrivalString) {
        console.log('Unable to parse arrival time: ' + arrivalString);
        // assumes that the time is unavailable
        return '-';
    }

    const utcArrival = Date.parse(arrivalString);
    const utcNow = Date.now();
    //const utcNow = 1433859075852;

    const differenceMs = utcArrival - utcNow;

    const min = Math.floor(differenceMs/1000/60);

    // todo round down seconds to nearest minute
    const sec = Math.floor((differenceMs/1000) % 60);

    if ((min < 0) || (sec < 0)) {
        return null;
    }

    //return min + ':' + sec;
    return min + 'm ' + sec + 's';
}

/**
 * @typedef {Object} parsedServiceDetailsResult
 * @property {string} serviceNo
 * @property {Object} nextbus
 * @property {Object} subsequentbus
 */

/**
 *
 * @param record
 * @param {number} desiredServiceNo bus service number
 * @return {parsedServiceDetailsResult} or null if service not found
 */
function parseForServiceDetails(record, desiredServiceNo) {
    const services = record[constants.RESPONSE_KEYS.services];

    // iterate through and find the correct service
    for (var i = 0; i < services.length; i++) {
        var currentService = services[i];

        // convert desiredServiceNo to string, as currentService is a string
        if (currentService[constants.RESPONSE_KEYS.serviceNo] === desiredServiceNo.toString()) {

            // copies the next buses
            var nextBus = pebbleHelpers.cloneObject(currentService[constants.RESPONSE_KEYS.nextBus]);
            var subsequentBus = pebbleHelpers.cloneObject(currentService[constants.RESPONSE_KEYS.subsequentBus]);

            // short circuit evaluation in case the tta is negative
            nextBus[constants.RESPONSE_KEYS.estimatedArrival] = getTimeToArrival(nextBus[constants.RESPONSE_KEYS.estimatedArrival]) || 'Arr.';
            subsequentBus[constants.RESPONSE_KEYS.estimatedArrival] = getTimeToArrival(subsequentBus[constants.RESPONSE_KEYS.estimatedArrival]) || 'Arr.';

            const serviceObject = {};
            serviceObject[constants.RESPONSE_KEYS.serviceNo] = currentService[constants.RESPONSE_KEYS.serviceNo];
            serviceObject[constants.RESPONSE_KEYS.nextBus] = nextBus;
            serviceObject[constants.RESPONSE_KEYS.subsequentBus] = subsequentBus;

            return serviceObject;
        }
    }

    return null;
}

/**
 * Parses a record and returns an array of operational bus services
 * @param record
 * @returns {Array}
 */
function parseForServicesList(record) {
    const services = record[constants.RESPONSE_KEYS.services];

    const result = [];

    for (var i = 0; i < services.length; i++) {

        var currentService = services[i];

        // only push if service is operational
        if (currentService[constants.RESPONSE_KEYS.status] === constants.RESPONSE_KEYS.inOperation) {

            var nextBusArrivalTime = getTimeToArrival(currentService[constants.RESPONSE_KEYS.nextBus][constants.RESPONSE_KEYS.estimatedArrival]) || "Arr.";

            var resultObj = {};
            resultObj[constants.RESPONSE_KEYS.serviceNo] = currentService[constants.RESPONSE_KEYS.serviceNo];
            resultObj[constants.RESPONSE_KEYS.estimatedArrival] = nextBusArrivalTime;

            // converts the service number to a numeric number so that we can use it for sorting
            resultObj[constants.RESPONSE_KEYS.serviceNoNumeric] = parseInt(currentService[constants.RESPONSE_KEYS.serviceNo].replace(/\D/g,''));
            result.push(resultObj);

        }
    }

    // result[] now contains a list of objs

    // now we sort it

    result.sort(function (a, b) {
        var busA = a[constants.RESPONSE_KEYS.serviceNoNumeric];
        var busB = b[constants.RESPONSE_KEYS.serviceNoNumeric];

        return busA - busB;
    });

    var resultProcessed = []; // array of strings with desired values set by delimiter

    for (var j = 0; j < result.length; j++) {
        var sortedResultObj = result[j];
        resultProcessed.push(sortedResultObj[constants.RESPONSE_KEYS.serviceNo] + constants.MESSAGE_DELIMITER + sortedResultObj[constants.RESPONSE_KEYS.estimatedArrival]);
    }

    return resultProcessed;

}

module.exports = {
    parseForServiceDetails: parseForServiceDetails,
    parseForServicesList: parseForServicesList,
    getTimeToArrival: getTimeToArrival
};