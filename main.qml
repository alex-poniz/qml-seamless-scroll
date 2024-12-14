import QtQuick
import QtQuick.Window
import QtQuick.Controls

Window {
    id: demo
    width: 400
    height: 800
    visible: true
    color: "#ff303030"

    property color accentColor: "#ff00b374"
    property int modelSize: appData.MessagesArraySize

    Rectangle {
        width: parent.width - 32
        height: parent.height - 32
        x: 16
        y: 16

        color: demo.accentColor
        clip: true

        ListView {
            id: listView
            reuseItems: false
            width: parent.width
            height: parent.height

            // Clip painting to his own bounding rectangle to avoid display
            // data outside specified size during flick
            clip: true
            model: appData.Messages

            // Increase Flick speed
            maximumFlickVelocity: 10000
            cacheBuffer:1000

            ScrollBar.vertical: ScrollBar {
                id: verticalScrollBar
                active: pressed || listView.moving
                orientation: Qt.Vertical
                opacity: active ? 1:0
                Behavior on opacity {NumberAnimation {duration: 500}}

                contentItem: Rectangle {
                    implicitWidth: 4
                    radius:2
                    implicitHeight: parent.height
                    color: demo.accentColor
                }
            }

            delegate: Rectangle {
                id: lineRectangle
                height: textField.height + 8
                width: listView.width
                //property int yoff: Math.round(lineRectangle.y - listView.contentY)
                //property bool isFullyVisible: (yoff >= listView.y && (yoff + height) <= (listView.y + listView.height))
                //property string Message

                // Change color based on index number for better readability
                color: (index & 1)? "#ffefefef" : "#ffffffff"

                Text {
                    id: textField
                    y: 4
                    //height: parent.height
                    width: parent.width
                    color: listView.currentIndex === index? accentColor:"#ff000000"
                    font.pixelSize: 16
                    horizontalAlignment : Text.AlignLeft
                    wrapMode: Text.Wrap
                    //text: parent.Message + (index) + isFullyVisible
                    text: appData.Messages[index].Message// + isFullyVisible
                }
            }

            property int listview_center_x: listView.x + listView.width / 2
            property int index_up_last: appData.UpBufferSize
            //property int index_up_last: 20
            property int positioningIndex: 0 //appData.MessagesArraySize - 1
            property bool movementLocked: false
            property int currentContentY


            onContentYChanged: {
                var vertVelocity = verticalVelocity
                //console.log("onContentYChanged velocity: " + vertVelocity)

                if (verticalVelocity > 0) {
                    // scrolling down
                    var index_down = indexAt(listview_center_x, listView.y + listView.contentY + listView.height - 10)
                    console.log("index_down: " + index_down)
                } else {
                    // scrolling up
                    var index_up = indexAt(listview_center_x, listView.y + listView.contentY + 10)

                    console.log("index_up_last: " + index_up_last + " index_up: " + index_up + " listView.y " + listView.y +
                                " movementLocked " + movementLocked +  " listView.contentY " + listView.contentY)

                    if (index_up < 0) {
                        console.error("Negative index")
                        return
                    }

                    var prependItemsCount = appData.UpBufferSize - index_up

                    console.log("index_up: " + index_up + " prependItemsCount: " + prependItemsCount)

                    // prepend message stubs to the list
                    if (!movementLocked && prependItemsCount === 1 && index_up_last > index_up) {
                        movementLocked = true;
                        currentContentY = contentY
                        index_up_last = index_up + prependItemsCount

                        appData.prependStubs(prependItemsCount)

                        //positioningIndex = index_up
                        positioningIndex = index_up + prependItemsCount
                        //currentIndex = positioningIndex
                        positionViewAtIndex(positioningIndex, ListView.Beginning)
                        //listView.flick(0, vertVelocity)
                        //console.log("curent index: " + listView.currentIndex)
                        //listView.contentY = listView.contentY + listView. - 10
                    }
                }

                /*
                if (index_up < appData.UpBufferSize) {
                    index_up_last = index_up
                    appData.updateMessagesArrayUp(index_up);
                } */
            }

            function updateContent() {
                var index_up = indexAt(listview_center_x, listView.y + listView.contentY + 10)
                var index_down = indexAt(listview_center_x, listView.y + listView.contentY + listView.height - 10)
                console.log("updateContent: index_up: " + index_up + " index_down: " + index_down)

                if (index_up < 0 || index_down < 0) {
                    console.error("Negative index")
                    return
                }

                // update content of visible items to real one instead of stubs
                /*
                if (index_up < index_up_last && index_up < appData.UpBufferSize) {
                    index_up_last = index_up
                    appData.updateMessagesArrayUp();
                } */

                appData.requestContentUpdate(index_up, index_down)
            }

            onMovementStarted: {
                console.log("onMovementStarted")
            }

            onMovementEnded: {
                updateContent();
                movementLocked = false;
                console.log("onMovementEnded")
            }
/*
            onAdd: {
                console.log("onAdd")
            }

            onRemove: {
                console.log("onRemove")
            } */

            onCurrentIndexChanged: {
                console.log("onCurrentIndexChanged: currentIndex: " + currentIndex)
                 // if (!moving) root.currentIndex = currentIndex
            }

            onMovingChanged: {
                console.log("onMovingChanged: currentIndex: " + currentIndex)

            }

            onModelChanged: {
                console.log("onModelChanged: contentY: " + contentY)
                contentY = currentContentY

                //currentIndex = positioningIndex;
                positionViewAtIndex(positioningIndex, ListView.Beginning)

                movementLocked = false;
            }


            Component.onCompleted: {
                console.log("Component.onCompleted")
                appData.requestLatestMessages(0)
                listView.positionViewAtEnd()
            }
        }
    }

    // Up Button to move list up
}
