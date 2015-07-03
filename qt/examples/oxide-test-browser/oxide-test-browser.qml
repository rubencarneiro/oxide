import QtQuick 2.0
import Ubuntu.Components 0.1
import com.canonical.Oxide 1.7

MainView {
    objectName: "mainView"
    applicationName: "com.ubuntu.developer.oxide-developers.oxide-test-browser"
    automaticOrientation: true

    width: units.gu(100)
    height: units.gu(75)

    Page {
        id: page
        anchors.fill: parent

        property var url: Qt.resolvedUrl("./index.html")
        //property var url: "http://start.ubuntu.com/"

        function updateWebView() {
            webView.url = locationField.text
        }

        Column {
            anchors.fill: parent
            Row {
                id: navRow
                width: parent.width
                height: locationField.height
                Button  {
                    id: backButton
                    iconSource: "image://theme/go-previous"
                    height: locationField.height
                    width: height

                    onClicked: {
                        webView.goBack()
                    }
                }

                Button  {
                    id: forwardButton
                    iconSource: "image://theme/go-next"
                    height: locationField.height
                    width: height

                    onClicked: {
                        webView.goForward()
                    }
                }

                TextField {
                    id: locationField
                    width: parent.width - backButton.width - forwardButton.width
                    text: page.url

                    onAccepted: {
                        page.updateWebView()
                    }
                }
            }

            WebView {
                id: webView
                width: parent.width
                height: parent.height - navRow.height - statusLabel.height
                url: page.url
                focus: true

                onUrlChanged: {
                    locationField.text = url
                }
            }

            Label {
                id: statusLabel
                text: webView.loading ?
                      i18n.tr("Loading") + " (%1%)".arg(webView.loadProgress) :
                      i18n.tr("Page loaded")
                width: parent.width
            }
        }
    }
}
