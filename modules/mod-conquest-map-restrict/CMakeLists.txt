AC_ADD_SCRIPT_LOADER("mod_conquest_map_restrict" "${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp")

AC_ADD_SCRIPT("ConquestMapRestrict" "${CMAKE_CURRENT_LIST_DIR}/src/ConquestMapRestrict.cpp")

AC_ADD_CONFIG_FILE("${CMAKE_CURRENT_LIST_DIR}/conf/conquest_map_restrict.conf.dist")
