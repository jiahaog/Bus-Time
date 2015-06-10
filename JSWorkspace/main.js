/**
* Created by JiaHao on 8/6/15.
*/
var pebbleHelpers = require('./pebbleHelpers');
var config = require('./config');
const API_URL = 'http://datamall2.mytransport.sg/ltaodataservice/BusArrival';

const REQUEST_HEADERS = {
    AccountKey: config.MY_TRANSPORT_KEYS.AccountKey,
    UniqueUserId: config.MY_TRANSPORT_KEYS.UniqueUserId,
    accept: 'application/json'
};


const TEST_RESPONSE = '{"odata.metadata":"http://datamall2.mytransport.sg/ltaodataservice/$metadata#BusArrival/@Element","BusStopID":"83139","Services":[{"ServiceNo":"15","Status":"In Operation","Operator":"SBST","NextBus":{"EstimatedArrival":"2015-06-09T14:25:49+00:00","Load":"Standing Available","Feature":"WAB"},"SubsequentBus":{"EstimatedArrival":"2015-06-09T13:56:32+00:00","Load":"Seats Available","Feature":"WAB"}},{"ServiceNo":"155","Status":"In Operation","Operator":"SBST","NextBus":{"EstimatedArrival":"2015-06-09T13:47:03+00:00","Load":"Seats Available","Feature":"WAB"},"SubsequentBus":{"EstimatedArrival":"2015-06-09T14:01:57+00:00","Load":"Seats Available","Feature":"WAB"}}]}';
const REFRESH_THRESHOLD = 60*60*1000; // in ms (temporarily set to 60 mins)

const APP_MESSAGE_KEYS = {

    KEY_BUS_SERVICE_LIST_START: 'KEY_BUS_SERVICE_LIST_START',
    KEY_BUS_SERVICE_LIST_VALUE: 'KEY_BUS_SERVICE_LIST_VALUE',
    KEY_BUS_SERVICE_LIST_END: 'KEY_BUS_SERVICE_LIST_END',

    KEY_BUS_SERVICE_DETAILS_START: 'KEY_BUS_SERVICE_DETAILS_START',
    KEY_BUS_SERVICE_DETAILS_VALUE: 'KEY_BUS_SERVICE_DETAILS_VALUE',
    KEY_BUS_SERVICE_DETAILS_END: 'KEY_BUS_SERVICE_DETAILS_END'
};

const RESPONSE_KEYS = {
    metadata: 'odata.metadata',
    stopId: 'BusStopID',
    services: 'Services',
    serviceNo: 'ServiceNo',
    status: 'Status',
    operator: 'Operator',
    nextBus: 'NextBus',
    estimatedArrival: 'EstimatedArrival',
    load: 'Load',
    feature: 'Feature',
    inOperation: 'In Operation',

    time: 'Time' // added key
};


var store = [];

/**
 * Queries the store for a valid record that falls within the threshold and has the same stopId
 * @param stopId
 * @returns {*} null if not found
 */
function getValidRecordFromStore(stopId) {
    for (var i = 0; i < store.length; i++) {
        var storeRecord = store[i];

        var recordInLastMinute = (Date.now() - storeRecord[RESPONSE_KEYS.time]) > REFRESH_THRESHOLD;
        var sameStopId = storeRecord[RESPONSE_KEYS.stopId] === stopId;

        if (sameStopId && recordInLastMinute) {
            return storeRecord;
        }
    }

    // clear array if not found
    // todo make it only clear the expired stopIds
    store = [];

    return null;
}
/**
 * @callback responseCallback
 * @param error SHOULD NOT BE CALLED
 * @param {obj} record object appended with the current time
 */

/**
 * Checks the cache for a record, and if not found makes a request to get the record
 * @param stopId
 * @param {responseCallback} callback
 */
function getBusTimings(stopId, callback) {

    var record = getValidRecordFromStore(stopId);

    if (record) {
        // if a valid record is found,
        console.log('Using cached record...');
        callback(undefined, record);

    } else {

        // make a query

        const params = {
            BusStopID: stopId
        };

        pebbleHelpers.xhrRequest(API_URL, 'GET', REQUEST_HEADERS, params, function(responseText) {
            console.log('StopId ' + stopId + ' request completed');

            record = JSON.parse(responseText);
            // add the time of the query in
            record[RESPONSE_KEYS.time] = Date.now();

            // cache it in the store
            store.push(record);

            callback(undefined, record);
        });
    }
}

/**
 *
 * @param record
 * @param {number} desiredServiceNo bus service number
 */
function parseForServiceDetails(record, desiredServiceNo) {
    const services = record[RESPONSE_KEYS.services];

    // iterate through and find the correct service
    for (var i = 0; i < services.length; i++) {
        var currentService = services[i];

        if (currentService[RESPONSE_KEYS.serviceNo] === desiredServiceNo) {

            // mutate the arrival timing to time from now
            var nextBus = currentService[RESPONSE_KEYS.nextBus];
            nextBus[RESPONSE_KEYS.estimatedArrival] = getTimeToArrival(nextBus[RESPONSE_KEYS.estimatedArrival]);

            var subsequentBus = currentService[RESPONSE_KEYS.subsequentBus];
            subsequentBus[RESPONSE_KEYS.estimatedArrival] = getTimeToArrival(subsequentBus[RESPONSE_KEYS.estimatedArrival]);

            const serviceObject = {
                serviceNo: currentService[RESPONSE_KEYS.serviceNo],
                nextBus: nextBus,
                subsequentBus: subsequentBus
            };

            return serviceObject;
        }
    }
}


/**
 * Parses a record and returns an array of operational bus services
 * @param record
 * @returns {Array}
 */
function parseForServicesList(record) {
    const services = record[RESPONSE_KEYS.services];

    const result = [];

    for (var i = 0; i < services.length; i++) {

        var currentService = services[i];

        // only push if service is operational
        if (currentService[RESPONSE_KEYS.status] === RESPONSE_KEYS.inOperation) {
            result.push(currentService[RESPONSE_KEYS.serviceNo]);
        }
    }

    return result;
}

/**
 * Translates the absolute arrival timing of the bus to relative time from the current moment
 *
 * Assumes mobile device time is set to the Singapore Standard Time
 *
 * @param arrivalString utc date string
 * @returns {string} e.g. '1m 20s'
 */
function getTimeToArrival(arrivalString) {
    if (arrivalString === 'null') {
        console.log('Unable to get parse arrival time');
        return arrivalString;
    }

    const utcArrival = Date.parse(arrivalString);
    const utcNow = Date.now();
    //const utcNow = 1433859075852;

    // find difference and convert milliseconds to minutes
    const differenceMs = (utcArrival - utcNow);


    const min = Math.floor(differenceMs/1000/60);

    // todo round down seconds to nearest minute
    const sec = Math.floor((differenceMs/1000) % 60);

    //return min + ':' + sec;
    return min + 'm ' + sec + 's';
}



var busTimings = {

    // gets a list of services at the bus stop
    sendServicesList: function (stopId) {
        getBusTimings(stopId, function (error, record) {
            if (error) {
                console.log('Error getting bus timings');
                console.log(error);
            } else {

                //const serviceListString = parseForServicesList(record).toString();

                //const dictionaryMessage = {
                //    KEY_BUS_SERVICE_LIST: serviceListString
                //};
                //
                //pebbleHelpers.pebbleSendMessage(dictionaryMessage);
                //

                const serviceList = parseForServicesList(record);
                pebbleHelpers.pebbleSendMessageSequentially(
                    APP_MESSAGE_KEYS.KEY_BUS_SERVICE_LIST_START,
                    APP_MESSAGE_KEYS.KEY_BUS_SERVICE_LIST_VALUE,
                    APP_MESSAGE_KEYS.KEY_BUS_SERVICE_LIST_END,
                    serviceList
                );


            }
        });
    },

    // gets the details for the service
    sendServiceDetails: function (stopId, service) {
        getBusTimings(stopId, function(error, record) {
            if (error) {
                console.log('Error getting bus timings');
                console.log(error);
            } else {

                const serviceDetails = parseForServiceDetails(record, service);

                const messageString = 'Bus ' + serviceDetails[RESPONSE_KEYS.serviceNo] + ': ' + serviceDetails[RESPONSE_KEYS.nextBus][RESPONSE_KEYS.estimatedArrival] + ' ' +  serviceDetails[RESPONSE_KEYS.nextBus][RESPONSE_KEYS.load];

                const dictionaryMessage = {
                    KEY_BUS_SERVICE_DETAILS: messageString
                };

                pebbleHelpers.pebbleSendMessage(dictionaryMessage);
            }
        })
    }
};

pebbleHelpers.addEventListener.onReady(function (event) {
    busTimings.sendServicesList(83139);
});

pebbleHelpers.addEventListener.onAppMessage(function (event) {
    busTimings.sendServicesList(83139);
});
//
