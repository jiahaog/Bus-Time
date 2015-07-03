/**
 * Created by JiaHao on 18/6/15.
 */

var stateTracker = require('./../model/stateTracker');
var constants = require('./../constants/constants');
var messageSender = require('./../controller/messageSender');
var hasBusArrived = require('./../service/hasBusArrived');

function processAppMessage(event) {

    const payload = event[constants.MISC_KEYS.payload];

    for (var key in payload) {
        if (payload.hasOwnProperty(key)) {
            // value is an int
            var value = payload[key];

            if (key === constants.APP_MESSAGE_KEYS.KEY_BUS_SERVICE_LIST_START) {
                messageSender.sendServicesList(value);

            } else if (key === constants.APP_MESSAGE_KEYS.KEY_BUS_SERVICE_DETAILS_START) {
                // enter the services details page from services list
                console.log('Received request for service: ' + value);
                messageSender.sendServiceDetails(stateTracker.lastStopID, value);

            } else if (key === constants.APP_MESSAGE_KEYS.KEY_BUS_SERVICE_DETAILS_END) {
                // going back to services list from details window



            } else if (key === constants.APP_MESSAGE_KEYS.KEY_BUS_SERVICE_LIST_END) {
                // going back to bus stop list from bus services list

            } else if (key === constants.APP_MESSAGE_KEYS.KEY_BUS_NOTIFICATION) {

                // message format {stop_id}|{service_no}
                var splitDetails = value.split(constants.MESSAGE_DELIMITER);
                var stopId = splitDetails[0];
                var serviceNo = splitDetails[1];
                console.log('RECEIVED NOTIFICATION MESSAGE _' + stopId + '_' + serviceNo + '_');

                hasBusArrived(stopId, serviceNo);

            }
        }
    }
}

module.exports = {
    processAppMessage: processAppMessage
};