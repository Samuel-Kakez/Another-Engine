cmake_minimum_required(VERSION 3.15)
project(PBRE VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------
# 1. Find Packages
# ---------------------------------------------------------
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

# ---------------------------------------------------------
# 2. Define Source Files
# ---------------------------------------------------------
set(ENGINE_SOURCES
    src/Core/main.cpp
    src/Core/Engine.cpp
    src/Core/GameObject.cpp
    src/Core/Scene.cpp
    src/Core/Transform.cpp
    src/Core/ComponentFactory.cpp
    src/Components/MeshRenderer.cpp
    src/Components/DirectionalLight.cpp
    src/Components/FreeCamController.cpp
    src/Math/Vector2.cpp
    src/Math/Vector3.cpp
    src/Math/Matrix4x4.cpp
    src/Math/Quaternion.cpp
    src/Math/Matrix3x3.cpp
    src/Math/AABB.cpp
    src/Math/Frustum.cpp
    src/Math/Plane.cpp
    src/Renderer/Renderer.cpp
    src/Renderer/Camera.cpp
    src/Renderer/Shader.cpp
    src/Renderer/Texture.cpp
    src/Renderer/FullscreenQuad.cpp
    src/Renderer/Material.cpp
    src/Renderer/Geometry/Mesh.cpp
    src/Renderer/Geometry/Model.cpp
    src/Renderer/Buffers/UniformBuffer.cpp
    src/Renderer/Buffers/FrameBuffer.cpp
    src/Renderer/Passes/LightingPass.cpp
    src/Renderer/Passes/ShadowPass.cpp
    src/Utils/SceneSerializer.cpp
    src/Managers/ResourceManager.cpp
    src/Managers/LightManager.cpp
    src/Managers/InputManager.cpp
    src/Debug/StatsManager.cpp
    src/Debug/DebugUI.cpp
    src/Debug/Logger.cpp
)

# ---------------------------------------------------------
# 3. Create Executable
# ---------------------------------------------------------
add_executable(${PROJECT_NAME} 
    ${ENGINE_SOURCES}
)

# ---------------------------------------------------------
# 4. Include Directories
# ---------------------------------------------------------
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${Stb_INCLUDE_DIR}  # This is how we use Stb!
)

# ---------------------------------------------------------
# 5. Link Libraries (FIXED)
# ---------------------------------------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenGL::GL
    glfw
    assimp::assimp
    imgui::imgui
    # Stb::Stb  <-- REMOVED (This was the error)
    nlohmann_json::nlohmann_json
    glad::glad
)

# ---------------------------------------------------------
# 6. Copy Assets (Post-Build)
# ---------------------------------------------------------
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    COMMENT "Copying assets to build directory"
)

# Copy scene file if it exists at root
if(EXISTS ${CMAKE_SOURCE_DIR}/main.scene.json)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/main.scene.json
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/main.scene.json
        COMMENT "Copying scene file to build directory"
    )
endif()