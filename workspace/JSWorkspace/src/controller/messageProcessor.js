/**
 * Created by JiaHao on 18/6/15.
 */

var stateTracker = require('./../model/stateTracker');
//var busServiceObserver = require('./../service/busServiceObserver');
var constants = require('./../constants/constants');
var pebbleHelpers = require('./../pebbleHelpers.js');
var messageSender = require('./../controller/messageSender');
var hasBusArrived = require('./../service/hasBusArrived');

function processAppMessage(event) {

    const payload = event[constants.MISC_KEYS.payload];

    for (var key in payload) {
        if (payload.hasOwnProperty(key)) {
            // value is an int
            var value = payload[key];

            if (key === constants.APP_MESSAGE_KEYS.KEY_BUS_SERVICE_LIST_START) {
                stateTracker.lastAppMessageTime = Date.now();
                //busServiceObserver.watchBusStop(value);
                messageSender.sendServicesList(value);

            } else if (key === constants.APP_MESSAGE_KEYS.KEY_BUS_SERVICE_DETAILS_START) {
                // enter the services details page from services list

                stateTracker.lastAppMessageTime = Date.now();
                console.log('Received request for service: ' + value);

                //// stop watching the bus services list
                //busServiceObserver.stopWatchingBusStop();

                // watch the service details
                messageSender.sendServiceDetails(stateTracker.lastStopID, value);

                //busServiceObserver.watchBusServiceDetails(stateTracker.lastStopID, value);

            } else if (key === constants.APP_MESSAGE_KEYS.KEY_BUS_SERVICE_DETAILS_END) {
                // going back to services list from details window

                stateTracker.lastAppMessageTime = Date.now();

                // stop watching the service details
                //busServiceObserver.stopWatchingBusServiceDetails();

                // start watching the bus services list
                //busServiceObserver.watchBusStop(stateTracker.lastStopID);


            } else if (key === constants.APP_MESSAGE_KEYS.KEY_BUS_SERVICE_LIST_END) {
                // going back to bus stop list from bus services list
                stateTracker.lastAppMessageTime = Date.now();

                // stop watching the bus services list
                //busServiceObserver.stopWatchingBusStop();

            } else if (key === constants.APP_MESSAGE_KEYS.KEY_BUS_NOTIFICATION) {
                stateTracker.lastAppMessageTime = Date.now();

                // message format {stop_id}|{service_no}
                var splitDetails = value.split(constants.MESSAGE_DELIMITER);
                var stopId = splitDetails[0];
                var serviceNo = splitDetails[1];

                hasBusArrived(stopId, serviceNo);

            } else if (key == constants.APP_MESSAGE_KEYS.KEY_APP_ALIVE) {
                // we dont save the last app message time because this is sent automatically by the watch as a service
                if (value === 1) {
                    var dictionaryMessage = {};
                    dictionaryMessage[constants.APP_MESSAGE_KEYS.KEY_APP_ALIVE] = 1;
                    pebbleHelpers.sendMessage(dictionaryMessage, function (error) {
                        if (error) {
                            console.log('Error sending keep alive message to pebble');
                        }
                    });
                }
            }
        }
    }
}

module.exports = {
    processAppMessage: processAppMessage
};