cmake_minimum_required(VERSION 3.16)
project(BrotatoGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)

set(SOURCES
    src/main.cpp
    src/Game.cpp
    src/Player.cpp
    src/Enemy.cpp
    src/SlimeEnemy.cpp
    src/PebblinEnemy.cpp
    src/Bullet.cpp
    src/Vector2.cpp
    src/ExperienceOrb.cpp
    src/Material.cpp
    src/SpeedUpBooster.cpp
    src/Weapon.cpp
    src/Shop.cpp
    src/BossEnemy.cpp
    src/MiniBossEnemy.cpp
    src/FractalNode.cpp
    src/FractalBoss.cpp
)

set(HEADERS
    src/Game.h
    src/Player.h
    src/Enemy.h
    src/SlimeEnemy.h
    src/PebblinEnemy.h
    src/Bullet.h
    src/Vector2.h
    src/ExperienceOrb.h
    src/Material.h
    src/SpeedUpBooster.h
    src/Weapon.h
    src/Shop.h
    src/BossEnemy.h
    src/MiniBossEnemy.h
    src/FractalNode.h
    src/FractalBoss.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} 
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
    $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
)

# Copy monster assets to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/monsters
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/monsters
    COMMENT "Copying monster assets to build directory"
)

# Copy UI assets
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    COMMENT "Copying UI assets to build directory"
)