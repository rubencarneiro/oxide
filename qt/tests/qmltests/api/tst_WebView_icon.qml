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
    signalName: "iconChanged"
  }

  property url iconAtStart: ""
  property url iconAtCommit: ""
  property int spyCountAtStart: -1
  property int spyCountAtCommit: -1

  TestCase {
    name: "WebView_icon"
    when: windowShown

    Connections {
      target: webView

      onLoadEvent: {
        if (event.type == LoadEvent.TypeStarted) {
          webView.iconAtStart = webView.icon;
          webView.spyCountAtStart = spy.count;
        }
        if (event.type == LoadEvent.TypeCommitted) {
          webView.iconAtCommit = webView.icon;
          webView.spyCountAtCommit = spy.count;
        }

      }
    }

    // Verifies that a commit is quickly followed by an icon update
    function waitForLoadCommitted() {
      verify(webView.waitForLoadCommitted());
      spy.wait(50);
    }

    function init() {
      webView.clearLoadEventCounters();
      webView.url = "about:blank";
      verify(webView.waitForLoadSucceeded());
      spy.clear();
      webView.iconAtStart = "";
      webView.iconAtCommit = "";
      webView.spyCountAtStart = -1;
      webView.spyCountAtCommit = -1;
      webView.clearLoadEventCounters();
    }

    function test_WebView_icon1_data() {
      return [
        { url: "http://testsuite/empty.html", icon: "http://testsuite/favicon.ico" },
        { url: "http://testsuite/tst_WebView_icon.html", icon: "http://testsuite/icon.ico" },
        { url: Qt.resolvedUrl("empty.html"), icon: "" }
      ];
    }

    // Verify that WebView.icon indicates the correct icon URL
    function test_WebView_icon1(data) {
      compare(webView.icon.toString(), "");

      webView.url = data.url;
      verify(webView.waitForLoadSucceeded());

      compare(webView.icon.toString(), data.icon);
    }

    // Test that WebView.icon updates at the correct time when doing a
    // browser-initiated navigation via WebView.url
    function test_WebView_icon2_browser_initiated_navigation() {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      compare(webView.icon.toString(), "http://testsuite/favicon.ico");

      webView.clearLoadEventCounters();
      spy.clear();

      webView.url = "http://testsuite/tst_WebView_icon.html";
      compare(webView.icon.toString(), "http://testsuite/favicon.ico");
      waitForLoadCommitted();

      compare(webView.loadsCommittedCount, 1);
      compare(webView.spyCountAtStart, 0);
      compare(webView.spyCountAtCommit, 0);
      compare(webView.iconAtStart.toString(), "http://testsuite/favicon.ico");
      // We shouldn't have an icon URL at commit
      compare(webView.iconAtCommit.toString(), "");

      spy.wait();

      compare(webView.icon.toString(), "http://testsuite/icon.ico");
    }

    function test_WebView_icon3_browser_initiated_history_navigation() {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      webView.url = "http://testsuite/tst_WebView_icon.html";
      verify(webView.waitForLoadSucceeded());

      compare(webView.icon.toString(), "http://testsuite/icon.ico");

      spy.clear();
      webView.clearLoadEventCounters();

      webView.goBack();
      compare(webView.icon.toString(), "http://testsuite/icon.ico");
      waitForLoadCommitted();

      compare(webView.loadsCommittedCount, 1);
      compare(webView.spyCountAtStart, 0);
      compare(webView.spyCountAtCommit, 0);
      compare(webView.iconAtStart.toString(), "http://testsuite/icon.ico");
      // We should have an appropriate URL at commit
      compare(webView.iconAtCommit.toString(), "http://testsuite/favicon.ico");

      verify(webView.waitForLoadSucceeded());
      compare(webView.icon.toString(), "http://testsuite/favicon.ico");
      compare(spy.count, 1);
    }

    function test_WebView_icon4_content_initiated_navigation() {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      compare(webView.icon.toString(), "http://testsuite/favicon.ico");

      spy.clear();
      webView.clearLoadEventCounters();

      webView.getTestApi().evaluateCode("window.location = \"http://testsuite/tst_WebView_icon.html\"", false);
      waitForLoadCommitted();

      compare(webView.loadsCommittedCount, 1);
      compare(webView.spyCountAtStart, 0);
      compare(webView.spyCountAtCommit, 0);
      compare(webView.iconAtStart.toString(), "http://testsuite/favicon.ico");
      // We shouldn't have an icon URL at commit
      compare(webView.iconAtCommit.toString(), "");

      spy.wait();

      compare(webView.icon.toString(), "http://testsuite/icon.ico");
    }
  }
}
