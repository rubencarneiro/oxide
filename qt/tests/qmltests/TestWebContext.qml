import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.3
import Oxide.testsupport 1.0

WebContext {
  property bool persistent: true

  dataPath: persistent ? QMLTEST_DATADIR : ""

  property var qtest_contextTestSupport: TestSupport.createWebContextTestSupport(this)

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
    "MAP *.testsuite:443 localhost:4443",
    "MAP invalid:80 255.255.255.255:80"
  ]

  property var qtest_cookiesDeletedSpy: SignalSpy {
    target: cookieManager
    signalName: "deleteCookiesResponse"
  }

  function deleteAllCookies() {
     var id = cookieManager.deleteAllCookies();

     var end = Date.now() + 5000;
     var i = Date.now();
     while (i < end) {
       qtest_cookiesDeletedSpy.clear();
       qtest_cookiesDeletedSpy.wait();
       if (qtest_cookiesDeletedSpy.signalArguments[0][0] == id) {
         return;
       }
       i = Date.now();
     }

     throw new Error("Timeout whilst deleting cookies");
  }

  function clearTemporarySavedPermissionStatuses() {
    qtest_contextTestSupport.clearTemporarySavedPermissionStatuses();
  }

  property var qtest_UserScriptFactory: Component {
    UserScript {}
  }

  function addTestUserScript(props) {
    var script = qtest_UserScriptFactory.createObject(null, props);
    addUserScript(script);
  }

  function clearTestUserScripts() {
    while (userScripts().length > 1) {
      var script = userScripts()[1];
      removeUserScript(script);
      script.destroy();
    }
  }
}
