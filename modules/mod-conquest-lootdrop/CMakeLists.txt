AC_ADD_SCRIPT_LOADER("mod_conquest_lootdrop" "${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp")

AC_ADD_SCRIPT("ConquestLootDrop" "${CMAKE_CURRENT_LIST_DIR}/src/ConquestLootDrop.cpp")

AC_ADD_CONFIG_FILE("${CMAKE_CURRENT_LIST_DIR}/conf/conquest_lootdrop.conf.dist")

