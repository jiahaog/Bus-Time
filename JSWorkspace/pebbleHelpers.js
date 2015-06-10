/**
 * Created by JiaHao on 9/6/15.
 */

/**
 * Appends query paramteres to a url
 *
 * @param {obj} url keys will be used as the query keys, and values as the value
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

/**
 * @typedef {Object} appMessage
 * @property {string} key
 * @property {string} message
 */

/**
 * @callback sendMessageCallback
 * @param error
 */

/**
 * Sends a message to the client
 * @param {appMessage} dictionaryMessage
 * @param {sendMessageCallback} callback
 */
function sendMessage(dictionaryMessage, callback) {
    Pebble.sendAppMessage(dictionaryMessage, function () {
        callback(undefined);
    }, function (error) {
        callback(error);
    })


}

/**
 * Helper method to send a list of messages over sequentially
 *
 * Messages will start with the startKey provided of an arbitrary value,
 * followed by the messages in the array keyed to the valueKey, and
 * a last arbitrary message keyed to the endKey.
 *
 * @param startKey
 * @param valueKey
 * @param endKey
 * @param messages
 */
function sendMessageStream(startKey, valueKey, endKey, messages) {
    console.log('Sending messages stream');
    const ARBITRARY_MESSAGE = 'm';

    // sets up
    const startMessage = {};
    startMessage[startKey] = ARBITRARY_MESSAGE;

    const endMessage = {};
    endMessage[endKey] = ARBITRARY_MESSAGE;

    // push start message
    var messagesToSend = [startMessage];

    // pushes messages
    for (var i = 0; i < messages.length; i++) {

        var currentMessage = messages[i];
        var currentMessageObj = {};
        currentMessageObj[valueKey] = currentMessage.toString();
        messagesToSend.push(currentMessageObj);

    }

    // push end message
    messagesToSend.push(endMessage);

    /**
     * Recursively sends the message sequentially
     */
    (function sendMessageRecursively() {

        var messageToSend = messagesToSend.shift();

        if (messageToSend) {

            sendMessage(messageToSend, function (error) {
                if (error) {
                    console.log('Error sending messages to pebble!');
                    console.log(error);
                } else {
                    sendMessageRecursively();
                }
            });

        } else {
            console.log('Message stream sent to Pebble successfully!');
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
    sendMessage: sendMessage,
    sendMessageStream: sendMessageStream,
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
