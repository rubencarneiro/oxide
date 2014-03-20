import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 0.1

Item {
  focus: true

  WebContext {
    id: context
  }

  TestCase {
    id: test
    name: "bug1292593_and_bug1292618"
    when: windowShown
  }

  WebView {
    width: 200
    height: 200
    context: context
  }
}
