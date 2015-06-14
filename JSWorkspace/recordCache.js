/**
 * Created by JiaHao on 14/6/15.
 */

var constants = require('./constants');
var recordParser = require('./recordParser');
var pebbleHelpers = require('./pebbleHelpers');

var store;
const RECORD_LIVE_DURATION = 60*60*1000; // in ms (todo temporarily set to 60 mins)

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

        var sameStopId = storeRecord[constants.RESPONSE_KEYS.stopId] === stopId.toString();
        var recordWithinUpdateThreshold = (Date.now() - storeRecord[constants.RESPONSE_KEYS.time]) < RECORD_LIVE_DURATION;
        var serviceNeedsRefresh = false;

        // if serviceNo is provided as an argument
        if (serviceNo) {

            var services = storeRecord[constants.RESPONSE_KEYS.services];

            // searches through services for the desired service
            for (var j = 0; j < services.length; j++) {
                var currentServiceRecord = services[j];

                if (currentServiceRecord[constants.RESPONSE_KEYS.serviceNo] === serviceNo.toString()) {

                    // checks if the time to arrival is still valid
                    var nextBusArrivalTime = currentServiceRecord[constants.RESPONSE_KEYS.nextBus][constants.RESPONSE_KEYS.estimatedArrival];
                    serviceNeedsRefresh = recordParser.getTimeToArrival(nextBusArrivalTime) ===  null;
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

function restoreCache() {
    store = pebbleHelpers.storageLocal.readObject(constants.MISC_KEYS.data) || [];
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

        pebbleHelpers.xhrRequest(constants.API_URL, 'GET', constants.REQUEST_HEADERS, params, function(error, responseText) {
            //console.log('StopId ' + stopId + ' request completed');

            if (error) {
                const errorString = 'Error making xhr request';
                console.log(errorString);
                callback(errorString, undefined);


            } else {

                record = JSON.parse(responseText);
                // add the time of the query in
                record[constants.RESPONSE_KEYS.time] = Date.now();

                // cache it in the store
                store.push(record);
                pebbleHelpers.storageLocal.saveObject(constants.MISC_KEYS.data, store);
                callback(undefined, record);
            }

        });
    }
}



module.exports = {
    getValidRecordFromStore: getValidRecordFromStore,
    restoreCache: restoreCache,
    getBusTimings: getBusTimings
};