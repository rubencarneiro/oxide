import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  focus: true

  property var workerTestType: ""
  property var workerMessages: []

  WebContextDelegateWorker {
    id: worker

    source: Qt.resolvedUrl("tst_NetworkCallbackEvents.js")

    onMessage: {
      webView.workerMessages.push(message);
    }
  }

  Component.onCompleted: {
    context.networkRequestDelegate = worker;
    context.userAgent = "Oxide Test";
  }

  onWorkerTestTypeChanged: {
    context.networkRequestDelegate.sendMessage({ test: workerTestType });
  }

  property var loadEvents: []

  onLoadEvent: {
    loadEvents.push({ url: event.url, type: event.type, originalUrl: event.originalUrl });
  }

  TestCase {
    id: test
    name: "NetworkCallbackEvents"
    when: windowShown

    function init() {
      webView.workerTestType = "";
      webView.workerMessages = [];
      webView.loadEvents = [];
      webView.clearLoadEventCounters();
    }

    function cleanupTestCase() {
      webView.context.userAgent = "";
    }

    function _verify_worker_messages(data) {
      compare(webView.workerMessages.length, data.length);
      for (var i = 0; i < data.length; ++i) {
        for (var k in data[i]) {
          compare(webView.workerMessages[i][k], data[i][k],
                  "Unexpected value for " + k + " at index " + i);
        }
      }
      webView.workerMessages = [];
    }

    function _verify_load_events(data) {
      compare(webView.loadEvents.length, data.length);
      for (var i = 0; i < data.length; ++i) {
        compare(webView.loadEvents[i].url, data[i].url, "Unexpected value at index " + i);
        compare(webView.loadEvents[i].type, data[i].type);
        compare(webView.loadEvents[i].originalUrl, data[i].originalUrl);
      }
      webView.loadEvents = [];
    }

    // Test onBeforeURLRequest by loading a URL that we redirect to the real
    // page, which loads a frame that is also redirected. We verify that
    // we get the correct sequence of events from both WebContextDelegateWorker
    // and WebView.loadEvent
    function test_NetworkCallbackEvents1_BeforeURLRequest() {
      webView.workerTestType = "beforeURLRequest";

      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");

      compare(webView.rootFrame.childFrames.length, 1,
              "Invalid number of child frames");

      compare(webView.getTestApi().documentURI, "http://testsuite/tst_NetworkCallbackEvents1.html",
              "Invalid documentURI for main frame");
      compare(webView.getTestApiForFrame(webView.rootFrame.childFrames[0]).documentURI,
              "http://testsuite/empty.html", "Invalid documentURI for child frame");

      _verify_worker_messages([
        { url: "http://testsuite/empty.html", method: "GET", requestCancelled: false, redirectUrl: undefined, isMainFrame: true },
        { url: "http://testsuite/tst_NetworkCallbackEvents1.html", method: "GET", requestCancelled: false, redirectUrl: undefined, isMainFrame: true },
        { url: "http://testsuite/tst_NetworkCallbackEvents1_foo.html", method: "GET", requestCancelled: false, redirectUrl: undefined, isMainFrame: false },
        { url: "http://testsuite/empty.html", method: "GET", requestCancelled: false, redirectUrl: undefined, isMainFrame: false }
      ]);

      _verify_load_events([
        { url: "http://testsuite/empty.html", type: LoadEvent.TypeStarted, originalUrl: "" },
        { url: "http://testsuite/tst_NetworkCallbackEvents1.html", type: LoadEvent.TypeRedirected, originalUrl: "http://testsuite/empty.html" },
        { url: "http://testsuite/tst_NetworkCallbackEvents1.html", type: LoadEvent.TypeCommitted, originalUrl: "" },
        { url: "http://testsuite/tst_NetworkCallbackEvents1.html", type: LoadEvent.TypeSucceeded, originalUrl: "" }
      ]);     
    }

    function test_NetworkCallbackEvents2_BeforeSendHeaders_data() {
      return [
        { url: "http://testsuite/get-headers.py", "User-Agent": "Oxide Test", "Foo": undefined },
        { url: "http://testsuite/get-headers.py?override-ua", "User-Agent": "Bleurgh", "Foo": undefined },
        { url: "http://testsuite/get-headers.py?add-foo", "User-Agent": "Oxide Test", "Foo": "Bar" }
      ];
    }

    // Test that we can modify existing headers and add new ones for main frame requests
    function test_NetworkCallbackEvents2_BeforeSendHeaders(data) {
      webView.workerTestType = "beforeSendHeaders";

      webView.url = data.url;
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");

      _verify_worker_messages([
        { url: data.url, method: "GET", requestCancelled: false, isMainFrame: true, hasUA: true, UA: "Oxide Test", hasFoo: false, Foo: "" }
      ]);

      var headers = JSON.parse(webView.getTestApi().evaluateCode(
          "return document.body.children[0].innerHTML", true));

      compare(headers["user-agent"], data["User-Agent"]);
      compare(headers["foo"], data["Foo"]);
    }

    function test_NetworkCallbackEvents3_BeforeSendHeaders_subframe_data() {
      return [
        { url: "http://testsuite/tst_NetworkCallbackEvents3.html", "User-Agent": "Oxide Test", "Foo": undefined },
        { url: "http://testsuite/tst_NetworkCallbackEvents3.html?override-ua", "User-Agent": "Bleurgh", "Foo": undefined },
        { url: "http://testsuite/tst_NetworkCallbackEvents3.html?add-foo", "User-Agent": "Oxide Test", "Foo": "Bar" }
      ];
    }

    // Test that we can modify existing headers and add new ones for subframe requests
    function test_NetworkCallbackEvents3_BeforeSendHeaders_subframe(data) {
      webView.workerTestType = "beforeSendHeaders";

      webView.url = data.url;
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");
      TestUtils.waitFor(function() { return webView.workerMessages.length == 2; });

      _verify_worker_messages([
        { url: data.url, method: "GET", requestCancelled: false, isMainFrame: true, hasUA: true, UA: "Oxide Test", hasFoo: false, Foo: "" },
        { url: data.url.replace(/([^\?]+)/, "http://testsuite/get-headers.py"), method: "GET", requestCancelled: false, isMainFrame: false, hasUA: true, UA: "Oxide Test", hasFoo: false, Foo: "" }
      ]);

      var testApi = webView.getTestApiForFrame(webView.rootFrame.childFrames[0]);
      verify(TestUtils.waitFor(function() { return testApi.evaluateCode("return document.body.children.length", true) == 1; }));
      var headers = JSON.parse(testApi.evaluateCode("return document.body.children[0].innerHTML", true));

      compare(headers["user-agent"], data["User-Agent"]);
      compare(headers["foo"], data["Foo"]);
    }

    // Test that onBeforeRedirect works for both main frame and sub frame requests
    function test_NetworkCallbackEvents4_BeforeRedirect() {
      webView.workerTestType = "beforeRedirect";

      webView.url = "http://testsuite/tst_NetworkCallbackEvents_redirect.py?http://testsuite/tst_NetworkCallbackEvents4.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");

      _verify_worker_messages([
        { url: "http://testsuite/tst_NetworkCallbackEvents4.html",
          method: "GET", requestCancelled: false,
          originalUrl: "http://testsuite/tst_NetworkCallbackEvents_redirect.py?http://testsuite/tst_NetworkCallbackEvents4.html",
          isMainFrame: true },
        { url: "http://testsuite/empty.html",
          method: "GET", requestCancelled: false,
          originalUrl: "http://testsuite/tst_NetworkCallbackEvents_redirect.py?http://testsuite/empty.html",
          isMainFrame: false }
      ]);
    }

    function test_NetworkCallbackEvents5_cancelRequest_data() {
      return [
        { type: "cancelBeforeURLRequest" },
        { type: "cancelBeforeSendHeaders" },
      ];
    }

    // Verify that cancelRequest works for all main-frame network notifications
    function test_NetworkCallbackEvents5_cancelRequest(data) {
      webView.workerTestType = data.type;
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadStopped());

      _verify_worker_messages([{ requestCancelled: true }]);

      _verify_load_events([
        { url: "http://testsuite/empty.html", type: LoadEvent.TypeStarted, originalUrl: "" },
        { url: "http://testsuite/empty.html", type: LoadEvent.TypeStopped, originalUrl: "" }
      ]);
    }

    function test_NetworkCallbackEvents6_cancelRequest_subframe_data() {
      return test_NetworkCallbackEvents5_cancelRequest_data();
    }

    // Verify that cancelRequest works for all subframe network notifications
    function test_NetworkCallbackEvents6_cancelRequest_subframe(data) {
      webView.workerTestType = data.type + "_SubFrame";
      webView.url = "http://testsuite/tst_NetworkCallbackEvents6.html";
      verify(webView.waitForLoadSucceeded());

      try {
        webView.getTestApiForFrame(webView.rootFrame.childFrames[0]).documentURI;
        fail("Expected an exception");
      } catch(e) {
        verify(e instanceof TestUtils.MessageError);
        compare(e.error, ScriptMessageRequest.ErrorDestinationNotFound);
      }
    }

    function test_NetworkCallbackEvents7_referrer() {
      webView.workerTestType = "referrer";
      webView.url = "http://testsuite/tst_NetworkCallbackEvents7.html";
      verify(webView.waitForLoadSucceeded());

      webView.workerMessages = [];
      webView.clearLoadEventCounters();

      var r = webView.getTestApi().getBoundingClientRectForSelector("#link");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);
      verify(webView.waitForLoadSucceeded());

      _verify_worker_messages([
        { referrer: "http://testsuite/tst_NetworkCallbackEvents7.html" },
        { referrer: "http://testsuite/tst_NetworkCallbackEvents7.html" },
        { referrer: "http://testsuite/tst_NetworkCallbackEvents7.html" },
        { referrer: "http://testsuite/tst_NetworkCallbackEvents7.html" },
        { referrer: "http://testsuite/tst_NetworkCallbackEvents7.html" },
      ]);
    }

    function test_NetworkCallbackEvents8_no_referrer() {
      webView.workerTestType = "referrer";
      webView.url = "http://testsuite/tst_NetworkCallbackEvents8.html";
      verify(webView.waitForLoadSucceeded());

      webView.workerMessages = [];
      webView.clearLoadEventCounters();

      var r = webView.getTestApi().getBoundingClientRectForSelector("#link");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);
      verify(webView.waitForLoadSucceeded());

      _verify_worker_messages([
        { referrer: "" },
        { referrer: "" },
        { referrer: "" },
        { referrer: "" },
        { referrer: "" },
      ]);
    }

    function test_NetworkCallbackEvents9_cancelRedirect() {
      webView.workerTestType = "cancelBeforeRedirect";
      webView.url = "http://testsuite/tst_NetworkCallbackEvents_redirect.py?http://testsuite/empty.html";
      verify(webView.waitForLoadStopped());

      _verify_worker_messages([{ requestCancelled: true }]);

      _verify_load_events([
        { url: "http://testsuite/tst_NetworkCallbackEvents_redirect.py?http://testsuite/empty.html", type: LoadEvent.TypeStarted, originalUrl: "" },
        { url: "http://testsuite/empty.html", type: LoadEvent.TypeRedirected, originalUrl: "http://testsuite/tst_NetworkCallbackEvents_redirect.py?http://testsuite/empty.html" },
        { url: "http://testsuite/tst_NetworkCallbackEvents_redirect.py?http://testsuite/empty.html", type: LoadEvent.TypeStopped, originalUrl: "" }
      ]);
    }
  }
}
