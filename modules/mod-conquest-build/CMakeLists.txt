AC_ADD_SCRIPT_LOADER("mod_conquest_build" "${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp")

AC_ADD_SCRIPT("ConquestBuildMgr" "${CMAKE_CURRENT_LIST_DIR}/src/ConquestBuildMgr.cpp")
AC_ADD_SCRIPT("ConquestBuildItem" "${CMAKE_CURRENT_LIST_DIR}/src/ConquestBuildItem.cpp")
AC_ADD_SCRIPT("ConquestBuildGameObject" "${CMAKE_CURRENT_LIST_DIR}/src/ConquestBuildGameObject.cpp")

AC_ADD_CONFIG_FILE("${CMAKE_CURRENT_LIST_DIR}/conf/conquest_build.conf.dist")

