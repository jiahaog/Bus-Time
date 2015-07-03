/**
 * Created by JiaHao on 2/7/15.
 */


var recordCache = require('./../model/recordCache');
var recordParser = require('./../process_data/recordParser');
var pebbleHelpers = require('./../pebbleHelpers');
var constants = require('./../constants/constants');
var messageSender = require('./../controller/messageSender');

const ARRIVAL_THRESHOLD = 60 * 1000; // 1 min

/**
 * Sends a notification to the watch if the bus has arrived
 * @param stopId
 * @param serviceNo
 */
function hasBusArrived(stopId, serviceNo) {
    console.log("Checking for _" + stopId +"_" + serviceNo + "_");
    recordCache.getBusTimings(stopId, serviceNo, false, function (error, record) {
        if (error) {
            console.log('Error getting bus timings');
        } else {
            var serviceDetails = recordParser.parseForServiceDetails(record, serviceNo, true);

            var timeToNextBusArrival = serviceDetails[constants.RESPONSE_KEYS.nextBus][constants.RESPONSE_KEYS.estimatedArrival];

            if (timeToNextBusArrival === null) {
                // if the timeToNextBusArrival is null, no data is available and we want to cancel the app timer
                messageSender.sendNotificationStatus(stopId, serviceNo);
            } else {

                // check if the time is within the threshold
                if (timeToNextBusArrival < ARRIVAL_THRESHOLD) {
                    // send the notification and cancel the app message
                    pebbleHelpers.sendNotification('Bus Time', 'Bus ' + serviceNo + " is arriving!");
                    messageSender.sendNotificationStatus(stopId, serviceNo);
                }
            }

        }
    });
}

module.exports = hasBusArrived;