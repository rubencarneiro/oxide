exports.onBeforeSendHeaders = function(event) {
  if (event.url == "http://testsuite/tst_LoadEvent_httpStatusCode.py?200") {
      event.setHeader("HttpStatusCode", "200");
  } else if (event.url == "http://testsuite/tst_LoadEvent_httpStatusCode.py?201") {
      event.setHeader("HttpStatusCode", "201");
  } else if (event.url == "http://testsuite/tst_LoadEvent_httpStatusCode.py?202") {
      event.setHeader("HttpStatusCode", "202");
  } else if (event.url == "http://testsuite/tst_LoadEvent_httpStatusCode.py?203") {
      event.setHeader("HttpStatusCode", "203");
  } else if (event.url == "http://testsuite/tst_LoadEvent_httpStatusCode.py?204") {
      event.setHeader("HttpStatusCode", "204");
  } else if (event.url == "http://testsuite/tst_LoadEvent_httpStatusCode.py?205") {
      event.setHeader("HttpStatusCode", "205");
  } else if (event.url == "http://testsuite/tst_LoadEvent_httpStatusCode.py?206") {
      event.setHeader("HttpStatusCode", "206");
  } else if (event.url == "http://testsuite/tst_LoadEvent_httpStatusCode.py?400") {
      event.setHeader("HttpStatusCode", "400");
  } else if (event.url == "http://testsuite/tst_LoadEvent_httpStatusCode.py?401") {
      event.setHeader("HttpStatusCode", "401");
  } else if (event.url == "http://testsuite/tst_LoadEvent_httpStatusCode.py?403") {
      event.setHeader("HttpStatusCode", "403");
  } else if (event.url == "http://testsuite/tst_LoadEvent_httpStatusCode.py?404") {
      event.setHeader("HttpStatusCode", "404");
  } else if (event.url == "http://testsuite/tst_LoadEvent_httpStatusCode.py?405") {
      event.setHeader("HttpStatusCode", "405");
  } else if (event.url == "http://testsuite/tst_LoadEvent_httpStatusCode.py?406") {
      event.setHeader("HttpStatusCode", "406");
  } else if (event.url == "http://testsuite/tst_LoadEvent_httpStatusCode.py?407") {
      event.setHeader("HttpStatusCode", "407");
  } else if (event.url == "http://testsuite/tst_LoadEvent_httpStatusCode.py?408") {
      event.setHeader("HttpStatusCode", "408");
  } else if (event.url == "http://testsuite/tst_LoadEvent_httpStatusCode.py?409") {
      event.setHeader("HttpStatusCode", "409");
  } else if (event.url == "http://testsuite/tst_LoadEvent_httpStatusCode.py?410") {
      event.setHeader("HttpStatusCode", "410");
  } else if (event.url == "http://testsuite/tst_LoadEvent_httpStatusCode.py?411") {
      event.setHeader("HttpStatusCode", "411");
  } else if (event.url == "http://testsuite/tst_LoadEvent_httpStatusCode.py?412") {
      event.setHeader("HttpStatusCode", "412");
  } else if (event.url == "http://testsuite/tst_LoadEvent_httpStatusCode.py?413") {
      event.setHeader("HttpStatusCode", "413");
  } else if (event.url == "http://testsuite/tst_LoadEvent_httpStatusCode.py?414") {
      event.setHeader("HttpStatusCode", "414");
  } else if (event.url == "http://testsuite/tst_LoadEvent_httpStatusCode.py?415") {
      event.setHeader("HttpStatusCode", "415");
  } else if (event.url == "http://testsuite/tst_LoadEvent_httpStatusCode.py?416") {
      event.setHeader("HttpStatusCode", "416");
  } else if (event.url == "http://testsuite/tst_LoadEvent_httpStatusCode.py?417") {
      event.setHeader("HttpStatusCode", "417");
  }
};
