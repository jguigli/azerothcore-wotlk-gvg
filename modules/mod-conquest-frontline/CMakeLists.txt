AC_ADD_SCRIPT_LOADER("mod_conquest_frontline" "${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp")

AC_ADD_SCRIPT("OutdoorPvPConquest" "${CMAKE_CURRENT_LIST_DIR}/src/OutdoorPvPConquest.cpp")
AC_ADD_SCRIPT("ConquestFrontlineAuto" "${CMAKE_CURRENT_LIST_DIR}/src/ConquestFrontlineAuto.cpp")
AC_ADD_SCRIPT("ConquestPoints" "${CMAKE_CURRENT_LIST_DIR}/src/ConquestPoints.cpp")
AC_ADD_SCRIPT("ConquestPointsHooks" "${CMAKE_CURRENT_LIST_DIR}/src/ConquestPointsHooks.cpp")
AC_ADD_SCRIPT("ConquestKillStreak" "${CMAKE_CURRENT_LIST_DIR}/src/ConquestKillStreak.cpp")
AC_ADD_SCRIPT("ConquestForcePvP" "${CMAKE_CURRENT_LIST_DIR}/src/ConquestForcePvP.cpp")
AC_ADD_SCRIPT("ConquestEquipBots" "${CMAKE_CURRENT_LIST_DIR}/src/ConquestEquipBots.cpp")
AC_ADD_SCRIPT("ConquestWaypointMgr" "${CMAKE_CURRENT_LIST_DIR}/src/ConquestWaypointMgr.cpp")
AC_ADD_SCRIPT("ConquestCommands" "${CMAKE_CURRENT_LIST_DIR}/src/ConquestCommands.cpp")
AC_ADD_SCRIPT("ConquestDefenseRedispatch" "${CMAKE_CURRENT_LIST_DIR}/src/ConquestDefenseRedispatch.cpp")

AC_ADD_CONFIG_FILE("${CMAKE_CURRENT_LIST_DIR}/conf/conquest_frontline.conf.dist")
