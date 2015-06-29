import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  ScriptMessageHandler {
    id: handler
  }

  SignalSpy {
    id: spy
    target: handler
  }

  messageHandlers: [
    ScriptMessageHandler {
      msgId: "TEST-THROW"
      contexts: [ "oxide://testutils/" ]
      callback: function(msg) {
        throw Error("This is an error");
      }
    }
  ]

  TestCase {
    id: test
    name: "ScriptMessageHandler"
    when: windowShown

    function init() {
      handler.msgId = ""
      handler.contexts = []
      handler.callback = null
      spy.clear()
    }

    function test_ScriptMessageHandler1_msgId() {
      spy.signalName = "msgIdChanged";

      handler.msgId = "FOO"
      compare(spy.count, 1, "Should have had a signal");
      compare(handler.msgId, "FOO", "Unexpected msgId");

      handler.msgId = handler.msgId;
      compare(spy.count, 1, "Should not have had a signal");
    }

    function test_ScriptMessageHandler2_contexts() {
      spy.signalName = "contextsChanged";

      handler.contexts = [ "http://foo/", "http://bar/" ];
      compare(spy.count, 1, "Should have had a signal");
      compare(handler.contexts.length, 2, "Unexpected number of contexts");
      compare(handler.contexts[0], "http://foo/", "Unexpected context");
      compare(handler.contexts[1], "http://bar/", "Unexpected context");
    }

    function test_ScriptMessageHandler3_callback() {
      spy.signalName = "callbackChanged";

      handler.callback = "foo";
      compare(spy.count, 0, "Shouldn't have had a signal");
      compare(handler.callback, null, "Callback shouldn't have changed");

      var func = function() {};

      handler.callback = func
      compare(spy.count, 1, "Should have had a signal");
      compare(handler.callback, func, "Unexpected callback");

      handler.callback = handler.callback;
      compare(spy.count, 1, "Shouldn't have had a signal");
    }
  }
}
