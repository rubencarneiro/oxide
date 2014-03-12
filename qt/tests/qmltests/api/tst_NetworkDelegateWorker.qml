import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 0.1
import com.canonical.Oxide.Testing 0.1

TestWebView {
  id: top

  Component {
    id: "networkDelegateWorkerFactory"
    NetworkDelegateWorker {}
  }

  TestCase {
    id: test
    name: "NetworkDelegateWorker"
    when: windowShown

    function test_NetworkDelegateWorker1_source_construct_only() {
      var d = networkDelegateWorkerFactory.createObject(
          null, { source: Qt.resolvedUrl("tst_NetworkDelegateWorker.js") });
      var s = d.source;
      d.source = "file:///foo";
      compare(d.source, s, "NetworkDelegateWorker.source should be construct only");
    }

    function test_NetworkDelegateWorker2_source_file_only() {
      var d = networkDelegateWorkerFactory.createObject(
          null, { source: "http://foo/" });
      compare(d.source, "", "NetworkDelegateWorker.source should only support file URLs");
    }

    function test_NetworkDelegateWorker3_messaging() {
      var d = networkDelegateWorkerFactory.createObject(
          null, { source: Qt.resolvedUrl("tst_NetworkDelegateWorker.js") });

      var receivedMsg = null;
      d.message.connect(function(m) { receivedMsg = m; });

      var msg = { foo: "bar", baz: 10 };
      d.sendMessage(msg);
      verify(top.waitFor(function() { return receivedMsg != null; }),
             "Timed out waiting for a response");
      compare(receivedMsg, msg, "Unexpected response");
    }
  }
}
