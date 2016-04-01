import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.9
import Oxide.testsupport 1.0

TestCase {
  id: test
  name: "WebContext_defaultVideoCaptureDevice"

  Component {
    id: webContextFactory
    TestWebContext {}
  }

  Component {
    id: webViewFactory
    TestWebView {}
  }

  SignalSpy {
    id: spy
    signalName: "defaultVideoCaptureDeviceIdChanged"
  }

  function init() {
    spy.clear();
  }

  // Verify that defaultVideoCaptureDeviceId is initially empty
  function test_WebContext_defaultVideoCaptureDevice1() {
    var context = webContextFactory.createObject(null, {});
    compare(context.defaultVideoCaptureDeviceId, "");
  }

  // Verify that defaultVideoCaptureDeviceId can be set to a valid device
  // and back again
  function test_WebContext_defaultVideoCaptureDevice2() {
    TestUtils.waitFor(function() { return Oxide.availableVideoCaptureDevices.length > 0; });
    var context = webContextFactory.createObject(null, {});
    spy.target = context;

    var id = Oxide.availableVideoCaptureDevices[0].id;
    context.defaultVideoCaptureDeviceId = id;

    compare(spy.count, 1);
    compare(context.defaultVideoCaptureDeviceId, id);

    context.defaultVideoCaptureDeviceId = "";

    compare(spy.count, 2);
    compare(context.defaultVideoCaptureDeviceId, "");
  }

  // Verify that defaultVideoCaptureDeviceId can't be set to an invalid
  // device.
  // XXX(chrisccoulson): Note, this currently only works once a WebContext is
  // "in-use", due to the way initialization works
  function test_WebContext_defaultVideoCaptureDevice3() {
    var context = webContextFactory.createObject(null, {});
    // We need a webview to activate this context for this test to work
    var view = webViewFactory.createObject(null, { context: context });
    spy.target = context;

    context.defaultVideoCaptureDeviceId = "AAAAAAAAAAAAA";

    compare(spy.count, 0);
    compare(context.defaultVideoCaptureDeviceId, "");
  }

  // XXX(chrisccoulson): How do we verify that defaultVideoCaptureDeviceId
  //  actually works?
  // XXX(chrisccoulson): Figure out a way to test what happens if the default
  //  device is removed from the system
}
