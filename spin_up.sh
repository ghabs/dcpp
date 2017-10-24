#!/bin/bash

./build/node 3000 127.0.0.1 2001 1 &
./build/node 3001 127.0.0.1 3000 2 &
./build/node 3002 127.0.0.1 3001 4 &
