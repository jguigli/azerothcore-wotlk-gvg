AC_ADD_SCRIPT_LOADER("mod_conquest_group_cap" "${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp")

AC_ADD_SCRIPT("ConquestGroupCap" "${CMAKE_CURRENT_LIST_DIR}/src/ConquestGroupCap.cpp")

AC_ADD_CONFIG_FILE("${CMAKE_CURRENT_LIST_DIR}/conf/conquest_group_cap.conf.dist")
