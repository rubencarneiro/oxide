import QtQuick 2.0
import com.canonical.Oxide 0.1
import com.canonical.Oxide.Testing 0.1

WebContext {
  dataPath: OxideTestingUtils.DATA_PATH
  userScripts: [
    UserScript {
      context: "oxide://testutils/"
      url: Qt.resolvedUrl("TestUtilsSlave.js")
      incognitoEnabled: true
      matchAllFrames: true
    }
  ]
}
