import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 0.1
import com.canonical.Oxide.Testing 0.1

TestWebView {
  id: webView

  property var workerMessageType: ""
  property var workerMessages: []

  context.networkRequestDelegate: NetworkDelegateWorker {
    source: Qt.resolvedUrl("tst_NetworkCallbackEvents.js")

    onMessage: {
      if (message.event == webView.workerMessageType) {
        webView.workerMessages.push(message);
      }
    }
  }

  context.userAgent: "Oxide Test"

  TestCase {
    id: test
    name: "NetworkCallbackEvents"
    when: windowShown

    function init() {
      webView.workerMessageType = "";
      webView.workerMessages = [];
    }

    function test_NetworkCallbackEvents1_BeforeURLRequest() {
      webView.workerMessageType = "beforeURLRequest";

      webView.url = "http://localhost:8080/tst_NetworkCallbackEvents1.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");

      compare(webView.rootFrame.childFrames.length, 2,
              "Invalid number of child frames");

      compare(webView.getTestApi().documentURI, webView.url,
              "Invalid documentURI for main frame");
      compare(webView.getTestApiForFrame(webView.rootFrame.childFrames[0]).documentURI,
              "http://localhost:8080/empty.html", "Invalid documentURI for child frame");

      try {
        webView.getTestApiForFrame(webView.rootFrame.childFrames[1]).documentURI;
        fail("Expected an exception");
      } catch(e) {
        verify(e instanceof TestUtils.MessageError);
        compare(e.error, ScriptMessageRequest.ErrorDestinationNotFound);
      }

      compare(webView.workerMessages.length, 4, "Unexpected number of worker messages");

      var data = [
        { url: "http://localhost:8080/tst_NetworkCallbackEvents1.html", method: "GET", requestCancelled: false, redirectUrl: "" },
        { url: "http://localhost:8080/tst_NetworkCallbackEvents2.html", method: "GET", requestCancelled: false, redirectUrl: "" },
        { url: "http://localhost:8080/tst_NetworkCallbackEvents3.html", method: "GET", requestCancelled: false, redirectUrl: "" },
        { url: "http://localhost:8080/empty.html", method: "GET", requestCancelled: false, redirectUrl: "" },
      ];

      for (var i = 0; i < data.length; ++i) {
        compare(webView.workerMessages[i].url, data[i].url, "Unexpected value for message " + i);
        compare(webView.workerMessages[i].method, data[i].method, "Unexpected value for message " + i);
        compare(webView.workerMessages[i].requestCancelled, data[i].requestCancelled, "Unexpected value for message " + i);
        compare(webView.workerMessages[i].redirectUrl, data[i].redirectUrl, "Unexpected value for message " + i);
      }
    }

    function test_NetworkCallbackEvents2_BeforeSendHeaders_data() {
      return [
        { url: "http://localhost:8080/tst_NetworkCallbackEvents.py", "User-Agent": "Oxide Test", "Foo": undefined },
        { url: "http://localhost:8080/tst_NetworkCallbackEvents.py?override-ua", "User-Agent": "Bleurgh", "Foo": undefined },
        // XXX: Clearing the User-Agent doesn't work - it seems to get added back later
        // { url: "http://localhost:8080/tst_NetworkCallbackEvents.py?clear-ua", "User-Agent": undefined, "Foo": undefined },
        { url: "http://localhost:8080/tst_NetworkCallbackEvents.py?add-foo", "User-Agent": "Oxide Test", "Foo": "Bar" }
      ];
    }

    function test_NetworkCallbackEvents2_BeforeSendHeaders(data) {
      webView.workerMessageType = "beforeSendHeaders";

      webView.url = data.url;
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");

      compare(webView.workerMessages.length, 1, "Unexpected number of worker messages");
      compare(webView.workerMessages[0].url, data.url);
      compare(webView.workerMessages[0].method, "GET");
      compare(webView.workerMessages[0].requestCancelled, false);
      compare(webView.workerMessages[0].hasUA, true);
      compare(webView.workerMessages[0].UA, "Oxide Test");
      compare(webView.workerMessages[0].hasFoo, false);
      compare(webView.workerMessages[0].Foo, "");

      function getHeader(header) {
        return webView.getTestApi().evaluateCode(
            "var el = document.querySelectorAll(\".header\");\
             for (var i = 0; i < el.length; ++i) {\
               var h = el[i].innerHTML.split(\"=\")[0];\
               var v = el[i].innerHTML.split(\"=\")[1];\
               if (h == \"" + header + "\") return v;\
             }\
             return undefined;",
             true);
      }

      compare(getHeader("User-Agent"), data["User-Agent"]);
      compare(getHeader("Foo"), data["Foo"]);
    }
  }
}
