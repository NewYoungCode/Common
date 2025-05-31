cmake_minimum_required(VERSION 3.0...4.0)

set(commonInclude ../include)
set(commonLib ../lib)

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

#软件许可注册机
project(RegisterLicenser)
file(GLOB src ./RegisterLicenser/*.*)
add_executable(RegisterLicenser ${src})
target_include_directories(RegisterLicenser PRIVATE ${commonInclude})
target_link_directories(RegisterLicenser PRIVATE ${commonLib})
target_link_libraries(RegisterLicenser PRIVATE Common)


#系统清理工具
project(DiskCleaner)
file(GLOB src ./DiskCleaner/*.*)
add_executable(DiskCleaner ${src})
set_target_properties(DiskCleaner PROPERTIES
    LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\""
)
target_include_directories(DiskCleaner PRIVATE ${commonInclude})
target_link_directories(DiskCleaner PRIVATE ${commonLib})
target_link_libraries(DiskCleaner PRIVATE Common)


#http服务
project(HttpServer)
file(GLOB src ./HttpServer/*.*)
add_executable(HttpServer ${src})
target_include_directories(HttpServer PRIVATE ${commonInclude})
target_link_directories(HttpServer PRIVATE ${commonLib})
target_link_libraries(HttpServer PRIVATE Common)