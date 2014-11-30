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

set(ASSET_SRCS
	Model.h
	Model.cpp
)

source_group(
	Asset FILES
	${ASSET_SRCS}
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

include_directories(
	.. 
	../../Include
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

add_library(
	Engine STATIC
	${CAMERA_SRCS}
	${ASSET_SRCS}
	${SCENE_SRCS}
	${RENDERER_SRCS}
	${ENGINE_SRCS}
	${MAT_SRCS}
	${PARAM_SRCS}
)

set_target_properties(Engine PROPERTIES FOLDER "Runtime")
add_precompiled_header(Engine "Kaleido3D.h" "${Kaleido3D_ROOT_DIR}/Include" "${Kaleido3D_ROOT_DIR}/Source/Engine/Engine.cpp")