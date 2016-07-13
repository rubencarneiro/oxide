import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  focus: true

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


    function test_WebPreferences_mixedContent1_api_data() {
      return [
        { display: true, run: false, displayChanged: false, runChanged: false },
        { display: false, run: false, displayChanged: true, runChanged: false },
        { display: false, run: true, displayChanged: false, runChanged: true },
        { display: true, run: true, displayChanged: true, runChanged: false }
      ];
    }

    function test_WebPreferences_mixedContent1_api(data) {
      // Verify we can set and read back the correct value, and that setting
      // the same value doesn't result in another signal
      webView.preferences.canDisplayInsecureContent = data.display;
      compare(webView.preferences.canDisplayInsecureContent, data.display)
      webView.preferences.canDisplayInsecureContent = data.display;
      compare(displaySpy.count, data.displayChanged ? 1 : 0);

      webView.preferences.canRunInsecureContent = data.run;
      compare(webView.preferences.canRunInsecureContent, data.run);
      webView.preferences.canRunInsecureContent = data.run;
      compare(runSpy.count, data.runChanged ? 1 : 0);
    }

    function test_WebPreferences_mixedContent2_block_main_frame_data() {
      return [
        { display: true, run: false, blockedCount: 2 },
        { display: false, run: false, blockedCount: 3 },
        { display: false, run: true, blockedCount: 2 },
        { display: true, run: true, blockedCount: 1 }
      ];
    }

    function test_WebPreferences_mixedContent2_block_main_frame(data) {
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

      webView.preferences.canDisplayInsecureContent = data.display;
      webView.preferences.canRunInsecureContent = data.run;

      webView.url = "https://testsuite/tst_WebPreferences_mixedContent.html";
      verify(webView.waitForLoadSucceeded());

      compare(_can_display(), data.display);
      compare(_can_run_script(), data.run);
      compare(_can_run_css(), data.run);

      compare(!(webView.blockedContent & WebView.ContentTypeMixedDisplay), data.display);
      compare(!(webView.blockedContent & WebView.ContentTypeMixedScript), data.run);
      compare(blockedSpy.count, data.blockedCount);
    }

    function test_WebPreferences_mixedContent3_block_sub_frame_data() {
      return [
        { display: true, run: false, blockedCount: 1 },
        { display: false, run: false, blockedCount: 2 },
        { display: false, run: true, blockedCount: 2 },
        { display: true, run: true, blockedCount: 1 }
      ];
    }

    function test_WebPreferences_mixedContent3_block_sub_frame(data) {
      function _can_display() {
        return webView.getTestApiForFrame(webView.rootFrame.childFrames[0]).evaluateCode("
return document.getElementsByTagName(\"img\")[0].width;", true) == 150;
      }
      function _can_run_script() {
        return webView.getTestApiForFrame(webView.rootFrame.childFrames[0]).evaluateCode(
"return document.title;", true) == "Ran insecure!";
      }
      function _can_run_css() {
        return webView.getTestApiForFrame(webView.rootFrame.childFrames[0]).evaluateCode("
var elem = document.getElementsByTagName(\"p\")[0];
var style = window.getComputedStyle(elem);
return style.getPropertyValue(\"color\");", true) == "rgb(255, 0, 0)";
      }

      webView.preferences.canDisplayInsecureContent = data.display;
      webView.preferences.canRunInsecureContent = data.run;

      webView.url = "https://testsuite/tst_WebPreferences_mixedContent_subframe.html";
      verify(webView.waitForLoadSucceeded());

      compare(_can_display(), data.display);
      compare(_can_run_script(), data.run);
      compare(_can_run_css(), data.run);

      compare(!(webView.blockedContent & WebView.ContentTypeMixedDisplay), data.display);
      compare(!(webView.blockedContent & WebView.ContentTypeMixedScript), true,
              "We don't get notified of blocked mixed script in subframes");
      compare(blockedSpy.count, data.blockedCount);
    }
  }
}
