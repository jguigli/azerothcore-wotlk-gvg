AC_ADD_SCRIPT_LOADER("mod_conquest_respawn_capital" "${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp")

AC_ADD_SCRIPT("ConquestRespawnCapital" "${CMAKE_CURRENT_LIST_DIR}/src/ConquestRespawnCapital.cpp")

AC_ADD_CONFIG_FILE("${CMAKE_CURRENT_LIST_DIR}/conf/conquest_respawn_capital.conf.dist")
