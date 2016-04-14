import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

TestCase {
  id: test
  name: "OxideGlobal"
  when: windowShown

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
    TestUtils.waitFor(function() { return Oxide.availableVideoCaptureDevices.length > 0; });

    var devices = Oxide.availableVideoCaptureDevices;
    compare(devices.length, 1);
    compare(devices[0].id, "/dev/video0");
    compare(devices[0].displayName.substr(0, 13), "fake_device_0");
    compare(devices[0].position, "unspecified");
  }

  function test_OxideGlobal3_chromiumVersion() {
    verify(/^\d+\.\d+\.\d+\.\d+$/.test(Oxide.chromiumVersion));
  }

  function test_OxideGlobal3_oxideVersion() {
    verify(/^\d+\.\d+\.\d+$/.test(Oxide.version));
  }
}
