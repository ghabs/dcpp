#define BOOST_TEST_MAIN
#include "../storage/storage.h"
#include <boost/test/included/unit_test.hpp>
#include <string>

using namespace std;
using namespace storage;
struct TestFixture
{
  storage::RoutingTable rt;

  TestFixture()
  : rt(5)
  {}

  ~TestFixture() = default;
};

BOOST_FIXTURE_TEST_SUITE(StorageTesting, TestFixture)

BOOST_AUTO_TEST_CASE(check_membership)
{
  //p.id = 7;
  struct sockaddr_in sserver;
  sserver.sin_port = htons(3000);
  sserver.sin_family = AF_INET;
  struct sockaddr_in pserver;
  sserver.sin_port = htons(3001);
  sserver.sin_family = AF_INET;
//  sserver.sin_addr.s_addr = inet_addr("127.0.0.1");
  int test = 1;
  rt.update_successor(7, sserver);
  BOOST_CHECK(rt.get_successor().id == 7);
  rt.update_predecessor(1, pserver);
  BOOST_CHECK(rt.get_predecessor().id == 1);
  auto k = rt.find_successor(6, &test);
  BOOST_CHECK(rt.get_successor().id == k.id);
  k = rt.find_successor(5, &test);
  BOOST_CHECK(rt.get_successor().id == k.id);
  k = rt.find_successor(4, &test);
  BOOST_CHECK(test == 0);
}

BOOST_AUTO_TEST_CASE(request_table)
{
  struct sockaddr_in sserver;
  sserver.sin_port = htons(3000);
  sserver.sin_family = AF_INET;
  storage::RequestTable reqt;
  Requests r = join;
  Requests s = handshake;

  reqt.add_request(1, s, sserver, r, "tests");
  BOOST_CHECK(reqt.get_size() == 1);
  cout << reqt.get_size() << '\n';
  reqt.remove_request(1, s);
  BOOST_CHECK(reqt.get_size() == 0);
  reqt.add_request(1, s, sserver, r, "tests");
  auto unful = reqt.clear_requests(0);
  BOOST_CHECK(unful.size() == 1);
}

BOOST_AUTO_TEST_SUITE_END()
