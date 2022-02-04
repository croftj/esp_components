idf_component_register(SRCS
                    "OneWireNg_BitBang.cpp"
                    "I2CPort.cpp"
                    "MAX7320.cpp"
                    "MPR121.cpp"
                    "OneWireNg.cpp"
                    #                    "IOPort.cpp"
                    "TempSensors.cpp"
                    "esp_exception.cpp"
                    "i2c_cxx.cpp"
                    "esp_event_api.cpp"
                    "esp_event_cxx.cpp"
                    "esp_timer_cxx.cpp"
                    "menu_entry.cpp"
                    "menu_console.cpp"
                    "PID.cpp"
                    "json11.cpp"
                    "misc.cpp"
                    "variant.cpp"
                    INCLUDE_DIRS "include"
#                    REQUIRES console json esp_event pthread)
                    REQUIRES console json driver esp_event esp_timer pthread)

