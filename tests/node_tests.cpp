
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
  : n(3000, "127.0.0.1", 2001),
  partner(3001, "127.0.0.1", 3000)
  {}

  ~TestFixture() = default;
};


BOOST_FIXTURE_TEST_SUITE(NodeTesting, TestFixture)

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

BOOST_AUTO_TEST_SUITE_END()
