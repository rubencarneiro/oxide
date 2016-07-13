import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  focus: true

  property var waitingForMessage: true
  property var lastMessagePayload: null

  messageHandlers: [
    ScriptMessageTestHandler {
      msgId: "TEST-SEND-MESSAGE-NO-REPLY-RESPONSE"
      callback: function(msg) {
        webView.lastMessagePayload = msg.payload;
        webView.waitingForMessage = false;
      }
    }
  ]

  Component.onCompleted: {
    ScriptMessageTestUtils.init(webView.context);
  }

  TestCase {
    id: test
    name: "WebFrame_sendMessageNoReply"
    when: windowShown

    function cleanupTestCase() {
      webView.context.clearTestUserScripts();
    }

    function init() {
      webView.lastMessagePayload = null;
      webView.waitingForMessage = true;
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
    }

    function test_WebFrame_sendMessageNoReply_data() {
      // Whilst we can send null to the renderer side, the browser can't
      // receive null as it gets converted to undefined (there is no difference
      // between null or defined with QVariant).
      // We test undefined here, even though this is bogus JSON
      return [ { payload: 10 },
               { payload: true },
               { payload: 1.65453543 },
               { payload: "This is a string" },
               { payload: undefined },
               { payload: [ "foo", 5, { a: 10, b: 5.7565, c: undefined }, false ] },
               { payload: { a: 7, b: "foo", c: [ 87.243532, true, undefined ] } }
      ];
    }

    function test_WebFrame_sendMessageNoReply(data) {
      var api = new ScriptMessageTestUtils.FrameHelper(webView.rootFrame);
      var res = api.sendMessageNoReply("TEST-SEND-MESSAGE-NO-REPLY", data.payload);

      verify(TestUtils.waitFor(function() { return !webView.waitingForMessage; }));

      compare(webView.lastMessagePayload, data.payload);
    }
  }
}
