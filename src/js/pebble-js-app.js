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

},{"./config":1,"./pebbleHelpers":3}],3:[function(require,module,exports){
/**
 * Created by JiaHao on 9/6/15.
 */

/**
 * @param url
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

function pebbleSendMessage(dictionaryMessage) {
    console.log('Sending message...');
    Pebble.sendAppMessage(dictionaryMessage, function (error) {
        console.log('Message sent to Pebble successfully!');
    }, function (error) {
        console.log('Error sending message to Pebble!');
        console.log(error);
    })
}

function pebbleSendMessageSequentially(startKey, valueKey, endKey, messages) {
    console.log('Sending messages sequentially');

    const MESSAGE_TOKEN = 'm';

    var firstMessageSent = false;
    var lastMessageSent = false;


    (function sendMessage() {
        //todo refactor this nonsence into a single array of key value objects
        var dictionaryMessage = {};
        var keyToSend;
        var messageToSend;

        // first message
        if (!firstMessageSent) {
            keyToSend = startKey;
            messageToSend = MESSAGE_TOKEN;
            firstMessageSent = true;
        } else {

            keyToSend = valueKey;
            messageToSend = messages.shift();

            // checks if any more messages are still pending
            if (!messageToSend) {

                // if no more messages pending and last message still not sent,
                // we make messageToSend a valid message
                if (!lastMessageSent) {
                    keyToSend = endKey;
                    messageToSend = MESSAGE_TOKEN;
                    lastMessageSent = true;
                }
            }

        }

        // if there are messages pending, send it
        if (messageToSend) {
            dictionaryMessage[keyToSend] = messageToSend.toString();

            Pebble.sendAppMessage(dictionaryMessage, function (error) {
                sendMessage();
            }, function (error) {
                console.log('Error sending messages to Pebble!');
                console.log(error);
            })
        } else {
            console.log('All messages sent to Pebble successfully!');
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
    pebbleSendMessage: pebbleSendMessage,
    pebbleSendMessageSequentially: pebbleSendMessageSequentially,
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
