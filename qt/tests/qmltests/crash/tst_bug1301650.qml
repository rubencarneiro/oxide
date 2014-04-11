import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0

TestCase {
  id: test
  name: "bug1301650"
  when: windowShown

  WebView {
    context: WebContext {
      sessionCookieMode: WebContext.SessionCookieModePersistent
    }
  }
}
