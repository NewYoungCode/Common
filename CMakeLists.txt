cmake_minimum_required (VERSION 3.2)
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

#守护进程
project(ProcessWatcher)
file(GLOB src ./ProcessWatcher/*.*)
add_executable(ProcessWatcher ${src})
target_include_directories(ProcessWatcher PRIVATE ${commonInclude})
target_link_directories(ProcessWatcher PRIVATE ${commonLib})
target_link_libraries(ProcessWatcher PRIVATE Common)
#target_compile_definitions(ProcessWatcher PRIVATE WIN32_LEAN_AND_MEAN) #添加一个宏,避免socket冲突

#测试用例
project(test)
file(GLOB src ./test/*.*)
add_executable(test ${src})
target_include_directories(test PRIVATE ${commonInclude})
target_link_directories(test PRIVATE ${commonLib})
target_link_libraries(test PRIVATE Common)
#target_compile_definitions(test PRIVATE WIN32_LEAN_AND_MEAN) #添加一个宏,避免socket冲突