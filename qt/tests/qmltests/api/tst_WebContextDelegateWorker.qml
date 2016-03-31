import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: top

  Component {
    id: "webContextDelegateWorkerFactory"
    WebContextDelegateWorker {}
  }

  TestCase {
    id: test
    name: "WebContextDelegateWorker"
    when: windowShown

    function test_WebContextDelegateWorker1_source_construct_only() {
      var d = webContextDelegateWorkerFactory.createObject(
          null, { source: Qt.resolvedUrl("tst_WebContextDelegateWorker.js") });
      var s = d.source;
      d.source = "file:///foo";
      compare(d.source, s, "WebContextDelegateWorker.source should be construct only");
    }

    function test_WebContextDelegateWorker2_source_file_only() {
      var d = webContextDelegateWorkerFactory.createObject(
          null, { source: "http://foo/" });
      compare(d.source, "", "WebContextDelegateWorker.source should only support file URLs");
    }

    function test_WebContextDelegateWorker3_messaging() {
      var d = webContextDelegateWorkerFactory.createObject(
          null, { source: Qt.resolvedUrl("tst_WebContextDelegateWorker.js") });

      var receivedMsg = null;
      d.message.connect(function(m) { receivedMsg = m; });

      var msg = { foo: "bar", baz: 10 };
      d.sendMessage(msg);
      verify(TestUtils.waitFor(function() { return receivedMsg != null; }),
             "Timed out waiting for a response");
      compare(receivedMsg, msg, "Unexpected response");
    }
  }
}
