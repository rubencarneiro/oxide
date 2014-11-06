import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.3
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

  hostMappingRules: [
    "MAP testsuite:80 localhost:8080",
    "MAP *.testsuite:80 localhost:8080",
    "MAP testsuite.com:80 localhost:8080",
    "MAP *.testsuite.com:80 localhost:8080",
    "MAP expired.testsuite:443 localhost:4444",
    "MAP selfsigned.testsuite:443 localhost:4445",
    "MAP badidentity.testsuite:443 localhost:4446",
    "MAP testsuite:443 localhost:4443",
    "MAP *.testsuite:443 localhost:4443"
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
