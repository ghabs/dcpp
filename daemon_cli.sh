#!/bin/bash


case "$1" in
  start)
  ./build/node 3000 127.0.0.1 2006 1 &
  ./build/node 3001 127.0.0.1 3000 2 &
  ./build/node 3002 127.0.0.1 3001 4 &
  sleep 5
  ./build/node 3003 127.0.0.1 3002 7 &
  ./build/node 3004 127.0.0.1 3003 20 &
  sleep 5
  ./build/node 3005 127.0.0.1 3004 22 &
  ./build/node 3006 127.0.0.1 3005 40 &

  ;;
  stats)
  echo "45:NAN:NAN:GET_STATS:REQ:" > /dev/udp/127.0.0.1/3000
  echo "45:NAN:NAN:GET_STATS:REQ:" > /dev/udp/127.0.0.1/3001
  echo "45:NAN:NAN:GET_STATS:REQ:" > /dev/udp/127.0.0.1/3002
  echo "45:NAN:NAN:GET_STATS:REQ:" > /dev/udp/127.0.0.1/3003
  echo "45:NAN:NAN:GET_STATS:REQ:" > /dev/udp/127.0.0.1/3004
  echo "45:NAN:NAN:GET_STATS:REQ:" > /dev/udp/127.0.0.1/3005
  echo "45:NAN:NAN:GET_STATS:REQ:" > /dev/udp/127.0.0.1/3006
  ;;

  put)
  echo "2:127.0.0.1:3001:QUERY_CHORD_PUT:REQ:$2" > /dev/udp/127.0.0.1/3001
  ;;
  get)
  echo "2:127.0.0.1:3001:QUERY_CHORD_GET:REQ:$2" > /dev/udp/127.0.0.1/3001
  ;;
  stop)
  echo "45:NAN:NAN:DISCONNECT:REQ:" > /dev/udp/127.0.0.1/3000
  echo "45:NAN:NAN:DISCONNECT:REQ:" > /dev/udp/127.0.0.1/3001
  echo "45:NAN:NAN:DISCONNECT:REQ:" > /dev/udp/127.0.0.1/3002
  echo "45:NAN:NAN:DISCONNECT:REQ:" > /dev/udp/127.0.0.1/3003
  echo "45:NAN:NAN:DISCONNECT:REQ:" > /dev/udp/127.0.0.1/3004
  echo "45:NAN:NAN:DISCONNECT:REQ:" > /dev/udp/127.0.0.1/3005
  echo "45:NAN:NAN:DISCONNECT:REQ:" > /dev/udp/127.0.0.1/3006
  ;;
  *)
  echo $"Usage: $0 {start|put|get|stop|stats}"
  ;;
esac
