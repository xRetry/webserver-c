cmake_minimum_required(VERSION 3.5)

project(webserver-c VERSION 0.0.1 LANGUAGES C)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME} src/main.c include/mongoose.c)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)
