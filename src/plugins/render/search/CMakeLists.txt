PROJECT( SearchPlugin )

INCLUDE_DIRECTORIES(
 ${CMAKE_CURRENT_SOURCE_DIR}
 ${CMAKE_CURRENT_BINARY_DIR}
 ${QT_INCLUDE_DIR}
)
if( QT4_FOUND )
  INCLUDE(${QT_USE_FILE})
endif()

set( my_SRCS
  SearchPlugin.cpp
  SearchItem.cpp
  SearchModel.cpp
)

set( search_UI SearchConfigWidget.ui )
qt_wrap_ui( my_SRCS ${search_UI} )
marble_qt4_automoc( ${my_SRCS} )

marble_add_plugin( SearchPlugin ${my_SRCS} )
