import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  width: 200
  height: 200

  SignalSpy {
    id: displaySpy
    target: webView.preferences
    signalName: "canDisplayInsecureContentChanged"
  }
  SignalSpy {
    id: runSpy
    target: webView.preferences
    signalName: "canRunInsecureContentChanged"
  }
  SignalSpy {
    id: blockedSpy
    target: webView
    signalName: "blockedContentChanged"
  }

  TestCase {
    id: test
    name: "WebPreferences_mixedContent"
    when: windowShown

    function init() {
      webView.url = "about:blank";
      verify(webView.waitForLoadSucceeded());
      compare(webView.blockedContent, WebView.ContentTypeNone);

      displaySpy.clear();
      runSpy.clear();
      blockedSpy.clear();
    }

    function _can_display() {
      return webView.getTestApi().evaluateCode("
return document.getElementsByTagName(\"img\")[0].width;", true) == 150;
    }

    function _can_run_script() {
      return webView.title == "Ran insecure!";
    }

    function _can_run_css() {
      return webView.getTestApi().evaluateCode("
var elem = document.getElementsByTagName(\"p\")[0];
var style = window.getComputedStyle(elem);
return style.getPropertyValue(\"color\");", true) == "rgb(255, 0, 0)";
    }

    function test_WebPreferences_mixedContent1_data() {
      return [
        { display: true, run: false, displayCount: 0, runCount: 0, blockedCount: 2 },
        { display: false, run: false, displayCount: 1, runCount: 0, blockedCount: 3 },
        { display: false, run: true, displayCount: 0, runCount: 1, blockedCount: 2 },
        { display: true, run: true, displayCount: 1, runCount: 0, blockedCount: 1 }
      ];
    }

    function test_WebPreferences_mixedContent1(data) {
      // Verify we can set and read back the correct value, and that setting
      // the same value doesn't result in another signal
      webView.preferences.canDisplayInsecureContent = data.display;
      compare(webView.preferences.canDisplayInsecureContent, data.display)
      webView.preferences.canDisplayInsecureContent = data.display;
      compare(displaySpy.count, data.displayCount);

      webView.preferences.canRunInsecureContent = data.run;
      compare(webView.preferences.canRunInsecureContent, data.run);
      webView.preferences.canRunInsecureContent = data.run;
      compare(runSpy.count, data.runCount);

      webView.url = "https://localhost:4443/tst_WebPreferences_mixedContent.html";
      verify(webView.waitForLoadSucceeded());

      compare(_can_display(), data.display);
      compare(_can_run_script(), data.run);
      compare(_can_run_css(), data.run);

      compare(!(webView.blockedContent & WebView.ContentTypeMixedDisplay), data.display);
      compare(!(webView.blockedContent & WebView.ContentTypeMixedScript), data.run);
      compare(blockedSpy.count, data.blockedCount);
    }
  }
}
