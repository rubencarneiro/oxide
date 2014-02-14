import QtQuick 2.0
import Ubuntu.Components 0.1
import com.canonical.Oxide 0.1

MainView {
    objectName: "mainView"
    applicationName: "com.ubuntu.developer.oxide-developers.oxide-test-browser"
    automaticOrientation: true

    width: units.gu(100)
    height: units.gu(75)

    Page {
        id: page
        anchors.fill: parent

        //property var url: "http://people.canonical.com/~jamie/oxide-test/index.html"
        property var url: Qt.resolvedUrl("./index.html")
        //property var url: "http://start.ubuntu.com/"

        function updateWebView() {
            webView.url = locationField.text
        }

        Button  {
            id: backButton
            text: "Back"
            height: locationField.height
            anchors  {
                left: parent.left
            }
            onClicked: {
                webView.goBack()
            }
        }

        Button  {
            id: forwardButton
            text: "Forward"
            height: locationField.height
            anchors  {
                left: backButton.right
            }
            onClicked: {
                webView.goForward()
            }
        }

        TextField {
            id: locationField
            width: parent.width - backButton.width - forwardButton.width - goButton.width
            text: page.url
            anchors  {
                left: forwardButton.right
            }
            Keys.onReturnPressed: {
                page.updateWebView()
            }
        }

        Button  {
            id: goButton
            text: "Go"
            height: locationField.height
            anchors  {
                left: locationField.right
            }

            onClicked: {
                page.updateWebView()
            }
        }

        WebView {
            id: webView
            width: parent.width
            height: parent.height - backButton.height - statusLabel.height
            anchors {
                bottom: statusLabel.top
            }
            url: page.url
            focus: true

            onUrlChanged: {
                locationField.text = url
            }
        }

        Label {
            id: statusLabel
            text: webView.loading ? "Loading (%1%%)".arg(webView.loadProgress) : "Page loaded"
            width: parent.width
            anchors  {
                bottom: parent.bottom
            }
        }
    }
}
