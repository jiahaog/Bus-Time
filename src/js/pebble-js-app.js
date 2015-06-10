(function e(t,n,r){function s(o,u){if(!n[o]){if(!t[o]){var a=typeof require=="function"&&require;if(!u&&a)return a(o,!0);if(i)return i(o,!0);var f=new Error("Cannot find module '"+o+"'");throw f.code="MODULE_NOT_FOUND",f}var l=n[o]={exports:{}};t[o][0].call(l.exports,function(e){var n=t[o][1][e];return s(n?n:e)},l,l.exports,e,t,n,r)}return n[o].exports}var i=typeof require=="function"&&require;for(var o=0;o<r.length;o++)s(r[o]);return s})({1:[function(require,module,exports){
module.exports = {
    MY_TRANSPORT_KEYS: {
        AccountKey: 'cogWcDwIgUCTzEI06Vrfpg==',
        UniqueUserId: 'b460e8e7-e015-4c6b-9ad1-b08d4498f3d4'
    }
};

},{}],2:[function(require,module,exports){
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
 * @param stopId
 * @param [serviceNo] if this argument is provided, it will perform another check to see if the service already arrived
 * @returns {*} null if not found
 */
function getValidRecordFromStore(stopId, serviceNo) {
    for (var i = 0; i < store.length; i++) {
        var storeRecord = store[i];

        var recordWithinUpdateThreshold = (Date.now() - storeRecord[RESPONSE_KEYS.time]) > REFRESH_THRESHOLD;

        var sameStopId = storeRecord[RESPONSE_KEYS.stopId] === stopId;

        var serviceNeedsRefresh = false;

        if (sameStopId && serviceNo) {

            var services = storeRecord[RESPONSE_KEYS.services];

            for (var j = 0; j < services.length; j++) {
                var currentServiceRecord = services[j];

                var nextBusArrivalTime = currentServiceRecord[RESPONSE_KEYS.nextBus][RESPONSE_KEYS.estimatedArrival];

                serviceNeedsRefresh = getTimeToArrival(nextBusArrivalTime) ===  null;

            }

        }

        if (sameStopId && recordWithinUpdateThreshold && !serviceNeedsRefresh) {
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
        

        // convert desiredServiceNo to string, as currentService is a string
        if (currentService[RESPONSE_KEYS.serviceNo] === desiredServiceNo.toString()) {
            // mutate the arrival timing to time from now
            var nextBus = currentService[RESPONSE_KEYS.nextBus];
            nextBus[RESPONSE_KEYS.estimatedArrival] = getTimeToArrival(nextBus[RESPONSE_KEYS.estimatedArrival]);

            var subsequentBus = currentService[RESPONSE_KEYS.subsequentBus];
            subsequentBus[RESPONSE_KEYS.estimatedArrival] = getTimeToArrival(subsequentBus[RESPONSE_KEYS.estimatedArrival]);

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
 * @param arrivalString utc date string
 * @returns {string} e.g. '1m 20s', null if negative
 */
function getTimeToArrival(arrivalString) {
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
                        console.log('Message sent!');
                    }
                });
            }
        })
    }
};

pebbleHelpers.addEventListener.onReady(function (event) {
    busTimings.sendServicesList(83139);
});

function processReceivedMessage(event) {

    const payload = event[PEBBLE_KEYS.payload];
    for (var key in payload) {
        if (payload.hasOwnProperty(key)) {
            var value = payload[key];

            if (key === APP_MESSAGE_KEYS.KEY_BUS_SERVICE_DETAILS_START) {
                console.log('Received request for service: ' + value);
                busTimings.sendServiceDetails(83139, value);
            }
        }
    }
}
pebbleHelpers.addEventListener.onAppMessage(function (event) {
    
    processReceivedMessage(event);

});
//

},{"./config":1,"./pebbleHelpers":3}],3:[function(require,module,exports){
/**
 * Created by JiaHao on 9/6/15.
 */

/**
 * Appends query paramteres to a url
 *
 * @param {obj} url keys will be used as the query keys, and values as the value
 * @param params
 */
function appendParamsToUrl(url, params) {
    var result = url + '?';
    for (var paramKey in params) {

        if (params.hasOwnProperty(paramKey)) {
            var paramValue = params[paramKey];
            result += paramKey + '=' + paramValue + '&';
        }

    }
    
    return result;
}

/**
 * @typedef {Object} appMessage
 * @property {string} key
 * @property {string} message
 */

/**
 * @callback sendMessageCallback
 * @param error
 */

/**
 * Sends a message to the client
 * @param {appMessage} dictionaryMessage
 * @param {sendMessageCallback} callback
 */
function sendMessage(dictionaryMessage, callback) {
    Pebble.sendAppMessage(dictionaryMessage, function () {
        callback(undefined);
    }, function (error) {
        callback(error);
    })


}

/**
 * Helper method to send a list of messages over sequentially
 *
 * Messages will start with the startKey provided of an arbitrary value,
 * followed by the messages in the array keyed to the valueKey, and
 * a last arbitrary message keyed to the endKey.
 *
 * @param startKey
 * @param valueKey
 * @param endKey
 * @param messages
 */
function sendMessageStream(startKey, valueKey, endKey, messages) {
    console.log('Sending messages stream');
    const ARBITRARY_MESSAGE = 'm';

    // sets up
    const startMessage = {};
    startMessage[startKey] = ARBITRARY_MESSAGE;

    const endMessage = {};
    endMessage[endKey] = ARBITRARY_MESSAGE;

    // push start message
    var messagesToSend = [startMessage];

    // pushes messages
    for (var i = 0; i < messages.length; i++) {

        var currentMessage = messages[i];
        var currentMessageObj = {};
        currentMessageObj[valueKey] = currentMessage.toString();
        messagesToSend.push(currentMessageObj);

    }

    // push end message
    messagesToSend.push(endMessage);

    /**
     * Recursively sends the message sequentially
     */
    (function sendMessageRecursively() {

        var messageToSend = messagesToSend.shift();

        if (messageToSend) {

            sendMessage(messageToSend, function (error) {
                if (error) {
                    console.log('Error sending messages to pebble!');
                    console.log(error);
                } else {
                    sendMessageRecursively();
                }
            });

        } else {
            console.log('Message stream sent to Pebble successfully!');
        }
    })();
}


function xhrRequest(url, type, headers, params, callback) {
    console.log('Making request...');

    // todo implement some error catching here
    const urlWithParams = appendParamsToUrl(url, params);

    var xhr = new XMLHttpRequest();
    xhr.onload = function() {
        callback(this.responseText);
    };
    xhr.open(type, urlWithParams);

    for (var key in headers) {

        if (headers.hasOwnProperty(key)) {
            xhr.setRequestHeader(key, headers[key]);
        }
    }

    xhr.send();
}

/**
 * @callback eventListenerCallback
 * @param [event] Mostly unused argument, sample: {"type":"ready","bubbles":false,"cancelable":false,"defaultPrevented":false,"target":null,"currentTarget":null,"eventPhase":2,"_aborted":false}
 */

/**
 * Functions that facilitate adding eventListener callbacks to pebble
 *
 * @param {eventListenerCallback} callback
 */
var addEventListener = {
    onReady: function (callback) {
        Pebble.addEventListener('ready', function(event) {
            console.log('PebbleKit JS ready!');
            callback(event);
        });
    },

    onAppMessage: function(callback) {
        Pebble.addEventListener('appmessage', function(event) {
            console.log('AppMessage received!');
            callback(event)
        });
    }
};

module.exports = {
    xhrRequest: xhrRequest,
    sendMessage: sendMessage,
    sendMessageStream: sendMessageStream,
    addEventListener: addEventListener
};

if (require.main === module) {
    const url = 'http://datamall2.mytransport.sg/ltaodataservice/BusArrival';
    const params = {
        AccountKey: 'cogWcDwIgUCTzEI06Vrfpg==',
        UniqueUserId: 'b460e8e7-e015-4c6b-9ad1-b08d4498f3d4'
    };

    const appended = appendParamsToUrl(url, params);
    console.log(appended);

}

},{}]},{},[2]);
