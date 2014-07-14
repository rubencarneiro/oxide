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
    target: webView.context
    signalName: "userScriptsChanged"
  }

  Component {
    id: userScript
    UserScript {}
  }

  Component {
    id: webContext
    WebContext {}
  }

  Component {
    id: scriptMessageHandler
    ScriptMessageHandler {}
  }

  TestCase {
    id: test
    name: "WebView_scriptMainWorld"
    when: windowShown

    function init() {
      spy.clear();
    }

    function test_oxide_api_in_main_scope_when_flag_set() {
      webView.context.__injectOxideApiInMainWorld = true;
      webView.url = "http://localhost:8080/tst_WebView_scriptMainWorld.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var res;
      try {
        res = webView.getTestApi().evaluateCode("\
var el = window.document.getElementById(\"result\");\
if (!el) throw Exception();\
return el.innerHTML;", true);
      } catch(e) {
        fail("'Result' DOM element not found: " + JSON.stringify(e));
      }

      compare(res, "oxide.postMessage",
              "Unexpected result message");
    }

    function test_can_inject_in_main_world_with_no_oxide_api() {
      webView.context.__injectOxideApiInMainWorld = false;
      var otherContext =  webContext.createObject(null, {});
      var script = userScript.createObject(null, {
        context: "oxide-private://main-world-private",
	injectInMainWorld: true,
	url: "tst_WebView_scriptMainWorld_user_script.js"});

      webView.context.addUserScript(script);

      webView.url = "http://localhost:8080/tst_WebView_scriptMainWorld.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var res;
      try {
        res = webView.getTestApi().evaluateCode("\
var el = document.getElementById(\"result\");\
if (!el) throw Exception();\
return el.innerHTML;", true);
      } catch(e) {
        fail("#result not found");
      }

      compare(res, "Main world content script DID NOT found oxide.postMessage",
              "Unexpected result message");
    }

    function test_can_inject_in_main_world_with_oxide_api() {
      webView.context.__injectOxideApiInMainWorld = true;
      var otherContext = webContext.createObject(null, {});
      var script = userScript.createObject(null, {
        context: "oxide-private://main-world-private",
	injectInMainWorld: true,
	url: "tst_WebView_scriptMainWorld_user_script.js"});

      webView.context.addUserScript(script);

      webView.url = "http://localhost:8080/tst_WebView_scriptMainWorld.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var res;
      try {
        res = webView.getTestApi().evaluateCode("\
var el = document.getElementById(\"result\");\
if (!el) throw Exception();\
return el.innerHTML;", true);
      } catch(e) {
        fail("#result not found");
      }

      compare(res, "Main world content script found oxide.postMessage",
              "Unexpected result message");
    }

    function test_receive_message_from_main_world_userscript() {
      var otherContext = webContext.createObject(null, {});
      var script = userScript.createObject(null, {
        context: "oxide-private://main-world-private",
	injectInMainWorld: true,
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

      webView.context.__injectOxideApiInMainWorld = true;
      webView.context.addUserScript(script);

      webView.url = "http://localhost:8080/tst_WebView_scriptMainWorld.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      verify(received != null, "Did not receive message from the main frame's userscript");
      compare(received.data, "mydata" , "Did not receive message from the main frame's userscript");
    }

  }
}
