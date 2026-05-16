AC_ADD_SCRIPT_LOADER("mod_conquest_mounts" "${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp")

AC_ADD_SCRIPT("ConquestMounts" "${CMAKE_CURRENT_LIST_DIR}/src/ConquestMounts.cpp")

AC_ADD_CONFIG_FILE("${CMAKE_CURRENT_LIST_DIR}/conf/conquest_mounts.conf.dist")

