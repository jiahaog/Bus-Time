/**
 * Created by JiaHao on 8/6/15.
 */

var pebbleHelpers = require('./pebbleHelpers');
var stateTracker = require('./model/stateTracker');
var busServiceObserver = require('./service/busServiceObserver');
var busServiceNotifier = require('./service/busServiceNotifier');
var recordCache = require('./model/recordCache');
var messageSender = require('./controller/messageSender');
var constants = require('./constants/constants');

function processReceivedMessage(event) {

    const payload = event[constants.MISC_KEYS.payload];

    for (var key in payload) {
        if (payload.hasOwnProperty(key)) {
            // value is an int
            var value = payload[key];

            if (key === constants.APP_MESSAGE_KEYS.KEY_BUS_SERVICE_LIST_START) {
                stateTracker.lastAppMessageTime = Date.now();
                busServiceObserver.watchBusStop(value);

            } else if (key === constants.APP_MESSAGE_KEYS.KEY_BUS_SERVICE_DETAILS_START) {
                // enter the services details page from services list

                stateTracker.lastAppMessageTime = Date.now();
                console.log('Received request for service: ' + value);

                // stop watching the bus services list
                busServiceObserver.stopWatchingBusStop();

                // watch the service details
                busServiceObserver.watchBusServiceDetails(stateTracker.lastStopID, value);

            } else if (key === constants.APP_MESSAGE_KEYS.KEY_BUS_SERVICE_DETAILS_END) {
                // going back to services list from details window

                stateTracker.lastAppMessageTime = Date.now();

                // stop watching the service details
                busServiceObserver.stopWatchingBusServiceDetails();

                // start watching the bus services list
                busServiceObserver.watchBusStop(stateTracker.lastStopID);

            } else if (key === constants.APP_MESSAGE_KEYS.KEY_BUS_SERVICE_LIST_END) {
                // going back to bus stop list from bus services list
                stateTracker.lastAppMessageTime = Date.now();

                // stop watching the bus services list
                busServiceObserver.stopWatchingBusStop();

            } else if (key === constants.APP_MESSAGE_KEYS.KEY_BUS_NOTIFICATION) {
                stateTracker.lastAppMessageTime = Date.now();

                // handle notification

                // message format {set_or_cancel_notification}|{stop_id}|{service_no}
                var splitDetails = value.split(constants.MESSAGE_DELIMITER);
                var startNotification = parseInt(splitDetails[0]);  // parseint here so we can do a if (setorcancel)
                var stopId = splitDetails[1];
                var serviceNo = splitDetails[2];

                if (startNotification) {
                    busServiceNotifier.startNotification(stopId, serviceNo);
                } else {
                    busServiceNotifier.stopNotification(stopId, serviceNo);
                }
            }
        }
    }
}

// when the app is launched get the location and send nearby bus stops to the watch
pebbleHelpers.addEventListener.onReady(function () {
    // restore bus data
    console.log('=== B U S  T I M E ===');
    recordCache.restoreCache();
    messageSender.sendBusStopList();

});

pebbleHelpers.addEventListener.onAppMessage(function (event) {
    processReceivedMessage(event);
});