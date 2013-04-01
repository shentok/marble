import QtQuick 1.0
import org.hildon.components 1.0

PageStackWindow {
    height: screen.height
    width: screen.width

    initialPage: mainPage

    MainPage {
        id: mainPage
    }
}
