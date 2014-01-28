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
    id: rootFrameSpy
    target: webView
    signalName: "rootFrameChanged"
  }

  Item {
    id: spy

    readonly property alias eventLog: spy.qtest_eventLog

    function clear() {
      qtest_eventLog = [];
    }

    function childFrameChanged(type, frame) {
      qtest_eventLog.push({ type: type, frame: frame.toString() });

      if (type == WebFrame.ChildAdded) {
        frame.childFrameChanged.connect(childFrameChanged);
      }      
    }

    function rootFrameChanged() {
      webView.rootFrame.childFrameChanged.connect(childFrameChanged);
    }

    property var qtest_eventLog: []

    Component.onCompleted: {
      webView.rootFrameChanged.connect(rootFrameChanged);
    }
  }

  TestCase {
    id: test
    name: WebFrame_tree
    when: windowShown

    function init() {
      webView.url = "about:blank";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      spy.clear();
      rootFrameSpy.clear();
    }

    function verify_events(data) {
      compare(spy.eventLog.length, data.length, "Unexpected number of events");
      for (var i = 0; i < data.length; ++i) {
        if (i >= spy.eventLog.length) {
          break;
        }
        compare(spy.eventLog[i].type, data[i].type, "Unexpected event type");
        compare(spy.eventLog[i].frame, data[i].frame, "Unexpected frame");
      }

      spy.clear();
    }

    function verify_tree() {
      compare(webView.rootFrame.parentFrame, null,
              "Root frame should have no parent");

      var queue = [];
      queue.push(webView.rootFrame);

      while(queue.length > 0) {
        var frame = queue.shift();
        for (var i = 0; i < frame.childFrames.length; ++i) {
          compare(frame.childFrames[i].parentFrame, frame,
                  "Incorrect parent");
          queue.push(frame.childFrames[i]);
        }
      }
    }

    function test_WebFrame_tree1_navigation() {
      webView.url = "http://localhost:8080/tst_WebFrame_tree1.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(rootFrameSpy.count, 0, "Should have the same root frame");

      var frames1 = [
        webView.rootFrame.childFrames[0].toString(),
        webView.rootFrame.childFrames[1].toString()
      ];

      verify_events([
        { type: WebFrame.ChildAdded, frame: frames1[0] },
        { type: WebFrame.ChildAdded, frame: frames1[1] }
      ]);
      verify_tree();

      webView.url = "http://localhost:8080/tst_WebFrame_tree2.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(rootFrameSpy.count, 0, "Should have the same root frame");

      var frames2 = [
        webView.rootFrame.childFrames[0].toString(),
        webView.rootFrame.childFrames[1].toString(),
        webView.rootFrame.childFrames[0].childFrames[0].toString(),
        webView.rootFrame.childFrames[0].childFrames[1].toString(),
      ];

      verify_events([
        { type: WebFrame.ChildRemoved, frame: frames1[1] },
        { type: WebFrame.ChildRemoved, frame: frames1[0] },
        { type: WebFrame.ChildAdded, frame: frames2[0] },
        { type: WebFrame.ChildAdded, frame: frames2[1] },
        { type: WebFrame.ChildAdded, frame: frames2[2] },
        { type: WebFrame.ChildAdded, frame: frames2[3] }
      ]);
      verify_tree();

      webView.goBack();
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(rootFrameSpy.count, 0, "Should have the same root frame");

      frames1 = [
        webView.rootFrame.childFrames[0].toString(),
        webView.rootFrame.childFrames[1].toString()
      ];

      verify_events([
        { type: WebFrame.ChildRemoved, frame: frames2[1] },
        { type: WebFrame.ChildRemoved, frame: frames2[3] },
        { type: WebFrame.ChildRemoved, frame: frames2[2] },
        { type: WebFrame.ChildRemoved, frame: frames2[0] },
        { type: WebFrame.ChildAdded, frame: frames1[0] },
        { type: WebFrame.ChildAdded, frame: frames1[1] }
      ]);
      verify_tree();
    }

    function test_WebFrame_tree2_crossRVHNavigation() {
      webView.url = "http://localhost:8080/tst_WebFrame_tree2.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(rootFrameSpy.count, 1, "Should have a new root frame");

      var frames = [
        webView.rootFrame.childFrames[0].toString(),
        webView.rootFrame.childFrames[1].toString(),
        webView.rootFrame.childFrames[0].childFrames[0].toString(),
        webView.rootFrame.childFrames[0].childFrames[1].toString(),
      ];

      verify_events([
        { type: WebFrame.ChildAdded, frame: frames[0] },
        { type: WebFrame.ChildAdded, frame: frames[1] },
        { type: WebFrame.ChildAdded, frame: frames[2] },
        { type: WebFrame.ChildAdded, frame: frames[3] }
      ]);
      verify_tree();

      webView.url = "about:blank";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(rootFrameSpy.count, 2, "Should have a new root frame");

      verify_events([
        { type: WebFrame.ChildRemoved, frame: frames[1] },
        { type: WebFrame.ChildRemoved, frame: frames[3] },
        { type: WebFrame.ChildRemoved, frame: frames[2] },
        { type: WebFrame.ChildRemoved, frame: frames[0] }
      ]);
      verify_tree();

      webView.goBack();
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(rootFrameSpy.count, 3, "Should have a new root frame");

      frames = [
        webView.rootFrame.childFrames[0].toString(),
        webView.rootFrame.childFrames[1].toString(),
        webView.rootFrame.childFrames[0].childFrames[0].toString(),
        webView.rootFrame.childFrames[0].childFrames[1].toString(),
      ];

      verify_events([
        { type: WebFrame.ChildAdded, frame: frames[0] },
        { type: WebFrame.ChildAdded, frame: frames[1] },
        { type: WebFrame.ChildAdded, frame: frames[2] },
        { type: WebFrame.ChildAdded, frame: frames[3] }
      ]);
      verify_tree();
    }
  }
}
