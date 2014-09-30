import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  SignalSpy {
    id: spy
    target: webView
    signalName: "loadingStateChanged"
  }

  property var inTest: true
  property var expectedLoadEvents: []

  function on_load_event(event) {
    if (!inTest) {
      return;
    }

    console.log("Event type: " + event.type + ", url: " + event.url);
    test.verify(expectedLoadEvents.length > 0);
    var expected = expectedLoadEvents[0];

    test.compare(event.type, expected.type, "Unexpected load event");
    test.compare(event.url, expected.url, "Unexpected load event URL");
    test.compare(loading, expected.loading,
                 "Unexpected state of WebView.loading");

    expectedLoadEvents.shift();
  }

  TestCase {
    id: test
    name: "WebView_loading"
    when: windowShown

    function init() {
      webView.inTest = false;
      webView.url = "about:blank";
      verify(webView.waitForLoadSucceeded());

      webView.waitFor(function() { return !webView.loading; });

      webView.inTest = true;
      webView.clearLoadEventCounters();
      spy.clear();
      expectedLoadEvents = [];
    }

    // Test we get the correct sequence of events for successful browser
    // initiated loads
    function test_WebView_loading1_browser_initiated() {
      var url = "http://testsuite/empty.html";
      expectedLoadEvents = [
        { type: LoadEvent.TypeStarted, url: url, loading: true },
        { type: LoadEvent.TypeCommitted, url: url, loading: true },
        { type: LoadEvent.TypeSucceeded, url: url, loading: false }
      ];

      webView.url = url;

      verify(webView.loading, "WebView.loading should be true once we start loading");
      compare(spy.count, 1);

      spy.clear();
      spy.wait();

      verify(!webView.loading, "WebView.loading should be false after we finish loading");
      compare(spy.count, 1,
              "WebView.loading should have changed twice during the load");
      compare(expectedLoadEvents.length, 0, "Some load events are missing");
    }

    // Test we get the correct sequence of events for successful content
    // initiated loads
    function test_WebView_loading2_renderer_initiated() {
      var initial_url = "http://testsuite/empty.html";
      var new_url = "http://foo.testsuite/empty.html";
      expectedLoadEvents = [
        { type: LoadEvent.TypeStarted, url: initial_url, loading: true },
        { type: LoadEvent.TypeCommitted, url: initial_url, loading: true },
        { type: LoadEvent.TypeSucceeded, url: initial_url, loading: false },
        { type: LoadEvent.TypeStarted, url: new_url, loading: true }, 
        { type: LoadEvent.TypeCommitted, url: new_url, loading: true },
        { type: LoadEvent.TypeSucceeded, url: new_url, loading: false }
      ];

      webView.url = initial_url;
      verify(webView.waitForLoadSucceeded());

      verify(!webView.loading);
      spy.clear();
      
      webView.getTestApi().evaluateCode("window.location = \"" + new_url + "\";", false);
      webView.waitFor(function() { return spy.count == 2; });

      verify(!webView.loading, "WebView.loading should be false once we finish loading");
      compare(expectedLoadEvents.length, 0, "Some load events are missing");
    }

    // Test we get the correct sequence of events when we stop a browser
    // initiated load before it is committed
    function test_WebView_loading3_stop() {
      var url = "http://testsuite/tst_WebView_loading_delay.py";
      expectedLoadEvents = [
        { type: LoadEvent.TypeStarted, url: url, loading: true },
        { type: LoadEvent.TypeStopped, url: url, loading: true }
      ];

      function _loadEvent(event) {
        if (event.type == LoadEvent.TypeStarted) {
          webView.stop();
        }
      }
      webView.loadEvent.connect(_loadEvent);

      webView.url = url;

      verify(webView.loading, "WebView.loading should be true once we start loading");
      compare(spy.count, 1);

      spy.clear();
      spy.wait();

      verify(!webView.loading, "WebView.loading should be false after we finish loading");
      compare(spy.count, 1,
              "WebView.loading should have changed twice during the load");
      compare(expectedLoadEvents.length, 0, "Some load events are missing");

      webView.loadEvent.disconnect(_loadEvent);
    }

    // Test we get the correct sequence of events for failed browser initiated
    // loads
    function test_WebView_loading4_fail() {
      var url = "http://moo.foo.bar.com/";
      expectedLoadEvents = [
        { type: LoadEvent.TypeStarted, url: url, loading: true },
        { type: LoadEvent.TypeFailed, url: url, loading: true },
        { type: LoadEvent.TypeStarted, url: "data:text/html,chromewebdata", loading: true },
        { type: LoadEvent.TypeCommitted, url: url, loading: true },
        { type: LoadEvent.TypeSucceeded, url: "data:text/html,chromewebdata", loading: false },
      ];

      webView.url = url;

      verify(webView.loading, "WebView.loading should be true once we start loading");
      compare(spy.count, 1);

      spy.clear();
      spy.wait();

      verify(!webView.loading, "WebView.loading should be false after we finish loading");
      compare(spy.count, 1,
              "WebView.loading should have changed twice during the load");
      compare(expectedLoadEvents.length, 0, "Some load events are missing");
    }

    function test_WebView_loading5_redirection() {
      var url = "http://testsuite/tst_WebView_loading_redirect.py";
      expectedLoadEvents = [
        { type: LoadEvent.TypeStarted, url: url, loading: true },
        { type: LoadEvent.TypeRedirected, url: "http://testsuite/empty.html", loading: true },
        { type: LoadEvent.TypeCommitted, url: "http://testsuite/empty.html", loading: true },
        { type: LoadEvent.TypeSucceeded, url: "http://testsuite/empty.html", loading: false }
      ];

      webView.url = url;

      verify(webView.loading, "WebView.loading should be true once we start loading");
      compare(spy.count, 1);

      spy.clear();
      spy.wait();

      verify(!webView.loading, "WebView.loading should be false after we finish loading");
      compare(spy.count, 1,
              "WebView.loading should have changed twice during the load");
      compare(expectedLoadEvents.length, 0, "Some load events are missing");
    }
  }
}
