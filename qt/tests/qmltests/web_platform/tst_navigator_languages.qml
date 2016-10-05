import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  focus: true

  property int languageChangeEvents: 0

  messageHandlers: [
    ScriptMessageHandler {
      msgId: "LANGUAGE-CHANGE"
      contexts: [ "oxide://testutils/" ]
      callback: function(msg) {
        languageChangeEvents++;
      }
    }
  ]

  TestCase {
    name: "navigator.languages"
    when: windowShown

    function get_navigator_languages() {
      return webView.getTestApi().evaluateCode("navigator.languages;");
    }

    function initTestCase() {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());
      verify(get_navigator_languages().length > 0);
    }

    function init() {
      webView.languageChangeEvents = 0;
    }

    function test_accept_langs() {
      webView.getTestApi().evaluateCode(
          "window.addEventListener(\"languagechange\", function(event) {
             oxide.sendMessage(\"LANGUAGE-CHANGE\");
           });", true);
      SingletonTestWebContext.acceptLangs = "fr,es,ca";
      compare(get_navigator_languages(), ["fr", "es", "ca"]);
      tryCompare(webView, "languageChangeEvents", 1);
    }
  }
}
