AC_ADD_SCRIPT_LOADER("mod_conquest_gear_manager" "${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp")

AC_ADD_SCRIPT("ConquestGearManager" "${CMAKE_CURRENT_LIST_DIR}/src/ConquestGearManager.cpp")

AC_ADD_CONFIG_FILE("${CMAKE_CURRENT_LIST_DIR}/conf/conquest_gear_manager.conf.dist")

