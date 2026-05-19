cmake_minimum_required(VERSION 3.16)
project(DefensiveSecurityToolkit VERSION 1.0.0 LANGUAGES CXX)

# ── C++ Standard ──────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Compiler Warnings ─────────────────────────────────────────
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wno-unused-parameter)
endif()

# ── Build Type Default ────────────────────────────────────────
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ── Source Files ──────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/HashAlgorithm.cpp
    src/MD5Hash.cpp
    src/SHA1Hash.cpp
    src/SHA256Hash.cpp
    src/PasswordAnalyzer.cpp
    src/BruteForceSimulator.cpp
    src/DictionaryAttackSimulator.cpp
    src/SaltDemonstration.cpp
    src/BenchmarkManager.cpp
    src/Logger.cpp
    src/ReportGenerator.cpp
    src/MenuSystem.cpp
)

# ── Executable ────────────────────────────────────────────────
add_executable(${PROJECT_NAME} ${SOURCES})

# ── Include Directories ───────────────────────────────────────
target_include_directories(${PROJECT_NAME} PRIVATE include)

# ── Threading ─────────────────────────────────────────────────
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

# ── Filesystem (GCC < 9 needs explicit link) ──────────────────
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
    target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
endif()

# ── Output Directories ────────────────────────────────────────
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# ── Create logs/reports dirs on build ─────────────────────────
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_BINARY_DIR}/bin/logs
        ${CMAKE_BINARY_DIR}/bin/reports
    COMMENT "Creating logs and reports directories..."
)

# ── Install ───────────────────────────────────────────────────
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# ── Summary ───────────────────────────────────────────────────
message(STATUS "========================================")
message(STATUS "  Defensive Security Toolkit")
message(STATUS "  Build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler   : ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "========================================")
