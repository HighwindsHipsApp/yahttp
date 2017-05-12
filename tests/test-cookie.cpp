#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_NO_MAIN

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include "yahttp/yahttp.hpp"

BOOST_AUTO_TEST_SUITE(test_cookie)

BOOST_AUTO_TEST_CASE(test_cookie_cookie) {
  YaHTTP::Cookie c;
  c.name = "hello world";
  c.value = "world hello"; 
  BOOST_CHECK_EQUAL(c.str(), "hello%20world=world%20hello");
  c.expires.parseCookie("05-May-2014 00:17:36 GMT");
  BOOST_CHECK_EQUAL(c.str(), "hello%20world=world%20hello; expires=05-May-2014 00:17:36 GMT");
  c.path = "/test";
  c.domain = "test.org";
  BOOST_CHECK_EQUAL(c.str(), "hello%20world=world%20hello; expires=05-May-2014 00:17:36 GMT; domain=test.org; path=/test");
  c.httponly = true;
  BOOST_CHECK_EQUAL(c.str(), "hello%20world=world%20hello; expires=05-May-2014 00:17:36 GMT; domain=test.org; path=/test; httpOnly");
  c.httponly = false;
  c.secure = true;
  BOOST_CHECK_EQUAL(c.str(), "hello%20world=world%20hello; expires=05-May-2014 00:17:36 GMT; domain=test.org; path=/test; secure");
}

BOOST_AUTO_TEST_CASE(test_cookie_parse)
{
  YaHTTP::CookieJar jar;
 
  const std::vector<std::string> tests = boost::assign::list_of
     ("hello%20world=world%20hello")
     ("hello%20world=world%20hello; expires=05-May-2014 00:17:36 GMT")
     ("hello%20world=world%20hello; expires=05-May-2014 00:17:36 GMT; domain=test.org; path=/test")
     ("hello%20world=world%20hello; expires=05-May-2014 00:17:36 GMT; domain=test.org; path=/test; httpOnly")
     ("hello%20world=world%20hello; expires=05-May-2014 00:17:36 GMT; domain=test.org; path=/test; secure");

  for(const auto &cookie: tests) {
    jar.parseSetCookieHeader(cookie);
    BOOST_CHECK_EQUAL(jar.cookies["hello world"].str(), cookie);
  }

  jar.parseCookieHeader("hello%20world=world%20hello; second=value; third=try");
  BOOST_CHECK_EQUAL(jar.cookies["hello world"].value, "world hello");
  BOOST_CHECK_EQUAL(jar.cookies["second"].value, "value");
  BOOST_CHECK_EQUAL(jar.cookies["third"].value, "try");

}

}
