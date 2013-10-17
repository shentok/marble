PROJECT( QtLocationPlugin )

find_package(QtMobility COMPONENTS Location)

INCLUDE_DIRECTORIES(
 ${CMAKE_CURRENT_SOURCE_DIR}
 ${CMAKE_CURRENT_BINARY_DIR}
 ${QT_INCLUDE_DIR}
 ${QTMOBILITY_INCLUDE_DIR}
 ${QTMOBILITY_LOCATION_INCLUDE_DIR}
)
INCLUDE( ${QT_USE_FILE} )

set( marbleplugin_SRCS
  QtLocationPlugin.cpp
  MarbleGeoMappingManagerEngine.cpp
)

marble_qt4_automoc(${marbleplugin_SRCS})

marble_add_plugin( QtLocationPlugin ${marbleplugin_SRCS})
target_link_libraries( QtLocationPlugin ${QTMOBILITY_LOCATION_LIBRARY} marblewidget )
