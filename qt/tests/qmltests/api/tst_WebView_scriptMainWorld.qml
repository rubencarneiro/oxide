import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  property var addedScript: null

  Component {
    id: userScript
    UserScript {}
  }

  Component {
    id: scriptMessageHandler
    ScriptMessageHandler {}
  }

  TestCase {
    name: "WebView_scriptMainWorld"
    when: windowShown

    function init() {
      if (addedScript)
        webView.context.removeUserScript(addedScript)
    }

    function resultUpdated(testapi) {
      var res;
      try {
        res = testapi.evaluateCode("\
  var el = document.getElementById(\"result\");\
  if (!el) throw Exception();\
  return el.innerHTML;", true);
      } catch(e) {
        fail("#result not found: " + JSON.stringify(e));
      }
      return res;
    }

    function test_WebView_canInjectInMainWorldWithOxideApi() {
      webView.addedScript = userScript.createObject(null, {
        context: "oxide-private://main-world-private",
        emulateGreasemonkey: true,
        url: "tst_WebView_scriptMainWorld_user_script.js"});
      webView.context.addUserScript(webView.addedScript);

      webView.url = "http://testsuite/tst_WebView_scriptMainWorld.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var testApi = webView.getTestApi();
      var res = webView.waitFor(function () { return resultUpdated(testApi); })

      compare(res, "Main world content script found oxide.sendMessage",
              "Unexpected result message");
    }

    function test_WebView_receiveMessageFromMainWorldUserscript() {
      webView.addedScript = userScript.createObject(null, {
        context: "oxide-private://main-world-private",
        emulateGreasemonkey: true,
        url: "tst_WebView_scriptMainWorld_user_script.js"});

      var received = null;
      var frame = webView.rootFrame;
      var handler = scriptMessageHandler.createObject(null, {
        msgId: "from-user-script",
        contexts: ["oxide-private://main-world-private"],
        callback: function(msg, frame) {
          received = msg.args;
        }
      });

      frame.addMessageHandler(handler);

      webView.context.addUserScript(webView.addedScript);
      webView.url = "http://testsuite/tst_WebView_scriptMainWorld.html";

      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      verify(received != null, "Did not receive message from the main frame's userscript");
      compare(received.data, "mydata" , "Did not receive message from the main frame's userscript");
    }
  }
}

