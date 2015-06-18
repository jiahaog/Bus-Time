/**
 * Created by JiaHao on 18/6/15.
 */

var recordCache = require('./recordCache');
var recordParser = require('./recordParser');
var pebbleHelpers = require('./pebbleHelpers');
var constants = require('./constants');

const SLOW_UPDATE_INTERVAL = 60 * 1000;
const FAST_UPDATE_THRESHOLD = 3 * 60 * 1000;
const FAST_UPDATE_INTERVAL = 30 * 1000;


// threshold for the notification to be sent
const ARRIVAL_THRESHOLD = 60 * 1000; // 1 min

function nextIntervalTime(currentTime) {
    if (currentTime < FAST_UPDATE_THRESHOLD) {
        return FAST_UPDATE_INTERVAL;
    } else {
        return SLOW_UPDATE_INTERVAL;
    }
}


var busNotificationStore = {

    store: [],

    startNotification: function(stopId, serviceNo) {
        console.log("Starting notification for " + stopId + "|" + serviceNo);
        var notification = this.findNotification(stopId, serviceNo);

        if (!notification) {
            notification = new BusNotification(stopId, serviceNo);

            this.store.push(notification);
        }

        notification.start();
    },

    stopNotification: function(stopId, serviceNo) {
        console.log("Stopping notification for " + stopId + "|" + serviceNo);
        var notification = this.findNotification(stopId, serviceNo);

        if (notification) {
            notification.stop();
        }
    },

    findNotification: function (stopId, serviceNo) {
        var store = this.store;

        for (var i = 0; i < store.length; i++) {
            var notification = store[i];

            if (notification.stopId === stopId && notification.serviceNo === serviceNo) {
                return notification;
            }
        }
        return null;
    }
};

function BusNotification(stopId, serviceNo) {
    this.stopId = stopId;
    this.serviceNo = serviceNo;
    this.notificationId = null;
}

BusNotification.prototype = {
    constructor: BusNotification,
    start: function () {
        this.update();
    },

    update: function () {
        var instance = this;
        if (instance.notificationId) {
            clearInterval(instance.notificationId);
        }

        recordCache.getBusTimings(instance.stopId, instance.serviceNo, false, function (error, record) {
            if (error) {
                console.log('Error getting bus timings');
            } else {
                var serviceDetails = recordParser.parseForServiceDetails(record, instance.serviceNo, true);

                var timeToNextBusArrival = serviceDetails[constants.RESPONSE_KEYS.nextBus][constants.RESPONSE_KEYS.estimatedArrival];

                if (timeToNextBusArrival < ARRIVAL_THRESHOLD) {
                    pebbleHelpers.sendNotification('Bus Time', 'Bus ' + instance.serviceNo + " is arriving!");
                } else {
                    instance.notificationId = setTimeout(function() {
                        instance.update();
                    }, nextIntervalTime(timeToNextBusArrival))
                }
            }
        });
    },

    stop: function() {
        var notificationId = this.notificationId;
        if (notificationId) {
            clearTimeout(notificationId);
        }
    }
};

module.exports = busNotificationStore;