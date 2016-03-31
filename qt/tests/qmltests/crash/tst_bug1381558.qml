import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

Item {
  width: 200
  height: 200

  TestWebView {
    id: webView
    anchors.fill: parent
  }

  Timer {
    id: timer
    interval: 100
    repeat: true
    running: false

    onTriggered: {
      webView.visible = !webView.visible;
    }
  }

  SignalSpy {
    id: spy
    target: webView
    signalName: "visibleChanged"
  }

  TestCase {
    id: test
    name: "bug1381558"
    when: windowShown

    function test_bug1381558() {
      webView.url = "http://testsuite/tst_bug1381558.html";
      verify(webView.waitForLoadSucceeded());

      timer.start();
      TestUtils.waitForSignalSpyCount(spy, 100);
      timer.stop();
    }
  }
}
