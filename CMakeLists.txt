cmake_minimum_required (VERSION 3.2)
#让所有项目都使用UNICODE
add_definitions(-D_UNICODE -DUNICODE)
#安装&下载程序
project(Common)
# 添加一个选项，供用户选择构建静态库或动态库
option(BUILD_SHARED_LIBS "Build shared library instead of static library" OFF)
file(GLOB commonFiles ./Common/include/*.h ./Common/include/*.hpp ./Common/sources/*.cpp)
if(BUILD_SHARED_LIBS)
    add_library(Common SHARED ${commonFiles})
else()
    add_library(Common STATIC ${commonFiles})
endif()
set_target_properties(Common PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(Common PROPERTIES DEBUG_POSTFIX "_D")
target_include_directories(Common PRIVATE ./Common/include ./Common/include/curl  ./Common/include/mysql ./Common/include/occi)
target_link_directories(Common PRIVATE ./Common/lib)


project(test)
file(GLOB src ./test/*.*)
add_executable(test ${src})
target_include_directories(test PRIVATE ./Common/include)
target_link_directories(test PRIVATE ./Common/lib)
target_link_libraries(test PRIVATE Common)
