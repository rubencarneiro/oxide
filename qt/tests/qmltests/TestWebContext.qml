import QtQuick 2.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

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
