import socket
import json


class NodeParser(object):
    """docstring for NodeParser."""
    def __init__(self, arg):
        super(NodeParser, self).__init__()
        self.arg = arg


UDP_IP = "127.0.0.1"
# DEFINE THE PORTS
ports = [3000, 3001, 3002]
MESSAGE = "45:NAN:GET_STATS:REQ"

print "UDP target IP:", UDP_IP
print "UDP target port:", UDP_PORT
print "message:", MESSAGE

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP

for p in ports:
    sock.sendto(MESSAGE, (UDP_IP, p))

json_output = {
 "nodes": [], "links": []
}

msg = len(ports)
while (msg >= 0):
    # does this block appropriately?
    data, addr = sock.recvfrom(1024)
    proto = data.split(':')
    for i, n in enumerate(proto):
        if i == 0:
            json_output.nodes.append({"id": n, "group": 1});
        else:
            json_output.links.append({"source": proto[0], "target": n, "value": 1})
    msg = msg - 1

with open('graph.json', 'w') as outfile:
    json.dump(json_output, outfile)
