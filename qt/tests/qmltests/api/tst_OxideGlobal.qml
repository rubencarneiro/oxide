import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0 as Testing

TestCase {
  id: test
  name: "OxideGlobal"
  when: windowShown

  TestResult {
    id: result

    function waitFor(predicate, timeout) {
      timeout = timeout || 5000;
      var end = Date.now() + timeout;
      var i = Date.now();
      while (i < end && !predicate()) {
        wait(50);
        i = Date.now();
      }
      return predicate();
    }
  }

  function test_OxideGlobal1_defaultWebContext() {
    verify(Oxide.defaultWebContext());
    var caught = false;
    try {
      Oxide.defaultWebContext().destroy();
    } catch(e) {
      caught = true; 
    }
    verify(caught, "Shouldn't be able to destroy the default WebContext");
  }

  function test_OxideGlobal2_availableVideoCaptureDevices() {
    result.waitFor(function() { return Oxide.availableVideoCaptureDevices().length > 0; });

    var devices = Oxide.availableVideoCaptureDevices();
    compare(devices.length, 1);
    compare(devices[0].id, "/dev/video0");
    compare(devices[0].displayName.substr(0, 14), "fake_device_0");
    compare(devices[0].position, "unspecified");
  }
}
