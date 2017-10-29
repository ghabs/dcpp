import socket
import json
import collections
import time

class Pinger(object):
    """docstring for NodeParser."""
    def __init__(self, arg):
        super(NodeParser, self).__init__()
        self.arg = arg

    def pinger():
        UDP_IP = "127.0.0.1"
        # DEFINE THE PORTS
        ports = [3000, 3001, 3002, 3003]
        # Change this to be dynamic depending on network
        MESSAGE = "45:NAN:NAN:GET_STATS:REQ"

        sock = socket.socket(socket.AF_INET, # Internet
                             socket.SOCK_DGRAM) # UDP
        sock.settimeout(5)

        for p in ports:
            sock.sendto(MESSAGE.encode("UTF-8"), (UDP_IP, p))

        json_output = {
         "nodes": [], "links": []
        }

        ids = set()
        ids_present = set();
        msg = len(ports)
        timeout = 5
        timeout_start = time.time()
        while time.time() < timeout_start + timeout:
            # does this block appropriately
            try:
                data, addr = sock.recvfrom(1024)
            except Exception as e:
                print(e)
                break
            d = data.decode("UTF-8")
            proto = d.split(':')
            for i, n in enumerate(proto):
                if i == 0:
                    ids_present.add(n)
                else:
                    json_output['links'].append({"source": proto[0], "target": n, "value": 1})
                    ids.add(n)

        for i in ids:
            if i not in ids_present:
                json_output["nodes"].append({"id": i, "group": 10})
            else:
                json_output["nodes"].append({"id": i, "group": 1})

        with open('graph.json', 'w') as outfile:
            json.dump(json_output, outfile)
