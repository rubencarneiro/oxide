import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

Item {

  WebContextDelegateWorker {
    id: worker
    source: Qt.resolvedUrl("tst_bug1445673.js")
  }

  QtObject {
    id: object
  }

  TestCase {
    id: test
    name: "bug1445673"
    when: windowShown

    function test_bug1445673() {
      var data = null;
      function onMessage(message) {
        data = message;
      }
      worker.message.connect(onMessage);

      worker.sendMessage({ test: object });

      var end = Date.now() + 5000;
      var i = Date.now();
      while (i < end && !data) {
        qtest_testResult.wait(50);
        i = Date.now();
      }

      verify(data);
      verify(data.test != object);
    }
  }

  TestResult { id: qtest_testResult }
}
