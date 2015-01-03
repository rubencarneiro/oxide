import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

WebView {
  id: webView
  width: 200
  height: 200

  context: TestWebContext {}

  property bool loading2: loading

  SignalSpy {
    id: spy
    target: webView
    signalName: "loading2Changed"
  }

  property bool loadSucceeded: false

  onLoadingChanged: {
    if (loadEvent.type == LoadEvent.TypeSucceeded) {
      loadSucceeded = true;
    }
  }

  SignalSpy {
    id: succeededSpy
    target: webView
    signalName: "loadSucceededChanged"
  }

  TestCase {
    id: test
    name: "tst_WebView_loading_old"
    when: windowShown

    // Starting in Oxide 1.3, the NOTIFY signal for WebView.loading changed
    // from WebView.loadingChanged to WebView.loadingStateChanged (even for
    // pre-1.3 embedders). As we hide WebView.onLoadingStateChanged for pre-1.3
    // embedders, just do a quick sanity check to make sure that bindings still
    // work ok so that we won't break existing apps
    function test_WebView_loading_old() {
      webView.url = "http://testsuite/empty.html";
      succeededSpy.wait();

      compare(spy.count, 2);
    }
  }
}
