#ifndef NODE_TEST
#define NODE_TEST
#include <boost/test/included/unit_test.hpp>
#include "../node.h"
#include <string>

BOOST_AUTO_TEST_SUITE (node) // name of the test suite is stringtest

BOOST_AUTO_TEST_CASE (test1)
{
  std::string s;
  BOOST_CHECK(s.size() == 0);
}

BOOST_AUTO_TEST_SUITE_END( )


#endif
