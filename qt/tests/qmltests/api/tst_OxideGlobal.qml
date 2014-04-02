import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0 as Testing

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
}
