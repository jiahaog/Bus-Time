/**
 * Created by JiaHao on 14/6/15.
 */

var config = require('./config');

// change this to false for testing, which will affect polling and record live intervals
const RELEASE_MODE = true;

const API_URL = 'http://datamall2.mytransport.sg/ltaodataservice/BusArrival';
const REQUEST_HEADERS = {
    AccountKey: config.MY_TRANSPORT_KEYS.AccountKey,
    UniqueUserId: config.MY_TRANSPORT_KEYS.UniqueUserId,
    accept: 'application/json'
};

// Keys so we don't make typos
const APP_MESSAGE_KEYS = {

    KEY_BUS_STOP_LIST_START: 'KEY_BUS_STOP_LIST_START',
    KEY_BUS_STOP_LIST_VALUE: 'KEY_BUS_STOP_LIST_VALUE',
    KEY_BUS_STOP_LIST_END: 'KEY_BUS_STOP_LIST_END',

    KEY_BUS_SERVICE_LIST_START: 'KEY_BUS_SERVICE_LIST_START',
    KEY_BUS_SERVICE_LIST_VALUE: 'KEY_BUS_SERVICE_LIST_VALUE',
    KEY_BUS_SERVICE_LIST_END: 'KEY_BUS_SERVICE_LIST_END',

    KEY_BUS_SERVICE_DETAILS_START: 'KEY_BUS_SERVICE_DETAILS_START',
    KEY_BUS_SERVICE_DETAILS_VALUE: 'KEY_BUS_SERVICE_DETAILS_VALUE',
    KEY_BUS_SERVICE_DETAILS_END: 'KEY_BUS_SERVICE_DETAILS_END',

    KEY_ERROR: 'KEY_ERROR'

};

// misc keys to access received appmessage
const MISC_KEYS = {
    payload: 'payload', // get object from received app message
    data: 'data' // key to store objects in localstorage
};

const MESSAGE_DELIMITER = '|';


// keys to access DataMall api response
const RESPONSE_KEYS = {
    metadata: 'odata.metadata',
    stopId: 'BusStopID',
    services: 'Services',
    serviceNo: 'ServiceNo',
    status: 'Status',
    operator: 'Operator',
    nextBus: 'NextBus',
    estimatedArrival: 'EstimatedArrival',
    subsequentBus: 'SubsequentBus',
    load: 'Load',
    feature: 'Feature',
    inOperation: 'In Operation',
    time: 'Time' // added key
};

const ERROR_CODES = {
    NETWORK_ERROR: 1,
    NO_SERVICES_OPERATIONAL: 2,
    // 3 is used by the watch to handle error in bluetooth
    LOCATION_ERROR: 4,
    APP_TIMEOUT: 5
};

module.exports = {
    API_URL: API_URL,
    REQUEST_HEADERS: REQUEST_HEADERS,
    APP_MESSAGE_KEYS: APP_MESSAGE_KEYS,
    MISC_KEYS: MISC_KEYS,
    MESSAGE_DELIMITER: MESSAGE_DELIMITER,
    RESPONSE_KEYS: RESPONSE_KEYS,
    ERROR_CODES: ERROR_CODES,
    RELEASE_MODE: RELEASE_MODE
};