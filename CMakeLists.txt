cmake_minimum_required(VERSION 3.5)

set(srcs 
  "src/Color.cpp"
  "src/AddressableLED.cpp"
  "src/OneWireLED.cpp"
  "src/TwoWireLED.cpp"
)

idf_component_register(
  SRCS "${srcs}" 
  INCLUDE_DIRS "src" 
  REQUIRES soc driver freertos)