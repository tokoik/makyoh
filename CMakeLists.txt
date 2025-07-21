cmake_minimum_required(VERSION 3.10)
project(Makyoh)

# C++17 の標準を使用
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Makyoh のソースファイル
set(SOURCE_FILES
    makyoh.cpp
    parseconfig.h
    Config.h
    Config.cpp
    Menu.h
    Menu.cpp
    main.cpp
    Rect.h
    Rect.cpp
    GgApp.h
    GgApp.cpp
    gg.h
    gg.cpp
)

# ImGui のソースファイル
set(IMGUI_SOURCES
    lib/imgui.cpp
    lib/imgui_draw.cpp
    lib/imgui_tables.cpp
    lib/imgui_widgets.cpp
    lib/imgui_impl_opengl3.cpp
    lib/imgui_impl_glfw.cpp
)

# クロスプラットフォーム対応のソースファイル
if(APPLE)
    # macOS の場合
    list(APPEND SOURCE_FILES
        lib/nfd_cocoa.m
    )
    # Objective-C の言語を有効にする
    set(CMAKE_OBJC_STANDARD 11)
    set(CMAKE_OBJC_STANDARD_REQUIRED ON)
elseif(UNIX)
    # Ubuntu の場合
    list(APPEND SOURCE_FILES
        lib/nfd_gtk.cpp
    )
    # GTK のライブラリを検索
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK REQUIRED gtk+-3.0)
elseif(WIN32)
    # Windows の場合
    list(APPEND SOURCE_FILES
        lib/nfd_win.cpp
        resource.h
        resource.rc
    )
endif()

# 実行ファイルを作成
add_executable(Makyoh ${SOURCE_FILES} ${IMGUI_SOURCES})

# ヘッダファイルのインクルードディレクトリを設定
target_include_directories(Makyoh PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}
)

# クロスプラットフォーム対応のリンク設定
if(APPLE)
    # macOS の場合
    target_link_libraries(Makyoh PUBLIC
        -L${CMAKE_SOURCE_DIR}/lib -lglfw3
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework CoreFoundation"
        "-framework UniformTypeIdentifiers"
    )
elseif(UNIX)
    # Linux (Ubuntu) の場合
    target_link_libraries(Makyoh PUBLIC
        # Ubuntu にはシステムライブラリを使用
        glfw
        OpenGL::GL
    )
elseif(WIN32)
    # Windows (Visual Studio) の場合
    # 32bit と 64bit を判別
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(PLATFORM "x64")
    else()
        set(PLATFORM "Win32")
    endif()
    # Debug または Release
    target_link_libraries(Makyoh PUBLIC
        # デバッグビルド
        debug
        lib/${PLATFORM}/Debug/glfw3.lib
        # リリースビルド
        optimized
        lib/${PLATFORM}/Release/glfw3.lib
        opengl32.lib
  )
endif()
