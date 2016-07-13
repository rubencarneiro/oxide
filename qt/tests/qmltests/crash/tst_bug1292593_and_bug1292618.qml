import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0

Item {
  WebContext {
    id: context
  }

  TestCase {
    id: test
    name: "bug1292593_and_bug1292618"
    when: windowShown
  }

  WebView {
    anchors.fill: parent
    context: context
  }
}
