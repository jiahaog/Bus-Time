/**
 * Created by JiaHao on 14/6/15.
 */
    
var constants = require('./../constants/constants');

const REDUCED_SERVICE_LOAD_STRING = {
    seatsAvailable: '3',
    standingAvailable: '2',
    limitedStanding: '1',
    error: '0'
};

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
 * @param {boolean} [number] returns the timing as a utc number
 * @returns {string|number} e.g. if number parameter false: '1m', 'Arr.' if negative, '-' if the data received from myTransport is null |
 *                               if number parameter is true: timing (ms), null if the arrivalString is null
 */
function getTimeToArrival(arrivalString, number) {
    if (!arrivalString) {
        console.log('Unable to parse arrival time: ' + arrivalString);
        // assumes that the time is unavailable

        if (number) {
            return null;
        } else {
            return '-';
        }
    }

    const utcArrival = Date.parse(arrivalString);
    const utcNow = Date.now();
    //const utcNow = 1433859075852;

    const differenceMs = utcArrival - utcNow;

    const min = Math.floor(differenceMs / 1000 / 60);
    if (number) {
        //return differenceMs;
        return min*60*1000;  // convert minutes to ms
    } else {

        if (min <= 0 ) {
            return 'Arr.';
        } else {
            return min + 'm';
        }
        //const sec = Math.floor((differenceMs / 1000) % 60);
        //if ((min < 0) || (sec < 0)) {
        //    return 'Arr.';
        //}

        //return min + ':' + sec;
        //return min + 'm ' + sec + 's';
    }
}

/**
 * Reduces the service load string to a value that can be easily parsed on the watch
 * @param {string} serviceStatus
 * @returns {string} a value inside {REDUCED_SERVICE_LOAD_STRING} or null if parameter not recognised
 */
function reduceServiceLoadString(serviceStatus) {
    if (serviceStatus === 'Seats Available') {
        return REDUCED_SERVICE_LOAD_STRING.seatsAvailable;
    } else if (serviceStatus === 'Standing Available') {
        return REDUCED_SERVICE_LOAD_STRING.standingAvailable;
    } else if (serviceStatus === 'Limited Standing') {
        return REDUCED_SERVICE_LOAD_STRING.limitedStanding;
    } else {
        console.log('INVALID SERVICE LOAD');
        return REDUCED_SERVICE_LOAD_STRING.error;
    }
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
 * @param {boolean} [timingsAsNumber] if true, dont parse the arrival time, and return the raw utc time
 * @return {parsedServiceDetailsResult} or null if service not found
 */
function parseForServiceDetails(record, desiredServiceNo, timingsAsNumber) {
    const services = record[constants.RESPONSE_KEYS.services];

    // iterate through and find the correct service
    for (var i = 0; i < services.length; i++) {
        var currentService = services[i];

        // convert desiredServiceNo to string, as currentService is a string
        if (currentService[constants.RESPONSE_KEYS.serviceNo] === desiredServiceNo.toString()) {

            // copies the next buses
            var recordNextBus = currentService[constants.RESPONSE_KEYS.nextBus];
            var recordSubsequentBus = currentService[constants.RESPONSE_KEYS.subsequentBus];

            var nextBus = {};
            var subsequentBus = {};
            nextBus[constants.RESPONSE_KEYS.estimatedArrival] = getTimeToArrival(recordNextBus[constants.RESPONSE_KEYS.estimatedArrival], timingsAsNumber);
            subsequentBus[constants.RESPONSE_KEYS.estimatedArrival] = getTimeToArrival(recordSubsequentBus[constants.RESPONSE_KEYS.estimatedArrival], timingsAsNumber);

            nextBus[constants.RESPONSE_KEYS.load] = reduceServiceLoadString(recordNextBus[constants.RESPONSE_KEYS.load]);
            subsequentBus[constants.RESPONSE_KEYS.load] = reduceServiceLoadString(recordSubsequentBus[constants.RESPONSE_KEYS.load]);

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