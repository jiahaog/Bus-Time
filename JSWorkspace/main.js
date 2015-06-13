/**
* Created by JiaHao on 8/6/15.
*/

var pebbleHelpers = require('./pebbleHelpers');
var config = require('./config');
var busStops = require('./busStops');

const API_URL = 'http://datamall2.mytransport.sg/ltaodataservice/BusArrival';

const REQUEST_HEADERS = {
    AccountKey: config.MY_TRANSPORT_KEYS.AccountKey,
    UniqueUserId: config.MY_TRANSPORT_KEYS.UniqueUserId,
    accept: 'application/json'
};

const TEST_RESPONSE = '{"odata.metadata":"http://datamall2.mytransport.sg/ltaodataservice/$metadata#BusArrival/@Element","BusStopID":"83139","Services":[{"ServiceNo":"15","Status":"In Operation","Operator":"SBST","NextBus":{"EstimatedArrival":"2015-06-09T14:25:49+00:00","Load":"Standing Available","Feature":"WAB"},"SubsequentBus":{"EstimatedArrival":"2015-06-09T13:56:32+00:00","Load":"Seats Available","Feature":"WAB"}},{"ServiceNo":"155","Status":"In Operation","Operator":"SBST","NextBus":{"EstimatedArrival":"2015-06-09T13:47:03+00:00","Load":"Seats Available","Feature":"WAB"},"SubsequentBus":{"EstimatedArrival":"2015-06-09T14:01:57+00:00","Load":"Seats Available","Feature":"WAB"}}]}';
const REFRESH_THRESHOLD = 60*60*1000; // in ms (temporarily set to 60 mins)

// Keys so we don't make typos
const APP_MESSAGE_KEYS = {

    KEY_BUS_SERVICE_LIST_START: 'KEY_BUS_SERVICE_LIST_START',
    KEY_BUS_SERVICE_LIST_VALUE: 'KEY_BUS_SERVICE_LIST_VALUE',
    KEY_BUS_SERVICE_LIST_END: 'KEY_BUS_SERVICE_LIST_END',

    KEY_BUS_SERVICE_DETAILS_START: 'KEY_BUS_SERVICE_DETAILS_START',
    KEY_BUS_SERVICE_DETAILS_VALUE: 'KEY_BUS_SERVICE_DETAILS_VALUE',
    KEY_BUS_SERVICE_DETAILS_END: 'KEY_BUS_SERVICE_DETAILS_END',

    KEY_BUS_STOP_LIST_START: 'KEY_BUS_STOP_LIST_START',
    KEY_BUS_STOP_LIST_VALUE: 'KEY_BUS_STOP_LIST_VALUE',
    KEY_BUS_STOP_LIST_END: 'KEY_BUS_STOP_LIST_END',

    KEY_ERROR: 'KEY_ERROR'

};

// misc keys to access received appmessage
const MISC_KEYS = {
    payload: 'payload', // get object from received app message
    data: 'data' // key to store objects in localstorage
};


// keys to access DataMall api response
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

const ERROR_CODES = {
    NETWORK_ERROR: 1,
    NO_SERVICES_OPERATIONAL: 2,
    // 3 is used by the watch to handle error in bluetooth
    LOCATION_ERROR: 4
};

// todo figure out a way to cache these things to disk
var store;
var lastBusStopsIDsSent = [];
var lastStopID;

/**
 * Queries the store for a valid record that falls within the threshold and has the same stopId
 * @param {number} stopId
 * @param {number} [serviceNo] if this argument is provided, it will perform another check to see if the service already arrived
 * @returns {*} null if not found
 */
function getValidRecordFromStore(stopId, serviceNo) {

    // iterate through records from newest to oldest
    for (var i = store.length - 1; i >= 0; i--) {
        var storeRecord = store[i];

        var sameStopId = storeRecord[RESPONSE_KEYS.stopId] === stopId.toString();
        var recordWithinUpdateThreshold = (Date.now() - storeRecord[RESPONSE_KEYS.time]) < REFRESH_THRESHOLD;
        var serviceNeedsRefresh = false;

        // if serviceNo is provided as an argument
        if (serviceNo) {

            var services = storeRecord[RESPONSE_KEYS.services];

            // searches through services for the desired service
            for (var j = 0; j < services.length; j++) {
                var currentServiceRecord = services[j];

                if (currentServiceRecord[RESPONSE_KEYS.serviceNo] === serviceNo.toString()) {

                    // checks if the time to arrival is still valid
                    var nextBusArrivalTime = currentServiceRecord[RESPONSE_KEYS.nextBus][RESPONSE_KEYS.estimatedArrival];
                    serviceNeedsRefresh = getTimeToArrival(nextBusArrivalTime) ===  null;
                }
            }
        }

        // break if the same bus stop is found, as the entry will be the newest
        if (sameStopId) {
            break;
        }
    }

    if (sameStopId && recordWithinUpdateThreshold && !serviceNeedsRefresh) {
        return storeRecord;
    }

    // todo cleanup expired records
    //store = [];
    return null;
}
/**
 * @callback responseCallback
 * @param error
 * @param {object} record object appended with the current time
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

        pebbleHelpers.xhrRequest(API_URL, 'GET', REQUEST_HEADERS, params, function(error, responseText) {
            //console.log('StopId ' + stopId + ' request completed');

            if (error) {
                console.log('Error making request');
                sendErrorCode(ERROR_CODES.NETWORK_ERROR);

            } else {

                record = JSON.parse(responseText);
                // add the time of the query in
                record[RESPONSE_KEYS.time] = Date.now();

                // cache it in the store
                store.push(record);
                pebbleHelpers.storageLocal.saveObject(MISC_KEYS.data, store);
                callback(undefined, record);
            }

        });
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
 * @return {parsedServiceDetailsResult} or null if service not found
 */
function parseForServiceDetails(record, desiredServiceNo) {
    const services = record[RESPONSE_KEYS.services];

    // iterate through and find the correct service
    for (var i = 0; i < services.length; i++) {
        var currentService = services[i];

        // convert desiredServiceNo to string, as currentService is a string
        if (currentService[RESPONSE_KEYS.serviceNo] === desiredServiceNo.toString()) {

            // copies the next buses
            var nextBus = pebbleHelpers.cloneObject(currentService[RESPONSE_KEYS.nextBus]);
            var subsequentBus = pebbleHelpers.cloneObject(currentService[RESPONSE_KEYS.subsequentBus]);

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

    return null;
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
            var nextBusArrivalTime = getTimeToArrival(currentService[RESPONSE_KEYS.nextBus][RESPONSE_KEYS.estimatedArrival]) || "Arr.";
            result.push(currentService[RESPONSE_KEYS.serviceNo] + ',' + nextBusArrivalTime);
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
    // todo WHAT IS THIS
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

/**
 * Gets the location of the watch and sends nearby bus stops to the watch
 */
function processLocation() {
    pebbleHelpers.getLocation(function (error, position) {
        if (error) {
            console.log('location error (' + error.code + '): ' + error.message);
            sendErrorCode(ERROR_CODES.LOCATION_ERROR);
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
                    busStop[busStops.CLOSEST_BUS_STOP_KEYS.description] + ',' +
                    busStop[busStops.CLOSEST_BUS_STOP_KEYS.road] + ',' +
                    busStop[busStops.CLOSEST_BUS_STOP_KEYS.stopId]);

                stopIds.push(busStop[busStops.CLOSEST_BUS_STOP_KEYS.stopId]);
            }

            // sends the message to the watch
            pebbleHelpers.sendMessageStream(
                APP_MESSAGE_KEYS.KEY_BUS_STOP_LIST_START,
                APP_MESSAGE_KEYS.KEY_BUS_STOP_LIST_VALUE,
                APP_MESSAGE_KEYS.KEY_BUS_STOP_LIST_END,
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
    dictionaryMessage[APP_MESSAGE_KEYS.KEY_ERROR] = code;

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
        getBusTimings(stopId, undefined, function (error, record) {
            if (error) {
                console.log('Error getting bus timings');
                console.log(error);
            } else {

                const serviceList = parseForServicesList(record);

                if (serviceList.length > 0) {
                    pebbleHelpers.sendMessageStream(
                        APP_MESSAGE_KEYS.KEY_BUS_SERVICE_LIST_START,
                        APP_MESSAGE_KEYS.KEY_BUS_SERVICE_LIST_VALUE,
                        APP_MESSAGE_KEYS.KEY_BUS_SERVICE_LIST_END,
                        serviceList
                    );
                } else {
                    sendErrorCode(ERROR_CODES.NO_SERVICES_OPERATIONAL);
                }



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
                //console.log(JSON.stringify(record));
                const serviceDetails = parseForServiceDetails(record, serviceNo);

                var messageString;
                if (serviceDetails) {
                    messageString = 'Bus ' + serviceDetails[RESPONSE_KEYS.serviceNo] + ': ' + serviceDetails[RESPONSE_KEYS.nextBus][RESPONSE_KEYS.estimatedArrival] + ' ' +  serviceDetails[RESPONSE_KEYS.nextBus][RESPONSE_KEYS.load];
                } else {
                    messageString = 'Service not found'
                }

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

// when the app is launched get the location and send nearby bus stops to the watch
pebbleHelpers.addEventListener.onReady(function (event) {
    // restore bus data
    store = pebbleHelpers.storageLocal.readObject(MISC_KEYS.data) || [];

    processLocation();

});

pebbleHelpers.addEventListener.onAppMessage(function (event) {
    processReceivedMessage(event);
});

function processReceivedMessage(event) {

    const payload = event[MISC_KEYS.payload];

    for (var key in payload) {
        if (payload.hasOwnProperty(key)) {
            // value is an int
            var value = payload[key];

            if (key === APP_MESSAGE_KEYS.KEY_BUS_SERVICE_LIST_START) {
                var stopId = lastBusStopsIDsSent[value];
                console.log('Received request for service list for stopID: ' + stopId);
                lastStopID = stopId;
                busTimings.sendServicesList(stopId);

            } else if (key === APP_MESSAGE_KEYS.KEY_BUS_SERVICE_DETAILS_START) {
                console.log('Received request for service: ' + value);
                busTimings.sendServiceDetails(lastStopID, value);
            }
        }
    }
}
