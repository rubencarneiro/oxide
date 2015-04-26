import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.8
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  width: 200
  height: 200

  property var expectedLoadEvents: []

  function on_load_event(event) {
    test.verify(expectedLoadEvents.length > 0);
    var expected = expectedLoadEvents[0];
    expectedLoadEvents.shift();

    test.compare(event.type, expected.type, "Unexpected type")

    test.compare(event.httpStatusCode,
                 event.type != LoadEvent.TypeStarted && event.type != LoadEvent.TypeStopped ?
                  expected.httpStatusCode : -1,
                 "Unexpected value of httpStatusCode");
  }

  TestCase {
    id: test
    name: "LoadEvent_httpStatusCode"
    when: windowShown

    function init() {
      webView.clearLoadEventCounters();
      expectedLoadEvents = [];
    }

    // Test we have right 2xx codes
    function test_HttpStatusCode2xx() {
      expectedLoadEvents = [
        // http://testsuite/tst_LoadEvent_httpStatusCode.py?200
        { type: LoadEvent.TypeStarted },
        { type: LoadEvent.TypeCommitted, httpStatusCode: 200 },
        { type: LoadEvent.TypeSucceeded, httpStatusCode: 200 },
        // http://testsuite/tst_LoadEvent_httpStatusCode.py?201
        { type: LoadEvent.TypeStarted },
        { type: LoadEvent.TypeCommitted, httpStatusCode: 201 },
        { type: LoadEvent.TypeSucceeded, httpStatusCode: 201 },
        // http://testsuite/tst_LoadEvent_httpStatusCode.py?202
        { type: LoadEvent.TypeStarted },
        { type: LoadEvent.TypeCommitted, httpStatusCode: 202 },
        { type: LoadEvent.TypeSucceeded, httpStatusCode: 202 },
        // http://testsuite/tst_LoadEvent_httpStatusCode.py?203
        { type: LoadEvent.TypeStarted },
        { type: LoadEvent.TypeCommitted, httpStatusCode: 203 },
        { type: LoadEvent.TypeSucceeded, httpStatusCode: 203 },
        // http://testsuite/tst_LoadEvent_httpStatusCode.py?204
        { type: LoadEvent.TypeStarted },
        { type: LoadEvent.TypeStopped, httpStatusCode: 204 },
        // http://testsuite/tst_LoadEvent_httpStatusCode.py?205
        // XXX: this causes a loop
        //{ type: LoadEvent.TypeStarted },
        //{ type: LoadEvent.TypeCommitted, httpStatusCode: 205 },
        // http://testsuite/tst_LoadEvent_httpStatusCode.py?206
        { type: LoadEvent.TypeStarted },
        { type: LoadEvent.TypeCommitted, httpStatusCode: 206 },
        { type: LoadEvent.TypeSucceeded, httpStatusCode: 200 },
      ]

      webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?200";
      verify(webView.waitForLoadSucceeded());

      webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?201";
      verify(webView.waitForLoadSucceeded());

      webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?202";
      verify(webView.waitForLoadSucceeded());

      webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?203";
      verify(webView.waitForLoadSucceeded());

      webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?204";
      verify(webView.waitForLoadStopped());

      //webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?205";
      //verify(webView.waitForLoadCommited());

      webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?206";
      verify(webView.waitForLoadSucceeded());
    }

    // Test we have right 4xx codes
    function test_HttpStatusCode4xx() {
      expectedLoadEvents = [
        // http://testsuite/tst_LoadEvent_httpStatusCode.py?400
        { type: LoadEvent.TypeStarted },
        { type: LoadEvent.TypeCommitted, httpStatusCode: 400 },
        { type: LoadEvent.TypeSucceeded, httpStatusCode: -1 },
        // http://testsuite/tst_LoadEvent_httpStatusCode.py?401
        { type: LoadEvent.TypeStarted },
        { type: LoadEvent.TypeCommitted, httpStatusCode: 401 },
        { type: LoadEvent.TypeSucceeded, httpStatusCode: -1 },
        // http://testsuite/tst_LoadEvent_httpStatusCode.py?403
        { type: LoadEvent.TypeStarted },
        { type: LoadEvent.TypeCommitted, httpStatusCode: 403 },
        { type: LoadEvent.TypeSucceeded, httpStatusCode: -1 },
        // http://testsuite/tst_LoadEvent_httpStatusCode.py?404
        { type: LoadEvent.TypeStarted },
        { type: LoadEvent.TypeCommitted, httpStatusCode: 404 },
        { type: LoadEvent.TypeSucceeded, httpStatusCode: -1 },
        // http://testsuite/tst_LoadEvent_httpStatusCode.py?405
        { type: LoadEvent.TypeStarted },
        { type: LoadEvent.TypeCommitted, httpStatusCode: 405 },
        { type: LoadEvent.TypeSucceeded, httpStatusCode: -1 },
        // http://testsuite/tst_LoadEvent_httpStatusCode.py?406
        { type: LoadEvent.TypeStarted },
        { type: LoadEvent.TypeCommitted, httpStatusCode: 406 },
        { type: LoadEvent.TypeSucceeded, httpStatusCode: -1 },
        // http://testsuite/tst_LoadEvent_httpStatusCode.py?407
        // XXX: this returns 406
        //{ type: LoadEvent.TypeStarted },
        //{ type: LoadEvent.TypeCommitted, httpStatusCode: 407 },
        //{ type: LoadEvent.TypeSucceeded, httpStatusCode: 0 },
        // http://testsuite/tst_LoadEvent_httpStatusCode.py?408
        { type: LoadEvent.TypeStarted },
        { type: LoadEvent.TypeCommitted, httpStatusCode: 408 },
        { type: LoadEvent.TypeSucceeded, httpStatusCode: -1 },
        // http://testsuite/tst_LoadEvent_httpStatusCode.py?409
        { type: LoadEvent.TypeStarted },
        { type: LoadEvent.TypeCommitted, httpStatusCode: 409 },
        { type: LoadEvent.TypeSucceeded, httpStatusCode: -1 },
        // http://testsuite/tst_LoadEvent_httpStatusCode.py?410
        { type: LoadEvent.TypeStarted },
        { type: LoadEvent.TypeCommitted, httpStatusCode: 410 },
        { type: LoadEvent.TypeSucceeded, httpStatusCode: -1 },
        // http://testsuite/tst_LoadEvent_httpStatusCode.py?411
        { type: LoadEvent.TypeStarted },
        { type: LoadEvent.TypeCommitted, httpStatusCode: 411 },
        { type: LoadEvent.TypeSucceeded, httpStatusCode: -1 },
        // http://testsuite/tst_LoadEvent_httpStatusCode.py?412
        { type: LoadEvent.TypeStarted },
        { type: LoadEvent.TypeCommitted, httpStatusCode: 412 },
        { type: LoadEvent.TypeSucceeded, httpStatusCode: -1 },
        // http://testsuite/tst_LoadEvent_httpStatusCode.py?413
        { type: LoadEvent.TypeStarted },
        { type: LoadEvent.TypeCommitted, httpStatusCode: 413 },
        { type: LoadEvent.TypeSucceeded, httpStatusCode: -1 },
        // http://testsuite/tst_LoadEvent_httpStatusCode.py?414
        { type: LoadEvent.TypeStarted },
        { type: LoadEvent.TypeCommitted, httpStatusCode: 414 },
        { type: LoadEvent.TypeSucceeded, httpStatusCode: -1 },
        // http://testsuite/tst_LoadEvent_httpStatusCode.py?415
        { type: LoadEvent.TypeStarted },
        { type: LoadEvent.TypeCommitted, httpStatusCode: 415 },
        { type: LoadEvent.TypeSucceeded, httpStatusCode: -1 },
        // http://testsuite/tst_LoadEvent_httpStatusCode.py?416
        { type: LoadEvent.TypeStarted },
        { type: LoadEvent.TypeCommitted, httpStatusCode: 416 },
        { type: LoadEvent.TypeSucceeded, httpStatusCode: -1 },
        // http://testsuite/tst_LoadEvent_httpStatusCode.py?417
        { type: LoadEvent.TypeStarted },
        { type: LoadEvent.TypeCommitted, httpStatusCode: 417 },
        { type: LoadEvent.TypeSucceeded, httpStatusCode: -1 },
      ]

      webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?400";
      verify(webView.waitForLoadSucceeded());

      webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?401";
      verify(webView.waitForLoadSucceeded());

      webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?403";
      verify(webView.waitForLoadSucceeded());

      webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?404";
      verify(webView.waitForLoadSucceeded());

      webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?405";
      verify(webView.waitForLoadSucceeded());

      webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?406";
      verify(webView.waitForLoadSucceeded());

      //webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?407";
      //verify(webView.waitForLoadSucceeded());

      webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?408";
      verify(webView.waitForLoadSucceeded());

      webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?409";
      verify(webView.waitForLoadSucceeded());

      webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?410";
      verify(webView.waitForLoadSucceeded());

      webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?411";
      verify(webView.waitForLoadSucceeded());

      webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?412";
      verify(webView.waitForLoadSucceeded());

      webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?413";
      verify(webView.waitForLoadSucceeded());

      webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?414";
      verify(webView.waitForLoadSucceeded());

      webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?415";
      verify(webView.waitForLoadSucceeded());

      webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?416";
      verify(webView.waitForLoadSucceeded());

      webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?417";
      verify(webView.waitForLoadSucceeded());
    }
  }
}
