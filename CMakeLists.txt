cmake_minimum_required(VERSION 3.0)

set(SRCS
    "src/Color.cpp"
    "src/RmtDriver4.cpp"
    "src/RmtDriver5.cpp"
    "src/SmartLeds.cpp"
)

idf_component_register(
    SRCS ${SRCS}
    INCLUDE_DIRS "./src"
    REQUIRES driver
)
