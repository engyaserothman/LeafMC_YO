import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QGroundControl 1.0
import QGroundControl.Controls 1.0
import QGroundControl.ScreenTools 1.0

Popup {
    id: root
    width: ScreenTools.defaultFontPixelWidth * 40
    height: ScreenTools.defaultFontPixelWidth * 20
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    background: Rectangle {
        color: "black"
        border.color: "gray"
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: ScreenTools.defaultFontPixelWidth

        Label {
            text: qsTr("SiYi Gimbal Settings")
            font.pointSize: ScreenTools.largeFontPointSize
            Layout.alignment: Qt.AlignHCenter
        }

        GridLayout {
            columns: 2
            Layout.fillWidth: true

            Label { text: qsTr("Gimbal IP Address:") }
            TextField {
                id: gimbalIpField
                text: QGroundControl.siyi.gimbalIp
                Layout.fillWidth: true
                color: "white"
                background: Rectangle {
                    color: "#222222"
                    border.color: "gray"
                }
            }

            Label { text: qsTr("Camera Port:") }
            TextField {
                id: cameraPortField
                text: QGroundControl.siyi.cameraPort
                Layout.fillWidth: true
                validator: IntValidator { bottom: 1; top: 65535 }
                color: "white"
                background: Rectangle {
                    color: "#222222"
                    border.color: "gray"
                }
            }

            Label { text: qsTr("Transmitter Port:") }
            TextField {
                id: transmitterPortField
                text: QGroundControl.siyi.transmitterPort
                Layout.fillWidth: true
                validator: IntValidator { bottom: 1; top: 65535 }
                color: "white"
                background: Rectangle {
                    color: "#222222"
                    border.color: "gray"
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: ScreenTools.defaultFontPixelWidth

            Button {
                text: qsTr("Save")
                onClicked: {
                    QGroundControl.siyi.gimbalIp = gimbalIpField.text
                    QGroundControl.siyi.cameraPort = parseInt(cameraPortField.text)
                    QGroundControl.siyi.transmitterPort = parseInt(transmitterPortField.text)
                    QGroundControl.siyi.connectLink()
                    root.close()
                }
            }

            Button {
                text: qsTr("Cancel")
                onClicked: root.close()
            }
        }
    }
}