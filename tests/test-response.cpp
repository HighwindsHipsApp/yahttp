#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include "yahttp/yahttp.hpp"

#include "md5.h"

using namespace boost;


BOOST_AUTO_TEST_SUITE(test_response)

BOOST_AUTO_TEST_CASE(test_response_parse_ok)
{
  std::ifstream ifs("response-google-200.txt");
  YaHTTP::Response resp;
  ifs >> resp;

  BOOST_CHECK_EQUAL(resp.status, 200);
  BOOST_CHECK_EQUAL(resp.statusText, "OK");
  // check cookie parsing here too
  BOOST_CHECK_EQUAL(resp.COOKIES()["PREF"].value, "ID=6362d8df6eb4d584:FF=0:TM=1396263524:LM=1396263524:S=V9YLPeP1P62fLXXB");
  BOOST_CHECK_EQUAL(resp.COOKIES()["NID"].value, "67=W89yJtULPiGMBuchaMlw_a5JLGL6irSRusm-mBQINYCGGVSBRGA9am0vqtPl61ZLLRFeqhmyfhhrV-E0VZ9rNlGKh4gwv7B1MrMEbcP8wGHxF68Fnpaiv4cSQ_Du81f4");
}

BOOST_AUTO_TEST_CASE(test_response_parse_arl_ok)
{
  std::ifstream ifs("response-google-200.txt");
  YaHTTP::Response resp;

  YaHTTP::AsyncResponseLoader arl;
  arl.initialize(&resp); 

  while(ifs.good()) {
    char buf[1024];
    ifs.read(buf, 1024);
    if (ifs.gcount()) { // did we actually read anything
      ifs.clear();
      if (arl.feed(std::string(buf, ifs.gcount())) == true) break; // completed
    }
  }
  BOOST_CHECK(arl.ready());

  arl.finalize();

  BOOST_CHECK_EQUAL(resp.status, 200);
}

BOOST_AUTO_TEST_CASE(test_response_parse_incomplete)
{
  std::ifstream ifs("response-incomplete.txt");
  YaHTTP::Response resp;
  BOOST_CHECK_THROW(ifs >> resp, YaHTTP::ParseError);
}

BOOST_AUTO_TEST_CASE(test_response_parse_binary) {
  unsigned char result[16];
  unsigned char expected[16] = { 0xb6, 0x37, 0x1c, 0x8a, 0xc6, 0xa6, 
                                 0xa0, 0xfb, 0x10, 0x4e, 0x3d, 0x64,
                                 0x79, 0xb4, 0xd4, 0x7a };
  std::ifstream ifs("response-binary.txt");
  YaHTTP::Response resp;

  ifs >> resp;

  BOOST_CHECK_EQUAL(resp.status, 200);
  BOOST_CHECK_EQUAL(resp.body.size(), 258504);
  
  MD5_CTX ctx;
  MD5_Init(&ctx);
  MD5_Update(&ctx, resp.body.c_str(), resp.body.size());
  MD5_Final(result, &ctx);

  BOOST_CHECK_EQUAL(::memcmp(result, expected, 16), 0);
}

BOOST_AUTO_TEST_CASE(test_response_parse_chunked) {
  char buffer[1024];
  size_t n;
  std::ifstream ifs;

  YaHTTP::AsyncResponseLoader arl;
  YaHTTP::Response resp;

  arl.initialize(&resp);

  ifs.open("response-chunked-headers.txt", std::ifstream::in);
  ifs.read(buffer, sizeof buffer);
  n = ifs.gcount();
  buffer[n] = 0;
 
  arl.feed(std::string(buffer, n));
  ifs.close();
  ifs.clear();
  
  ifs.open("response-chunked-body.txt", std::ifstream::in);
  ifs.read(buffer, sizeof buffer);
  n = ifs.gcount();
  buffer[n] = 0;

  arl.feed(std::string(buffer, n));

  arl.finalize();

  BOOST_CHECK_EQUAL(resp.status, 200);
  BOOST_CHECK_EQUAL(resp.body.size(), 249);

  BOOST_CHECK_EQUAL(resp.body, "{\"result\":[{\"qname\":\"example.com\",\"qtype\":\"SOA\",\"content\":\"sns.dns.icann.org noc.dns.icann.org 2014051935 7200 3600 1209600 3600\",\"ttl\":3600,\"auth\":1},{\"qname\":\"example.com\",\"qtype\":\"NS\",\"content\":\"sns.dns.icann.org\",\"ttl\":3600,\"auth\":1}],\"log\":[]}\n"); 
}

BOOST_AUTO_TEST_CASE(test_response_print_chunked) {
  YaHTTP::Response resp;
  std::ostringstream oss;
  resp.status = 200;
  resp.headers["content-type"] = "text/html; charset=utf-8";
  resp.body = "<!DOCTYPE html>\n<html lang=\"en\"><head><title>Hello, world</title><link rel=\"stylesheet\" href=\"style.css\" type=\"text/css\" /></head><body><h1>200 OK</h1><p>Hello, world</p></body></html>";
 
  oss << resp;

  BOOST_CHECK_EQUAL(oss.str(), "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\nContent-Type: text/html; charset=utf-8\r\n\r\nb8\r\n<!DOCTYPE html>\n<html lang=\"en\"><head><title>Hello, world</title><link rel=\"stylesheet\" href=\"style.css\" type=\"text/css\" /></head><body><h1>200 OK</h1><p>Hello, world</p></body></html>\r\n0\r\n\r\n");
}

BOOST_AUTO_TEST_CASE(test_response_empty_chunked) {
  YaHTTP::Response resp;
  std::ostringstream oss;
  resp.status = 201;
  resp.headers["content-type"] = "text/html; charset=utf-8";
  resp.headers["content-length"] = "0";
  resp.body = "";

  oss << resp;
  BOOST_CHECK_EQUAL(oss.str(), "HTTP/1.1 201 Created\r\nContent-Length: 0\r\nContent-Type: text/html; charset=utf-8\r\n\r\n");
}

BOOST_AUTO_TEST_CASE(test_response_content_length) {
  YaHTTP::Response resp;
  std::ostringstream oss;
  resp.status = 200;
  resp.headers["content-type"] = "text/html; charset=utf-8";
  resp.headers["content-length"] = "12";
  resp.body = "hello, world";

  oss << resp;
  BOOST_CHECK_EQUAL(oss.str(), "HTTP/1.1 200 OK\r\nContent-Length: 12\r\nContent-Type: text/html; charset=utf-8\r\n\r\nhello, world");
}

}
