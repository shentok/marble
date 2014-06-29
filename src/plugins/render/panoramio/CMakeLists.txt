PROJECT(Panoramio)

INCLUDE_DIRECTORIES(
 ${CMAKE_CURRENT_SOURCE_DIR}
 ${CMAKE_CURRENT_BINARY_DIR}
 ${QT_INCLUDE_DIR}
)
if( QT4_FOUND )
  INCLUDE(${QT_USE_FILE})
endif()

set( panoramio_SRCS PanoramioPlugin.cpp
                    PanoramioModel.cpp
                    PanoramioItem.cpp
                    PanoramioParser.cpp )

set( panoramio_UI ConfigWidget.ui )
qt_wrap_ui( panoramio_SRCS ${panoramio_UI} )

qt_add_resources( panoramio_SRCS panoramio.qrc )

marble_add_plugin( Panoramio ${panoramio_SRCS} )
