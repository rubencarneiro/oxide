import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

WebContext {
  dataPath: QMLTEST_USE_CONTEXT_DATADIR ? QMLTEST_DATADIR : ""
  userScripts: [
    UserScript {
      context: "oxide://testutils/"
      url: Qt.resolvedUrl("TestUtilsSlave.js")
      incognitoEnabled: true
      matchAllFrames: true
    }
  ]

  property var _cookiesDeletedSpy: SignalSpy {
    target: cookieManager
    signalName: "deleteCookiesResponse"
  }

  function deleteAllCookies() {
     var id = cookieManager.deleteAllCookies();

     var end = Date.now() + 5000;
     var i = Date.now();
     while (i < end) {
       _cookiesDeletedSpy.clear();
       _cookiesDeletedSpy.wait();
       if (_cookiesDeletedSpy.signalArguments[0][0] == id) {
         return;
       }
       i = Date.now();
     }

     throw new Error("Timeout whilst deleting cookies");
  }
}
