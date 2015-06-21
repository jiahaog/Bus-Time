/**
 * Created by JiaHao on 8/6/15.
 */

var messageSender = require('./controller/messageSender');
var messageProcessor = require('./controller/messageProcessor');
var recordCache = require('./model/recordCache');
var pebbleHelpers = require('./pebbleHelpers');

// when the app is launched get the location and send nearby bus stops to the watch
pebbleHelpers.addEventListener.onReady(function () {
    // restore bus data
    console.log('=== B U S  T I M E ===');
    recordCache.restoreCache();
    messageSender.sendBusStopList();

});

pebbleHelpers.addEventListener.onAppMessage(function (event) {
    messageProcessor.processAppMessage(event);
});