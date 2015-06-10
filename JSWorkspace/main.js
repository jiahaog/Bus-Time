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
    subsequentBus: 'SubsequentBus',
    load: 'Load',
    feature: 'Feature',
    inOperation: 'In Operation',

    time: 'Time' // added key
};

const PEBBLE_KEYS = {
    payload: 'payload'
};


var store = [];

/**
 * Queries the store for a valid record that falls within the threshold and has the same stopId
 * @param {string} stopId
 * @param [serviceNo] if this argument is provided, it will perform another check to see if the service already arrived
 * @returns {*} null if not found
 */
function getValidRecordFromStore(stopId, serviceNo) {

    // iterate through records
    for (var i = store.length - 1; i >= 0; i--) {
        var storeRecord = store[i];

        var sameStopId = storeRecord[RESPONSE_KEYS.stopId] === stopId.toString();


        var recordWithinUpdateThreshold = (Date.now() - storeRecord[RESPONSE_KEYS.time]) < REFRESH_THRESHOLD;
        var serviceNeedsRefresh = false;

        if (serviceNo) {

            var services = storeRecord[RESPONSE_KEYS.services];

            for (var j = 0; j < services.length; j++) {
                var currentServiceRecord = services[j];

                if (currentServiceRecord[RESPONSE_KEYS.serviceNo] === serviceNo.toString()) {
                    console.log(JSON.stringify(currentServiceRecord));
                    var nextBusArrivalTime = currentServiceRecord[RESPONSE_KEYS.nextBus][RESPONSE_KEYS.estimatedArrival];
                    serviceNeedsRefresh = getTimeToArrival(nextBusArrivalTime) ===  null;

                }
            }
        }
        //
        //console.log('same stop id ' + sameStopId );
        //console.log('record within update threshold' + recordWithinUpdateThreshold);
        //console.log('service needs refresh' + serviceNeedsRefresh);
        if (sameStopId) {
            break;
        }
    }

    if (sameStopId && recordWithinUpdateThreshold && !serviceNeedsRefresh) {
        return storeRecord;
    }
    // clear array if not found
    // todo make it only clear the expired stopIds
    //store = [];
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
 * @param [serviceNo]
 * @param {responseCallback} callback
 */
function getBusTimings(stopId, serviceNo, callback) {

    var record = getValidRecordFromStore(stopId, serviceNo);

    if (record) {
        // if a valid record is found,
        console.log('Using cached record...');
        callback(undefined, record);

    } else {
        console.log('Cached record not valid, making query...');
        // make a query

        const params = {
            BusStopID: stopId
        };

        pebbleHelpers.xhrRequest(API_URL, 'GET', REQUEST_HEADERS, params, function(responseText) {
            //console.log('StopId ' + stopId + ' request completed');

            record = JSON.parse(responseText);
            // add the time of the query in
            record[RESPONSE_KEYS.time] = Date.now();

            // cache it in the store
            store.push(record);

            callback(undefined, record);
        });
    }
}

function cloneObject(obj) {
    return JSON.parse(JSON.stringify(obj));
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
        

        // convert desiredServiceNo to string, as currentService is a string
        if (currentService[RESPONSE_KEYS.serviceNo] === desiredServiceNo.toString()) {

            // copies the next buses
            var nextBus = cloneObject(currentService[RESPONSE_KEYS.nextBus]);
            var subsequentBus = cloneObject(currentService[RESPONSE_KEYS.subsequentBus]);

            // short circuit evaluation in case the tta is negative
            nextBus[RESPONSE_KEYS.estimatedArrival] = getTimeToArrival(nextBus[RESPONSE_KEYS.estimatedArrival]) || 'arriving';
            subsequentBus[RESPONSE_KEYS.estimatedArrival] = getTimeToArrival(subsequentBus[RESPONSE_KEYS.estimatedArrival]) || 'arriving';

            const serviceObject = {};
            serviceObject[RESPONSE_KEYS.serviceNo] = currentService[RESPONSE_KEYS.serviceNo];
            serviceObject[RESPONSE_KEYS.nextBus] = nextBus;
            serviceObject[RESPONSE_KEYS.subsequentBus] = subsequentBus;

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
 * This function is used in two situations:
 *  to check if the record is valid,
 *  and to format a string to be sent to the watch
 *
 *  Therefore, it is possible for the time to be negative and the result to be null, in the second case where the records are not updated
 *  on the server side
 *
 *
 * @param arrivalString utc date string
 * @returns {string} e.g. '1m 20s', null if negative
 */
function getTimeToArrival(arrivalString) {
    console.log('ARrivalstring: ' + arrivalString);
    if (arrivalString === 'null') {
        console.log('Unable to get parse arrival time');
        return arrivalString;
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


var busTimings = {

    // gets a list of services at the bus stop
    sendServicesList: function (stopId) {
        getBusTimings(stopId, undefined, function (error, record) {
            if (error) {
                console.log('Error getting bus timings');
                console.log(error);
            } else {

                const serviceList = parseForServicesList(record);
                pebbleHelpers.sendMessageStream(
                    APP_MESSAGE_KEYS.KEY_BUS_SERVICE_LIST_START,
                    APP_MESSAGE_KEYS.KEY_BUS_SERVICE_LIST_VALUE,
                    APP_MESSAGE_KEYS.KEY_BUS_SERVICE_LIST_END,
                    serviceList
                );


            }
        });
    },

    // gets the details for the service
    sendServiceDetails: function (stopId, serviceNo) {
        getBusTimings(stopId, serviceNo, function(error, record) {
            if (error) {
                console.log('Error getting bus timings');
                console.log(error);
            } else {

                const serviceDetails = parseForServiceDetails(record, serviceNo);
                const messageString = 'Bus ' + serviceDetails[RESPONSE_KEYS.serviceNo] + ': ' + serviceDetails[RESPONSE_KEYS.nextBus][RESPONSE_KEYS.estimatedArrival] + ' ' +  serviceDetails[RESPONSE_KEYS.nextBus][RESPONSE_KEYS.load];

                var dictionaryMessage = {};
                dictionaryMessage[APP_MESSAGE_KEYS.KEY_BUS_SERVICE_DETAILS_VALUE] = messageString;

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

pebbleHelpers.addEventListener.onReady(function (event) {
    busTimings.sendServicesList(96041);
});

function processReceivedMessage(event) {

    const payload = event[PEBBLE_KEYS.payload];
    for (var key in payload) {
        if (payload.hasOwnProperty(key)) {
            // value is an int
            var value = payload[key];
            if (key === APP_MESSAGE_KEYS.KEY_BUS_SERVICE_DETAILS_START) {
                console.log('Received request for service: ' + value);
                busTimings.sendServiceDetails(96041, value);
            }
        }
    }
}
pebbleHelpers.addEventListener.onAppMessage(function (event) {
    
    processReceivedMessage(event);

});
//
