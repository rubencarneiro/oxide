import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

Column {
  id: root
  focus: true

  property var webView: null

  Component {
    id: testWebViewComponent

    TestWebView {
      id: webView
      width: 600
      height: 600

      onJavaScriptConsoleMessage: {
        var msg = "[JS] (%1:%2) %3".arg(sourceId).arg(lineNumber).arg(message)
        if (level === WebView.LogSeverityVerbose) {
            console.log(msg)
        } else if (level === WebView.LogSeverityInfo) {
            console.info(msg)
        } else if (level === WebView.LogSeverityWarning) {
            console.warn(msg)
        } else if ((level === WebView.LogSeverityError) ||
                   (level === WebView.LogSeverityErrorReport) ||
                   (level === WebView.LogSeverityFatal)) {
            console.error(msg)
        }
      }
    }
  }

  SignalSpy {
    id: spy
    target: webView ? webView.context : undefined
    signalName: "userScriptsChanged"
  }

  Component {
    id: userScript
    UserScript {}
  }

  Component {
    id: webContext
    TestWebContext {}
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

    function test_can_inject_in_main_world_with_no_oxide_api() {
      var context = webContext.createObject(null, {__injectOxideApiInMainWorld: false});
      var script = userScript.createObject(null, {
        context: "oxide-private://main-world-private",
        injectInMainWorld: true,
        url: "tst_WebView_scriptMainWorld_user_script.js"});

      root.webView = testWebViewComponent.createObject(root, { context: context });
      root.webView.context.addUserScript(script);

      root.webView.url = "http://localhost:8080/tst_WebView_scriptMainWorld.html";
      verify(root.webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var testApi = root.webView.getTestApi();
      var res = root.webView.waitFor(function () { return resultUpdated(testApi); })

      compare(res, "Main world content script DID NOT found oxide.sendMessage",
              "Unexpected result message");
    }

    function test_oxide_api_in_main_scope_when_flag_set() {
      var context =  webContext.createObject(null, {__injectOxideApiInMainWorld: true});
      webView = testWebViewComponent.createObject(root, { context: context });
      webView.url = "http://localhost:8080/tst_WebView_scriptMainWorld.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var testApi = root.webView.getTestApi();
      var res = root.webView.waitFor(function () { return resultUpdated(testApi); })

      compare(res, "oxide.sendMessage",
              "Unexpected result message");
    }

    function test_can_inject_in_main_world_with_oxide_api() {
      var context =  webContext.createObject(null, {__injectOxideApiInMainWorld: true});
      var script = userScript.createObject(null, {
        context: "oxide-private://main-world-private",
        injectInMainWorld: true,
        url: "tst_WebView_scriptMainWorld_user_script.js"});

      webView = testWebViewComponent.createObject(root, { context: context });
      webView.context.addUserScript(script);

      webView.url = "http://localhost:8080/tst_WebView_scriptMainWorld.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var testApi = root.webView.getTestApi();
      var res = root.webView.waitFor(function () { return resultUpdated(testApi); })

      compare(res, "Main world content script found oxide.sendMessage",
              "Unexpected result message");
    }

    function test_receive_message_from_main_world_userscript() {
      var context = webContext.createObject(null, {__injectOxideApiInMainWorld: true});
      var script = userScript.createObject(null, {
        context: "oxide-private://main-world-private",
        injectInMainWorld: true,
        url: "tst_WebView_scriptMainWorld_user_script.js"});

      webView = testWebViewComponent.createObject(root, { context: context });
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

      webView.context.addUserScript(script);

      webView.url = "http://localhost:8080/tst_WebView_scriptMainWorld.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      verify(received != null, "Did not receive message from the main frame's userscript");
      compare(received.data, "mydata" , "Did not receive message from the main frame's userscript");
    }

  }
}
