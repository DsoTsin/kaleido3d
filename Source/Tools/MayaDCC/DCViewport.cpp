#include "DCViewport.h"
#include <stdio.h>
#include <maya/MString.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MRenderTargetManager.h>

viewRenderOverrideMRT::viewRenderOverrideMRT(const MString& name)
	: MRenderOverride(name)
	, mUIName("Sample VP2 MRT Override")
{
	unsigned int i = 0;
	for (i = 0; i<kOperationCount; i++)
	{
		mRenderOperations[i] = NULL;
	}
	mCurrentOperation = -1;

	for (i = 0; i<kShaderCount; i++)
	{
		mShaderInstances[i] = NULL;
	}

	unsigned int sampleCount = 1; // no multi-sampling, 16-bit floating point target
	MHWRender::MRasterFormat colorFormat = MHWRender::kR16G16B16A16_FLOAT;

	mTargetOverrideNames[kTempColor1] = MString("_viewRenderOverrideMRT_MRT_Color_Target1__");
	mTargetDescriptions[kTempColor1] =
		new MHWRender::MRenderTargetDescription(mTargetOverrideNames[kTempColor1], 256, 256, sampleCount, colorFormat, 0, false);
	mTargets[kTempColor1] = NULL;

	mTargetOverrideNames[kTempColor2] = MString("_viewRenderOverrideMRT_MRT_Color_Target2__");
	mTargetDescriptions[kTempColor2] =
		new MHWRender::MRenderTargetDescription(mTargetOverrideNames[kTempColor2], 256, 256, sampleCount, colorFormat, 0, false);
	mTargets[kTempColor2] = NULL;
}

viewRenderOverrideMRT::~viewRenderOverrideMRT()
{
	for (unsigned int i = 0; i<kOperationCount; i++)
	{
		delete mRenderOperations[i];
		mRenderOperations[i] = NULL;
	}

	MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
	if (theRenderer)
	{
		// Release shaders
		const MHWRender::MShaderManager* shaderMgr = theRenderer->getShaderManager();
		for (unsigned int i = 0; i<kShaderCount; i++)
		{
			if (mShaderInstances[i])
			{
				if (shaderMgr)
					shaderMgr->releaseShader(mShaderInstances[i]);
				mShaderInstances[i] = NULL;
			}
		}

		// Release targets
		const MHWRender::MRenderTargetManager* targetManager = theRenderer->getRenderTargetManager();
		for (unsigned int i = 0; i<kTargetCount; i++)
		{
			if (mTargetDescriptions[i])
			{
				delete mTargetDescriptions[i];
				mTargetDescriptions[i] = NULL;
			}

			if (mTargets[i])
			{
				if (targetManager)
				{
					targetManager->releaseRenderTarget(mTargets[i]);
				}
				mTargets[i] = NULL;
			}
		}
	}
}

MHWRender::DrawAPI viewRenderOverrideMRT::supportedDrawAPIs() const
{
	return (MHWRender::kOpenGL | MHWRender::kDirectX11 | MHWRender::kOpenGLCoreProfile);
}

bool viewRenderOverrideMRT::startOperationIterator()
{
	mCurrentOperation = 0;
	return true;
}

MHWRender::MRenderOperation* viewRenderOverrideMRT::renderOperation()
{
	if (mCurrentOperation >= 0 && mCurrentOperation < kOperationCount)
	{
		// Skip empty and disabled operations
		//
		while (!mRenderOperations[mCurrentOperation] || !mRenderOperationEnabled[mCurrentOperation])
		{
			mCurrentOperation++;
			if (mCurrentOperation >= kOperationCount)
			{
				return NULL;
			}
		}

		if (mRenderOperations[mCurrentOperation])
		{
			return mRenderOperations[mCurrentOperation];
		}
	}
	return NULL;
}

bool viewRenderOverrideMRT::nextRenderOperation()
{
	mCurrentOperation++;
	if (mCurrentOperation < kOperationCount)
	{
		return true;
	}
	return false;
}

//
// Update list of operations to perform:
//
// 1. Clear 2 color targets, 1 depth target. Set different fill colors for each MRT target
// 2. Render the scene to 2 color targets.
// 3. Perform user operation on 2 targets
// 4. Preview the 2 targets as sub-regions in a 3rd target
// 5. Present 3rd target
//
// Operations before the preview can be enabled / disable to change
// what is shown by the preview operation.
//
MStatus viewRenderOverrideMRT::updateRenderOperations()
{
	bool initOperations = true;
	for (unsigned int i = 0; i<kOperationCount; i++)
	{
		if (mRenderOperations[i])
			initOperations = false;
	}

	if (initOperations)
	{
		mRenderOperationNames[kTargetClear] = "_viewRenderOverrideMRT_TargetClearMRT";
		quadRenderMRT * quadOp = new quadRenderMRT(mRenderOperationNames[kTargetClear]);
		mRenderOperations[kTargetClear] = quadOp;
		mRenderOperationEnabled[kTargetClear] = true;

		mRenderOperationNames[kMaya3dSceneRender] = "_viewRenderOverrideMRT_SceneRenderMRT";
		sceneRenderMRT * sceneOp = new sceneRenderMRT(mRenderOperationNames[kMaya3dSceneRender]);
		mRenderOperations[kMaya3dSceneRender] = sceneOp;
		mRenderOperationEnabled[kMaya3dSceneRender] = true;

		mRenderOperationNames[kUserOp] = "_viewRenderOverrideMRT_UserOpMRT";
		userOperationMRT * userOp = new userOperationMRT(mRenderOperationNames[kUserOp]);
		mRenderOperations[kUserOp] = userOp;
		mRenderOperationEnabled[kUserOp] = false;

		mRenderOperationNames[kTargetPreview] = "_viewRenderOverrideMRT_TargetPreview";
		quadRenderMRT * quadOp2 = new quadRenderMRT(mRenderOperationNames[kTargetPreview]);
		mRenderOperations[kTargetPreview] = quadOp2;
		mRenderOperationEnabled[kTargetPreview] = true;

		mRenderOperationNames[kPresentOp] = "_viewRenderOverrideMRT_PresentTargetMRT";
		mRenderOperations[kPresentOp] = new presentTargetMRT(mRenderOperationNames[kPresentOp]);
		mRenderOperationEnabled[kPresentOp] = true;
	}
	mCurrentOperation = -1;

	MStatus haveOperations = MStatus::kFailure;
	for (unsigned int i = 0; i<kOperationCount; i++)
	{
		if (mRenderOperations[i])
			haveOperations = MStatus::kSuccess;
	}
	return haveOperations;
}

//
// Update all targets used for rendering
//
MStatus viewRenderOverrideMRT::updateRenderTargets(MHWRender::MRenderer *theRenderer,
	const MHWRender::MRenderTargetManager *targetManager)
{
	if (!targetManager || !theRenderer)
		return MStatus::kFailure;

	// Get the current output target size
	unsigned int targetWidth = 256;
	unsigned int targetHeight = 256;
	theRenderer->outputTargetSize(targetWidth, targetHeight);

	// Create or resize the targets
	for (unsigned int targetId = 0; targetId < kTargetCount; ++targetId)
	{
		mTargetDescriptions[targetId]->setWidth(targetWidth);
		mTargetDescriptions[targetId]->setHeight(targetHeight);

		// Contrived test code to create incompatible target descriptions.
		if (targetId > 0)
		{
			mTargetDescriptions[targetId]->setWidth(100);
			if (!mTargetDescriptions[targetId]->compatibleWithDescription(*(mTargetDescriptions[0])))
				mTargetDescriptions[targetId]->setWidth(targetWidth);
		}
		if (!mTargets[targetId])
			mTargets[targetId] = targetManager->acquireRenderTarget(*(mTargetDescriptions[targetId]));
		else
			mTargets[targetId]->updateDescription(*(mTargetDescriptions[targetId]));
	}

	// We want to render to 2 targets for each of the following operations
	//
	quadRenderMRT * quadOp = (quadRenderMRT *)mRenderOperations[kTargetClear];
	if (quadOp)
		quadOp->setRenderTargets(mTargets, 2);
	sceneRenderMRT *sceneOp = (sceneRenderMRT *)mRenderOperations[kMaya3dSceneRender];
	if (sceneOp)
		sceneOp->setRenderTargets(mTargets);
	userOperationMRT * userOp = (userOperationMRT *)mRenderOperations[kUserOp];
	if (userOp)
		userOp->setRenderTargets(mTargets);

	// We NULL the output targets here so that the result will go into the
	// internal target and be presented from the internal targets
	quadRenderMRT * quadOp2 = (quadRenderMRT *)mRenderOperations[kTargetPreview];
	if (quadOp2)
		quadOp2->setRenderTargets(NULL, 0);
	presentTargetMRT *presentOp = (presentTargetMRT *)mRenderOperations[kPresentOp];
	if (presentOp)
		presentOp->setRenderTargets(NULL);

	return MStatus::kSuccess;
}

//
// Update all shaders used for rendering
//
MStatus viewRenderOverrideMRT::updateShaders(const MHWRender::MShaderManager* shaderMgr)
{
	// Set up a MRT clear shader
	//
	MHWRender::MShaderInstance *shaderInstance = mShaderInstances[kTargetClearShader];
	if (!shaderInstance)
	{
		shaderInstance = mShaderInstances[kTargetClearShader] = shaderMgr->getEffectsFileShader("mayaClearColorMRT", "");
		if (shaderInstance)
		{
			// Set constant parameters
			const float target1Color[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
			const float target2Color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
			shaderInstance->setParameter("gTargetColor1", target1Color);
			shaderInstance->setParameter("gTargetColor2", target2Color);
		}
	}
	// Update shader on quad operation
	quadRenderMRT * quadOp = (quadRenderMRT *)mRenderOperations[kTargetClear];
	if (quadOp)
		quadOp->setShader(mShaderInstances[kTargetClearShader]);

	// Set up a MRT geometry attribute shader
	MHWRender::MShaderInstance *shaderInstance1 = mShaderInstances[kSceneAttributeShader];
	if (!shaderInstance1)
	{
		shaderInstance1 = shaderMgr->getEffectsFileShader("mayaGeometryAttributeMRT", "");
		mShaderInstances[kSceneAttributeShader] = shaderInstance1;
	}
	// Update shader on scene operation
	sceneRenderMRT *sceneOp = (sceneRenderMRT *)mRenderOperations[kMaya3dSceneRender];
	if (sceneOp)
	{
		sceneOp->setShader(shaderInstance1);
	}

	// Set up a preview target shader (MRT as input)
	//
	MHWRender::MShaderInstance *shaderInstance2 = mShaderInstances[kTargetPreviewShader];
	if (!shaderInstance2)
	{
		shaderInstance2 = shaderMgr->getEffectsFileShader("FreeView", "");
		mShaderInstances[kTargetPreviewShader] = shaderInstance2;

		// Set constant parmaeters
		if (shaderInstance2)
		{
			const float borderClr[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
			const float backGroundClr[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
			shaderInstance2->setParameter("gBorderColor", borderClr);
			shaderInstance2->setParameter("gBackgroundColor", backGroundClr);
		}
	}
	// Update shader's per frame parameters
	if (shaderInstance2)
	{
		unsigned int targetWidth = 0;
		unsigned int targetHeight = 0;
		MHWRender::MRenderer *theRenderer = MHWRender::MRenderer::theRenderer();
		if (theRenderer)
			theRenderer->outputTargetSize(targetWidth, targetHeight);

		float vpSize[2] = { (float)targetWidth,  (float)targetHeight };
		shaderInstance2->setParameter("gViewportSizePixels", vpSize);

		float sourceSize[2] = { (float)targetWidth,  (float)targetHeight };
		shaderInstance2->setParameter("gSourceSizePixels", sourceSize);

		/// Could use 0.0125 * width / 2
		shaderInstance2->setParameter("gBorderSizePixels", 0.00625f * targetWidth);

		// Bind two input targets
		if (mTargets[0] && mTargets[1])
		{
			MHWRender::MRenderTargetAssignment assignment;
			assignment.target = mTargets[0];
			shaderInstance2->setParameter("gSourceTex", assignment);

			MHWRender::MRenderTargetAssignment assignment2;
			assignment2.target = mTargets[1];
			shaderInstance2->setParameter("gSourceTex2", assignment2);
		}
	}
	// Update shader on quad operation
	quadRenderMRT * quadOp2 = (quadRenderMRT *)mRenderOperations[kTargetPreview];
	if (quadOp2)
		quadOp2->setShader(mShaderInstances[kTargetPreviewShader]);

	if (quadOp && shaderInstance && quadOp2 && shaderInstance2)
		return MStatus::kSuccess;
	return MStatus::kFailure;
}

//
// Update override for the current frame
//
MStatus viewRenderOverrideMRT::setup(const MString& destination)
{
	// Firewall checks
	MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
	if (!renderer) return MStatus::kFailure;

	const MHWRender::MShaderManager* shaderMgr = renderer->getShaderManager();
	if (!shaderMgr) return MStatus::kFailure;

	const MHWRender::MRenderTargetManager *targetManager = renderer->getRenderTargetManager();
	if (!targetManager) return MStatus::kFailure;

	// Update render operations
	MStatus status = updateRenderOperations();
	if (status != MStatus::kSuccess)
		return status;

	// Update render targets
	status = updateRenderTargets(renderer, targetManager);
	if (status != MStatus::kSuccess)
		return status;

	// Update shaders
	status = updateShaders(shaderMgr);

	return status;
}

MStatus viewRenderOverrideMRT::cleanup()
{
	mCurrentOperation = -1;

	quadRenderMRT * quadOp = (quadRenderMRT *)mRenderOperations[kTargetClear];
	if (quadOp)
		quadOp->setRenderTargets(NULL, 0);

	sceneRenderMRT *sceneOp = (sceneRenderMRT *)mRenderOperations[kMaya3dSceneRender];
	if (sceneOp)
		sceneOp->setRenderTargets(NULL);

	userOperationMRT * userOp = (userOperationMRT *)mRenderOperations[kUserOp];
	if (userOp)
		userOp->setRenderTargets(NULL);

	quadRenderMRT * quadOp2 = (quadRenderMRT *)mRenderOperations[kTargetPreview];
	if (quadOp2)
		quadOp2->setRenderTargets(NULL, 0);

	presentTargetMRT *presentOp = (presentTargetMRT *)mRenderOperations[kPresentOp];
	if (presentOp)
		presentOp->setRenderTargets(NULL);

	return MStatus::kSuccess;
}

///////////////////////////////////////////////////////////////////

sceneRenderMRT::sceneRenderMRT(const MString& name)
	: MSceneRender(name)
{
	float val[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	mClearOperation.setClearColor(val);
	mTargets = NULL;
	mShaderInstance = NULL;
}

sceneRenderMRT ::~sceneRenderMRT()
{
	mTargets = NULL;
	mShaderInstance = NULL;
}

/* virtual */
MHWRender::MRenderTarget* const*
sceneRenderMRT::targetOverrideList(unsigned int &listSize)
{
	if (mTargets)
	{
		listSize = 2;
		return &mTargets[0];
	}
	return NULL;
}

/* virtual */
MHWRender::MClearOperation &
sceneRenderMRT::clearOperation()
{
	mClearOperation.setMask((unsigned int)
		(MHWRender::MClearOperation::kClearDepth | MHWRender::MClearOperation::kClearStencil));
	return mClearOperation;
}


/* virtual */
const MHWRender::MShaderInstance* sceneRenderMRT::shaderOverride()
{
	return mShaderInstance;
}

/* virtual */
// We only care about the opaque objects
MHWRender::MSceneRender::MSceneFilterOption sceneRenderMRT::renderFilterOverride()
{
	return MHWRender::MSceneRender::kRenderOpaqueShadedItems;
}

void
sceneRenderMRT::setRenderTargets(MHWRender::MRenderTarget **targets)
{
	mTargets = targets;
}

///////////////////////////////////////////////////////////////////

presentTargetMRT::presentTargetMRT(const MString& name)
	: MPresentTarget(name)
{
	mTargets = NULL;
}

presentTargetMRT ::~presentTargetMRT()
{
	mTargets = NULL;
}

/* virtual */
MHWRender::MRenderTarget* const*
presentTargetMRT::targetOverrideList(unsigned int &listSize)
{
	if (mTargets)
	{
		listSize = 2;
		return &mTargets[0];
	}
	listSize = 0;
	return NULL;
}

void
presentTargetMRT::setRenderTargets(MHWRender::MRenderTarget **targets)
{
	mTargets = targets;
}

///////////////////////////////////////////////////////////////////

quadRenderMRT::quadRenderMRT(const MString &name)
	: MQuadRender(name)
	, mShaderInstance(NULL)
{
	mOutputTargetCount = 0;
	mOutputTargets = NULL;
}

quadRenderMRT::~quadRenderMRT()
{
	mOutputTargetCount = 0;
	mOutputTargets = NULL;
	mShaderInstance = NULL;
}

const MHWRender::MShaderInstance *
quadRenderMRT::shader()
{
	return mShaderInstance;
}

MHWRender::MRenderTarget* const*
quadRenderMRT::targetOverrideList(unsigned int &listSize)
{
	if (mOutputTargets)
	{
		listSize = mOutputTargetCount;
		return &mOutputTargets[0];
	}
	listSize = 0;
	return NULL;
}

MHWRender::MClearOperation &
quadRenderMRT::clearOperation()
{
	mClearOperation.setMask((unsigned int)MHWRender::MClearOperation::kClearAll);
	return mClearOperation;
}

///////////////////////////////////////////////////////////////////

userOperationMRT::userOperationMRT(const MString &name)
	: MUserRenderOperation(name)
{
	mTargets = NULL;
}

userOperationMRT::~userOperationMRT()
{
	mTargets = NULL;
}

MStatus userOperationMRT::execute(const MHWRender::MDrawContext & drawContext)
{
	// Empty execute
	return MStatus::kSuccess;
}

MHWRender::MRenderTarget* const*
userOperationMRT::targetOverrideList(unsigned int &listSize)
{
	if (mTargets)
	{
		listSize = 2;
		return &mTargets[0];
	}
	return NULL;
}

