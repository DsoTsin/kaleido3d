#pragma once

#include <maya/MString.h>
#include <maya/MColor.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MRenderTargetManager.h>
#include <maya/MShaderManager.h>

class MDagPath;
class MBoundingBox;

class viewRenderOverrideMRT : public MHWRender::MRenderOverride
{
public:
	enum
	{
		kTargetClear,
		kMaya3dSceneRender,
		kUserOp,
		kTargetPreview,
		kPresentOp,
		kOperationCount
	};

	enum {
		kTempColor1 = 0,	// Intermediate target 1
		kTempColor2,		// Intermediate target 2
		kTargetCount
	};

	enum {
		kTargetClearShader,		// To clear targets to specific color
		kSceneAttributeShader,	// To render geometry attributes
		kTargetPreviewShader,	// To preview targets
		kShaderCount
	};

	viewRenderOverrideMRT(const MString& name);
	virtual ~viewRenderOverrideMRT();

	virtual MHWRender::DrawAPI supportedDrawAPIs() const;
	virtual bool startOperationIterator();
	virtual MHWRender::MRenderOperation * renderOperation();
	virtual bool nextRenderOperation();
	virtual MStatus setup(const MString& destination);
	virtual MStatus cleanup();
	virtual MString uiName() const
	{
		return mUIName;
	}

protected:
	MStatus updateRenderOperations();
	MStatus updateRenderTargets(MHWRender::MRenderer *theRenderer,
		const MHWRender::MRenderTargetManager *targetManager);
	MStatus updateShaders(const MHWRender::MShaderManager* shaderMgr);

	MString mUIName;
	MColor mClearColor;

	MHWRender::MRenderOperation * mRenderOperations[kOperationCount];
	MString mRenderOperationNames[kOperationCount];
	bool mRenderOperationEnabled[kOperationCount];
	int mCurrentOperation;

	MString mTargetOverrideNames[kTargetCount];
	MHWRender::MRenderTargetDescription* mTargetDescriptions[kTargetCount];
	MHWRender::MRenderTarget* mTargets[kTargetCount];

	MHWRender::MShaderInstance * mShaderInstances[kShaderCount];
};

// Scene render to output to targets
class sceneRenderMRT : public MHWRender::MSceneRender
{
public:
	sceneRenderMRT(const MString &name);
	virtual ~sceneRenderMRT();

	virtual MHWRender::MRenderTarget* const* targetOverrideList(unsigned int &listSize);
	virtual MHWRender::MClearOperation & clearOperation();
	virtual const MHWRender::MShaderInstance* shaderOverride();
	virtual MHWRender::MSceneRender::MSceneFilterOption renderFilterOverride();

	void setRenderTargets(MHWRender::MRenderTarget **targets);
	void setShader(MHWRender::MShaderInstance *shader)
	{
		mShaderInstance = shader;
	}

protected:
	MHWRender::MRenderTarget **mTargets;
	MHWRender::MShaderInstance *mShaderInstance;
};

// Present operation to present to screen
class presentTargetMRT : public MHWRender::MPresentTarget
{
public:
	presentTargetMRT(const MString &name);
	virtual ~presentTargetMRT();

	virtual MHWRender::MRenderTarget* const* targetOverrideList(unsigned int &listSize);

	void setRenderTargets(MHWRender::MRenderTarget **targets);
protected:
	MHWRender::MRenderTarget **mTargets;
};

// Quad render
class quadRenderMRT : public MHWRender::MQuadRender
{
public:
	quadRenderMRT(const MString &name);
	~quadRenderMRT();

	virtual const MHWRender::MShaderInstance * shader();
	virtual MHWRender::MRenderTarget* const* targetOverrideList(unsigned int &listSize);
	virtual MHWRender::MClearOperation & clearOperation();

	void setRenderTargets(MHWRender::MRenderTarget **targets, unsigned int count)
	{
		mOutputTargets = targets;
		mOutputTargetCount = count;
	}
	void setShader(MHWRender::MShaderInstance *shader)
	{
		mShaderInstance = shader;
	}

protected:
	// Shader to use for the quad render
	MHWRender::MShaderInstance *mShaderInstance;
	// Output targets
	MHWRender::MRenderTarget** mOutputTargets;
	unsigned int mOutputTargetCount;
};

// User operation
class userOperationMRT : public MHWRender::MUserRenderOperation
{
public:
	userOperationMRT(const MString &name);
	virtual ~userOperationMRT();

	virtual MStatus execute(const MHWRender::MDrawContext & drawContext);
	virtual MHWRender::MRenderTarget* const* targetOverrideList(unsigned int &listSize);

	void setRenderTargets(MHWRender::MRenderTarget **targets)
	{
		mTargets = targets;
	}

protected:
	// Output targets
	MHWRender::MRenderTarget** mTargets;
};


