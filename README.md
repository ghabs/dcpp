Experimenting with C++ and the distributed hash table chord protocol. A distributed hash table is a hash table split among several computers; in order to minimize the number of reshuffles that occur when a node drops in and out of the shared keyspace the hashes are distributed in a circle. When a ring drops out its predecessor takes responsibility for that new keyspace, ensuring that the content is still accessible.

Simple visual representation:

![chordvisual](https://i.imgur.com/ddblSMI.png)

To run use the cli.sh script which will spin up multiple nodes on the same computer.

Notes: There's several sleeps in the implementation that slow down the ring formation to better show off what is happening.
