/**
 * Created by JiaHao on 18/6/15.
 */

var messageSender = require('./../controller/messageSender');
var statTracker = require('./../model/stateTracker');
var constants = require('./../constants/constants');

var watchBusStopIntervalId;
var watchBusServiceIntervalId;


if (constants.RELEASE_MODE) {
    var REFRESH_INTERVAL = 60 * 1000; // 1 min
} else {
    REFRESH_INTERVAL = 10 * 1000; // 10 sec
}

//const REFRESH_TIMEOUT = 10*60*1000; // 10 min
const REFRESH_TIMEOUT = 15 * 60 * 1000; // 10 min


function watchBusStop(stopId) {

    function sendAndManageServicesList(stopId) {
        // if app has not timed out
        if (!checkIfAppTimeout()) {
            messageSender.sendServicesList(stopId, function (error) {

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

    statTracker.lastStopID = stopId;

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
            messageSender.sendServiceDetails(stopId, serviceNo, function (error) {
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

    if (currentTime - statTracker.lastAppMessageTime > REFRESH_TIMEOUT) {
        clearInterval(watchBusStopIntervalId);
        clearInterval(watchBusServiceIntervalId);
        messageSender.sendErrorCode(constants.ERROR_CODES.APP_TIMEOUT);

        console.log('App timeout. Cleared poll intervals');
        return true;
    } else {
        return false;
    }
}

function stopWatchingBusStop() {
    clearInterval(watchBusStopIntervalId);
}

function stopWatchingBusServiceDetails() {
    clearInterval(watchBusServiceIntervalId);
}

module.exports = {
    watchBusStop: watchBusStop,
    watchBusServiceDetails: watchBusServiceDetails,
    stopWatchingBusStop: stopWatchingBusStop,
    stopWatchingBusServiceDetails: stopWatchingBusServiceDetails
};