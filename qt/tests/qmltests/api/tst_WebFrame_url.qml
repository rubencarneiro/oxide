import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.3
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  SignalSpy {
    id: spy
    signalName: "urlChanged"
  }

  Component.onCompleted: {
    spy.target = webView.rootFrame;
  }

  TestCase {
    id: test
    name: "WebFrame_url"
    when: windowShown

    function init() {
      spy.clear();
      webView.clearLoadEventCounters();
    }

    function test_WebFrame_url1_data() {
      return [
        { url: "http://testsuite/empty.html" },
        { url: "http://invalid/", type: "fail" },
        { url: "foo://bar.com/", type: "stop" }
      ];
    }

    function test_WebFrame_url1(data) {
      var origUrl = webView.rootFrame.url;

      function _onLoadEvent(event) {
        var committed = event.type == LoadEvent.TypeCommitted ||
                        event.type == LoadEvent.TypeSucceeded;

        compare(spy.count, committed ? 1 : 0);
        compare(webView.rootFrame.url, committed ? data.url : origUrl);
      }

      webView.loadEvent.connect(_onLoadEvent);

      webView.url = data.url;
      if (data.type == "fail") {
        verify(webView.waitForLoadCommitted(),
               "Timed out waiting for failed load");
      } else if (data.type == "stop") {
        verify(webView.waitForLoadStopped(),
               "Timed out waiting for cancelled load");
      } else {
        verify(webView.waitForLoadSucceeded(),
               "Timed out waiting for successful load");
      }

      webView.loadEvent.disconnect(_onLoadEvent);

      if (!("finalUrl" in data)) {
        data.finalUrl = data.url;
      }

      compare(spy.count, data.type == "stop" ? 0 : 1,
              "Incorrect number of urlChanged signals");
      compare(webView.rootFrame.url, data.type == "stop" ? origUrl : data.finalUrl,
              "Unexpected WebFrame.url");
    }
  }
}
