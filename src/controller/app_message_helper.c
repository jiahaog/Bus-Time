#include "app_message_helper.h"


// helper method to parse string into an int and send it as an appmessage
void send_app_message_string(int key, char *message) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    // parse the message to a int at the same time
    dict_write_uint8(iter, key, atoi(message));

    // Send the message!
    app_message_outbox_send();
}

// helper method to send an int as an appmessage
void send_app_message_int(int key, int message) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    // parse the message to a int at the same time
    dict_write_uint8(iter, key, message);

    // Send the message!
    app_message_outbox_send();
}