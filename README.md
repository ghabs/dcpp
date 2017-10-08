First project with C++, building a decentralized hash table.

Todo: Change this protocol to be serialized with protobufs

Todo: Reevaluate spacing
Protocol:
Message 0 - 3: Command
Message 4 - 1021: Optional Data
Message 1022: Additional message?
Message 1023: end


This shouldn't actually be prepended with character, it is just for testing.
Probably a better idea is use a standard RPC syntax.
Commands:
C1: Handshake
C2: Connect
C3: Disconnect
C4: Set Keyspace
C5: Put values
C6: Get values
C7: Current Time
C8:
C9: Ack
