if(UNIX)
   add_compile_options(-finput-charset=UTF-8)
   add_compile_options(-fexec-charset=UTF-8)
elseif(WIN32)
   add_compile_options(/utf-8)
endif()
#让所有项目都使用UNICODE
add_definitions(-D_UNICODE -DUNICODE)

# ===========================================================链接common库,在每个项目的CMakeLists.txt中调用此函数,传入项目名称===========================================================
function(common_add_lib projectName)
    set(common_include_dir "D:/C++/common/include")#根据实际情况修改为你的EzUI库的头文件目录
    set(common_include_lib_dir "D:/C++/common/lib")#根据实际情况修改为你的EzUI依赖库的文件目录
    set(common_build_dir "D:/C++/common/build_x64")#根据实际情况修改为你的EzUI库的构建目录

    # 设置 Debug/Release 类型
    set(Build_Type "$<IF:$<CONFIG:Debug>,Debug,Release>")
    # 包含头文件路径
    target_include_directories(${projectName} PRIVATE ${common_include_dir})
    # 包含库文件目录
    target_link_directories(${projectName} PRIVATE ${common_include_lib_dir})
    # 链接库文件
    target_link_libraries(${projectName} PRIVATE
        "${common_build_dir}/${Build_Type}/common.lib"
    )
endfunction()

#include(CommonConfig.cmake)//在所需Cmakelist.txt文件导入此通用配置
#AddConfig(Your_Target)//为项目添加通用配置

#target_compile_definitions(test PRIVATE WIN32_LEAN_AND_MEAN) #添加一个宏,避免socket冲突
#set_target_properties(DiskCleaner PROPERTIES  LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"" ) #设置管理员启动权限