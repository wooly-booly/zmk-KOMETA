if(CONFIG_ZMK_PM_SOFT_OFF AND CONFIG_DT_HAS_ZMK_BEHAVIOR_KOMETA_SOFT_OFF_ENABLED)
  # Register before ZMK's combo and hold-tap listeners can capture key events.
  target_sources(app PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/behavior_kometa_soft_off.c)
endif()
