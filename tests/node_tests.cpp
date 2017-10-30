
#define BOOST_TEST_MAIN
#include "../node.h"
#include <boost/test/included/unit_test.hpp>
#include <string>

using namespace std;
struct TestFixture
{
  node::Node n;
  node::Node partner;

  TestFixture()
  : n(3000, "127.0.0.1", 2001, 1),
  partner(3001, "127.0.0.1", 3000, 2)
  {}

  ~TestFixture() = default;
};


BOOST_FIXTURE_TEST_SUITE(NodeTesting, TestFixture)
/*
BOOST_AUTO_TEST_CASE (keyspace_test)
{
  auto k = n.get_keyspace();
  BOOST_CHECK(k[0] == 0);
  BOOST_CHECK(k[1] == 100);
  n.set_keyspace(0, 51, 100);
  BOOST_CHECK(k[0] == 51);
  BOOST_CHECK(k[1] == 100);
}

BOOST_AUTO_TEST_CASE(put_get_val)
{
  auto resp = n.put_value("test");
  BOOST_CHECK(resp.data == 91);
  auto sresp = n.get_value(91);
  BOOST_CHECK(sresp.data == "test");

  n.set_keyspace(0, 51, 100);
  resp = n.put_value("tanp");
  BOOST_CHECK(resp.s == -1);
}

BOOST_AUTO_TEST_CASE(reshuffle)
{
  auto resp = n.put_value("tanp");
  n.set_keyspace(0, 51, 100);
  auto rstatus = n.reshuffle();
  BOOST_CHECK(rstatus.data == "tanp");
}
*/
BOOST_AUTO_TEST_CASE(command_struct)
{
  string test_success = "1:NAN:C1:REQ:TEST:TWO";
  commands::Commands commands(test_success);
  BOOST_CHECK(commands.id == "1");
  BOOST_CHECK(commands.ori == "NAN");
  BOOST_CHECK(commands.option == "C1");
  BOOST_CHECK(commands.reqres == "REQ");
  BOOST_CHECK(commands.data[0] == "TEST");
  BOOST_CHECK(commands.data[1] == "TWO");
  BOOST_CHECK(commands.to_string() == "1:NAN:C1:REQ:TEST:TWO");
}

BOOST_AUTO_TEST_CASE(check_membership)
{
  storage::RoutingTable rt(1, 5);
  BOOST_CHECK(rt.check_membership(1,1,0));
  BOOST_CHECK(rt.check_membership(2, 1, 0));
  BOOST_CHECK(rt.check_membership(2, 4, 3));
  BOOST_CHECK(rt.check_membership(2, 4, 5) == false);
}

BOOST_AUTO_TEST_CASE(check_rc_query_chord)
{
  //TEST FOR ONE NODE
  auto rn = n.remote_node_controller("1:NAN:QUERY_CHORD_PUT:REQ:test", partner.get_server_info());
  string teststring = "";
  teststring += n.print_chord_id();
  teststring += ":3001:PUT_CHORD:RES:59";
  BOOST_CHECK(rn.data == teststring);
  rn = n.remote_node_controller("1:NAN:QUERY_CHORD_GET:REQ:59", partner.get_server_info());
  teststring = "";
  teststring += n.print_chord_id();
  teststring += ":3001:QUERY_CHORD_GET:RES:test";
  BOOST_CHECK(rn.data == teststring);
}

BOOST_AUTO_TEST_CASE(handshake)
{
  //TEST FOR ONE NODE
  auto rn = n.remote_node_controller("1:NAN:HANDSHAKE_CHORD:REQ:1", n.get_server_info());
  string teststring;
  teststring += n.print_chord_id();
  teststring += ":NAN:SET_SUCCESSOR:RES:";
  teststring += n.print_chord_id();
  teststring += ":3000";
  BOOST_CHECK(rn.data == teststring);
}

BOOST_AUTO_TEST_CASE(SET_SUCCESSOR)
{
  string send_test;
  send_test += n.print_chord_id() + ":NAN:SET_SUCCESSOR:RES:" + n.print_chord_id() + ":3000";
  auto rn = partner.remote_node_controller(send_test, n.get_server_info());
  string teststring = "";
  teststring += partner.print_chord_id();
  teststring += ":NAN:NOTIFY_CHORD:REQ";
  BOOST_CHECK(rn.data == teststring);
}

//TODO: STABILIZE
BOOST_AUTO_TEST_CASE(STABILIZE)
{
  string send_test;
  send_test += n.print_chord_id() + ":NAN:SET_SUCCESSOR:RES:" + n.print_chord_id() + ":3000";
  auto rn = partner.remote_node_controller(send_test, n.get_server_info());
  string teststring = "";
  teststring += partner.print_chord_id();
  teststring += ":NAN:NOTIFY_CHORD:REQ";
  BOOST_CHECK(rn.data == teststring);
}

/*
BOOST_AUTO_TEST_CASE(NOTIFY_CHORD)
{
  string send_test;
  send_test += n.print_chord_id() + "NAN:NOTIFY_CHORD:REQ:";
  auto rn = partner.remote_node_controller(send_test, n.get_server_info());
  string teststring;
  teststring += partner.print_chord_id();
  teststring += ":NAN:NOTIFY_CHORD:REQ";
}
*/
/*
BOOST_AUTO_TEST_CASE(test_red){
  //TEST FOR RED
  rn = n.remote_node_controller("1:NAN:QUERY_CHORD:REQ:test", server);
  teststring = "";
  teststring += n.print_chord_id();
  teststring += ":3000:PUT_CHORD:REQ:test";
  BOOST_CHECK(rn.data == teststring);
  cout << "peer sockaddr: "<< rn.peer_sockaddr.sin_port << '\n';
  std::cout << rn.data << '\n';
}
*/
BOOST_AUTO_TEST_SUITE_END()
