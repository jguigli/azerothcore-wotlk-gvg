AC_ADD_SCRIPT_LOADER("mod_conquest_npc_vendor" "${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp")

AC_ADD_SCRIPT("ConquestVendorMgr" "${CMAKE_CURRENT_LIST_DIR}/src/ConquestVendorMgr.cpp")
AC_ADD_SCRIPT("ConquestVendorScript" "${CMAKE_CURRENT_LIST_DIR}/src/ConquestVendorScript.cpp")
AC_ADD_SCRIPT("ConquestVehicleVendor" "${CMAKE_CURRENT_LIST_DIR}/src/ConquestVehicleVendor.cpp")

AC_ADD_CONFIG_FILE("${CMAKE_CURRENT_LIST_DIR}/conf/conquest_npc_vendor.conf.dist")

