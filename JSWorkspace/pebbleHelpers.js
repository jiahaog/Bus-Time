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
