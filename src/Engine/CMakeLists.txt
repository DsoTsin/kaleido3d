#include_directories(${RAPIDJSON_INCLUDE_DIR})
include_directories(
	.. 
	../../Include
)

set(SRC_CAMERA			Assets/CameraData.h Assets/CameraData.cpp)
set(SRC_MESH			Assets/MeshData.h Assets/MeshData.cpp Assets/ObjectMesh.h Assets/ObjectMesh.cpp Assets/RiggedMeshData.h Assets/RiggedMeshData.cpp)
set(SRC_IMAGE			Assets/ImageData.h Assets/ImageData.cpp)
set(SRC_BUNDLE	        Assets/Bundle.h Assets/Bundle.cpp)

source_group(Asset				FILES ${SRC_BUNDLE})
source_group("Asset\\Mesh"		FILES ${SRC_MESH})
source_group("Asset\\Camera"	FILES ${SRC_CAMERA})
source_group("Asset\\Image"		FILES ${SRC_IMAGE})

set(ASSET_SRCS ${SRC_BUNDLE} ${SRC_CAMERA} ${SRC_MESH} ${SRC_IMAGE})

set(ENGINE_SRCS
	Engine.h
	Engine.cpp
)

source_group(
	Engine FILES
	${ENGINE_SRCS}
)

set(CAMERA_SRCS
	Camera.h
	Camera.cpp
	FollowCamera.h
	FollowCamera.cpp
	FreeCamera.h
	FreeCamera.cpp
	TargetCamera.h
	TargetCamera.cpp
)

source_group(
	Camera FILES
	${CAMERA_SRCS}
)

set(SCENE_SRCS
	SceneObject.h
	SceneObject.cpp
	SceneManager.h
	SceneManager.cpp
	OctreeManager.h
	OctreeManager.cpp
)

set(MAT_SRCS
	Material.h
	Material.cpp
	MaterialGroup.h
	MaterialGroup.cpp
	MaterialManager.h
	MaterialManager.cpp
)

source_group(
	Material FILES
	${MAT_SRCS}
)

source_group(
	Scene FILES
	${SCENE_SRCS}
)

set(RENDERER_SRCS
	RendererFactory.h
	RendererFactory.cpp
	RenderResourceManager.h
	RenderResourceManager.cpp
)

source_group(
	Renderer FILES
	${RENDERER_SRCS}
)

set(PARAM_SRCS
	ShaderParameter.h
	ShaderParameter.cpp
)

source_group(
	Parameter FILES
	${PARAM_SRCS}
)

k3d_add_lib(Engine 
SRCS 
	${CAMERA_SRCS}
	${SCENE_SRCS}
	${RENDERER_SRCS}
	${ENGINE_SRCS}
	${MAT_SRCS}
	${PARAM_SRCS} 
    ${ASSET_SRCS}
LIBS 
	Kaleido3D.Core 
FOLDER 
	"Runtime"
PCH
    "Kaleido3D.h"
PCH_SRC
    "${Kaleido3D_ROOT_DIR}/Source/Engine/Engine.cpp"
PCH_INCLUDE
    "${Kaleido3D_ROOT_DIR}/Include"
)


install(TARGETS Kaleido3D.Engine 
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin)
	
if(BUILD_WITH_UNIT_TEST)
add_subdirectory(UnitTest)
endif()