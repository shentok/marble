PROJECT(ThemeSelectFloatItem)

INCLUDE_DIRECTORIES(
 ${CMAKE_CURRENT_SOURCE_DIR}
 ${CMAKE_CURRENT_BINARY_DIR}
 ${QT_INCLUDE_DIR}
)
INCLUDE(${QT_USE_FILE})

set( themeselect_SRCS ThemeSelectFloatItem.cpp )
set (themeselect_UI themeselect.ui)

qt4_add_resources(themeselect_SRCS themeselect.qrc)

if (QTONLY)
  qt4_wrap_ui(themeselect_SRCS ${themeselect_UI})
  qt4_automoc(${themeselect_SRCS})
else (QTONLY)
  include_directories (${KDE4_INCLUDES})
  kde4_add_ui_files(themeselect_SRCS ${themeselect_UI})
endif (QTONLY)

marble_add_plugin( ThemeSelectFloatItem ${themeselect_SRCS} )
