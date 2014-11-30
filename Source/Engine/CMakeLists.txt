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
	AssetManager.h
	AssetManager.cpp
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
)

source_group(
	Renderer FILES
	${RENDERER_SRCS}
)

add_library(
	Engine STATIC
	${CAMERA_SRCS}
	${ASSET_SRCS}
	${SCENE_SRCS}
	${RENDERER_SRCS}
)