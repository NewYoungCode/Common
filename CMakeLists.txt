cmake_minimum_required(VERSION 3.5)
if(UNIX)
   add_compile_options(-finput-charset=UTF-8)
   add_compile_options(-fexec-charset=UTF-8)
elseif(WIN32)
   add_compile_options(/utf-8)
endif()
#让所有项目都使用UNICODE
add_definitions(-D_UNICODE -DUNICODE)
# 添加一个选项，供用户选择构建静态库或动态库
option(BUILD_SHARED_LIBS "Build shared library instead of static library" OFF)

set(commonInclude ./include)
set(commonLib ./lib)

#通用库
project(Common)
file(GLOB commonFiles ./include/*.h ./include/*.hpp ./sources/*.cpp)
if(BUILD_SHARED_LIBS)
    add_library(Common SHARED ${commonFiles}) #dll库
else()
    add_library(Common STATIC ${commonFiles}) #lib静态库
endif()
#set_target_properties(Common PROPERTIES DEBUG_POSTFIX "_D") #Common_D
target_include_directories(Common PRIVATE ${commonInclude} ${commonInclude}/curl  ${commonInclude}/mysql ${commonInclude}/occi)
target_link_directories(Common PRIVATE ${commonLib})


#添加子项目
add_subdirectory(./demo)