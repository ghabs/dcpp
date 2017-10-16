#define BOOST_TEST_MAIN

#include <boost/test/included/unit_test_framework.hpp>
#include <string>


BOOST_AUTO_TEST_CASE (test1)
{
  std::string s;
  BOOST_CHECK(s.size() == 0);
}
