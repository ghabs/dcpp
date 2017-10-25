//TODO
//TURN all strings into defined messages that are made in this class and serialized
//Add typedefs for possible commands received

#ifndef MESSAGES_H
#define MESSAGES_H

namespace messages {

typedef enum {
  join, handshake, stabilize, disconnect
} Requests;


//ex. if message received is handshake, defined response

} //messages
#endif
