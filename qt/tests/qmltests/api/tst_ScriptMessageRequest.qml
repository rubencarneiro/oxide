import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 0.1
import com.canonical.Oxide.Testing 0.1

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  SignalSpy {
    id: spy
  }

  function init() {
    webView.url = "http://localhost:8080/empty.html";
    verify(webView.waitForSuccessfulLoad(),
           "Timed out waiting for successful load");
  }

  TestCase {
    id: test
    name: "ScriptMessageRequest"
    when: windowShown

    function test_ScriptMessageRequest_onreply() {
      var req = webView.rootFrame.sendMessage("http://foo/", "FOO", {});
      spy.target = req;
      spy.signalName = "replyCallbackChanged";
      spy.clear();

      var func = function() {};

      req.onreply = func;
      compare(spy.count, 1, "Should have had a signal");
      compare(req.onreply, func, "Unexpected handler");

      req.onreply = req.onreply;
      compare(spy.count, 1, "Shouldn't have had a signal");

      req.onreply = "foo";
      compare(spy.count, 1, "Shouldn't have had a signal");
      compare(req.onreply, func, "Handler should be unchanged");

      req.onreply = null;
      compare(spy.count, 2, "Should have had a signal");
      compare(req.onreply, null, "Unexpected handler");
    }

    function test_ScriptMessageRequest_onerror() {
      var req = webView.rootFrame.sendMessage("http://foo/", "FOO", {});
      spy.target = req;
      spy.signalName = "errorCallbackChanged";
      spy.clear();

      var func = function() {};

      req.onerror = func;
      compare(spy.count, 1, "Should have had a signal");
      compare(req.onerror, func, "Unexpected handler");

      req.onerror = req.onerror;
      compare(spy.count, 1, "Shouldn't have had a signal");

      req.onerror = "foo";
      compare(spy.count, 1, "Shouldn't have had a signal");
      compare(req.onerror, func, "Handler should be unchanged");

      req.onerror = null;
      compare(spy.count, 2, "Should have had a signal");
      compare(req.onerror, null, "Unexpected handler");
    }
  }
}
