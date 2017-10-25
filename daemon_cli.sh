#!/bin/bash


case "$1" in
  start)
  ./build/node 3000 127.0.0.1 2001 1 &
  ./build/node 3001 127.0.0.1 3000 2 &
  ./build/node 3002 127.0.0.1 3001 4 &
  ;;
  stats)
  echo "45:NAN:GET_STATS:REQ:" > /dev/udp/127.0.0.1/3000
  echo "45:NAN:GET_STATS:REQ:" > /dev/udp/127.0.0.1/3001
  echo "45:NAN:GET_STATS:REQ:" > /dev/udp/127.0.0.1/3002
  ;;
  put)
  echo "45:NAN:QUERY_CHORD_PUT:REQ:$2" > /dev/udp/127.0.0.1/3001
  ;;
  get)
  echo "45:NAN:QUERY_CHORD_GET:REQ:$2" > /dev/udp/127.0.0.1/3001
  ;;
  stop)
  echo "45:NAN:DISCONNECT:REQ:" > /dev/udp/127.0.0.1/3000
  echo "45:NAN:DISCONNECT:REQ:" > /dev/udp/127.0.0.1/3001
  echo "45:NAN:DISCONNECT:REQ:" > /dev/udp/127.0.0.1/3002
  ;;
  *)
  echo $"Usage: $0 {start|put|get|stop|stats}"
  ;;
esac