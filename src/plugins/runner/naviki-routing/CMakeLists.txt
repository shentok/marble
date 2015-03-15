PROJECT( NavikiRoutingPlugin )

INCLUDE_DIRECTORIES(
 ${CMAKE_CURRENT_SOURCE_DIR}
 ${CMAKE_CURRENT_BINARY_DIR}
 ${QT_INCLUDE_DIR}
)
if( QT4_FOUND )
  INCLUDE(${QT_USE_FILE})
else()
INCLUDE_DIRECTORIES(
  ${Qt5WebKitWidgets_INCLUDE_DIRS}
  ${Qt5Xml_INCLUDE_DIRS}
)
endif()

set( naviki_routing_SRCS NavikiRoutingConfigWidget.cpp NavikiRoutingPlugin.cpp NavikiRoutingRunner.cpp )

set( naviki_routing_UI NavikiRoutingConfigWidget.ui )

qt_wrap_ui( naviki_routing_SRCS ${naviki_routing_UI} )

marble_add_plugin( NavikiRoutingPlugin ${naviki_routing_SRCS} )
