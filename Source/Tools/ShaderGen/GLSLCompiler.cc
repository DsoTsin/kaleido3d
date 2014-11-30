#include "SPIRV/GlslangToSpv.h"
#include <vulkan/vulkan.h>

#include "GLSLCompiler.h"

namespace k3d {

	IShaderCompilerOutput* GLSLCompiler::Compile(ShaderCompilerOption const& option,  const char * source)
	{
		return glslToSpv(option.ShaderType, source);
	}

	void initResources(TBuiltInResource &resources)
	{
		resources.maxLights = 32;
		resources.maxClipPlanes = 6;
		resources.maxTextureUnits = 32;
		resources.maxTextureCoords = 32;
		resources.maxVertexAttribs = 64;
		resources.maxVertexUniformComponents = 4096;
		resources.maxVaryingFloats = 64;
		resources.maxVertexTextureImageUnits = 32;
		resources.maxCombinedTextureImageUnits = 80;
		resources.maxTextureImageUnits = 32;
		resources.maxFragmentUniformComponents = 4096;
		resources.maxDrawBuffers = 32;
		resources.maxVertexUniformVectors = 128;
		resources.maxVaryingVectors = 8;
		resources.maxFragmentUniformVectors = 16;
		resources.maxVertexOutputVectors = 16;
		resources.maxFragmentInputVectors = 15;
		resources.minProgramTexelOffset = -8;
		resources.maxProgramTexelOffset = 7;
		resources.maxClipDistances = 8;
		resources.maxComputeWorkGroupCountX = 65535;
		resources.maxComputeWorkGroupCountY = 65535;
		resources.maxComputeWorkGroupCountZ = 65535;
		resources.maxComputeWorkGroupSizeX = 1024;
		resources.maxComputeWorkGroupSizeY = 1024;
		resources.maxComputeWorkGroupSizeZ = 64;
		resources.maxComputeUniformComponents = 1024;
		resources.maxComputeTextureImageUnits = 16;
		resources.maxComputeImageUniforms = 8;
		resources.maxComputeAtomicCounters = 8;
		resources.maxComputeAtomicCounterBuffers = 1;
		resources.maxVaryingComponents = 60;
		resources.maxVertexOutputComponents = 64;
		resources.maxGeometryInputComponents = 64;
		resources.maxGeometryOutputComponents = 128;
		resources.maxFragmentInputComponents = 128;
		resources.maxImageUnits = 8;
		resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
		resources.maxCombinedShaderOutputResources = 8;
		resources.maxImageSamples = 0;
		resources.maxVertexImageUniforms = 0;
		resources.maxTessControlImageUniforms = 0;
		resources.maxTessEvaluationImageUniforms = 0;
		resources.maxGeometryImageUniforms = 0;
		resources.maxFragmentImageUniforms = 8;
		resources.maxCombinedImageUniforms = 8;
		resources.maxGeometryTextureImageUnits = 16;
		resources.maxGeometryOutputVertices = 256;
		resources.maxGeometryTotalOutputComponents = 1024;
		resources.maxGeometryUniformComponents = 1024;
		resources.maxGeometryVaryingComponents = 64;
		resources.maxTessControlInputComponents = 128;
		resources.maxTessControlOutputComponents = 128;
		resources.maxTessControlTextureImageUnits = 16;
		resources.maxTessControlUniformComponents = 1024;
		resources.maxTessControlTotalOutputComponents = 4096;
		resources.maxTessEvaluationInputComponents = 128;
		resources.maxTessEvaluationOutputComponents = 128;
		resources.maxTessEvaluationTextureImageUnits = 16;
		resources.maxTessEvaluationUniformComponents = 1024;
		resources.maxTessPatchComponents = 120;
		resources.maxPatchVertices = 32;
		resources.maxTessGenLevel = 64;
		resources.maxViewports = 16;
		resources.maxVertexAtomicCounters = 0;
		resources.maxTessControlAtomicCounters = 0;
		resources.maxTessEvaluationAtomicCounters = 0;
		resources.maxGeometryAtomicCounters = 0;
		resources.maxFragmentAtomicCounters = 8;
		resources.maxCombinedAtomicCounters = 8;
		resources.maxAtomicCounterBindings = 1;
		resources.maxVertexAtomicCounterBuffers = 0;
		resources.maxTessControlAtomicCounterBuffers = 0;
		resources.maxTessEvaluationAtomicCounterBuffers = 0;
		resources.maxGeometryAtomicCounterBuffers = 0;
		resources.maxFragmentAtomicCounterBuffers = 1;
		resources.maxCombinedAtomicCounterBuffers = 1;
		resources.maxAtomicCounterBufferSize = 16384;
		resources.maxTransformFeedbackBuffers = 4;
		resources.maxTransformFeedbackInterleavedComponents = 64;
		resources.maxCullDistances = 8;
		resources.maxCombinedClipAndCullDistances = 8;
		resources.maxSamples = 4;
		resources.limits.nonInductiveForLoops = 1;
		resources.limits.whileLoops = 1;
		resources.limits.doWhileLoops = 1;
		resources.limits.generalUniformIndexing = 1;
		resources.limits.generalAttributeMatrixVectorIndexing = 1;
		resources.limits.generalVaryingIndexing = 1;
		resources.limits.generalSamplerIndexing = 1;
		resources.limits.generalVariableIndexing = 1;
		resources.limits.generalConstantMatrixVectorIndexing = 1;
	}

	EShLanguage findLanguage(const rhi::EShaderType shader_type)
	{
		switch (shader_type) {
		case rhi::ES_Vertex:
			return EShLangVertex;

		case rhi::ES_Hull:
			return EShLangTessControl;

		case rhi::ES_Domain:
			return EShLangTessEvaluation;

		case rhi::ES_Geometry:
			return EShLangGeometry;

		case rhi::ES_Fragment:
			return EShLangFragment;

		case rhi::ES_Compute:
			return EShLangCompute;

		default:
			return EShLangVertex;
		}
	}

	GLSLOutput* glslToSpv(
		const rhi::EShaderType shader_type,
		const char *pshader)
	{
		glslang::TProgram& program = *new glslang::TProgram;
		const char *shaderStrings[1];
		TBuiltInResource Resources;
		initResources(Resources);

		// Enable SPIR-V and Vulkan rules when parsing GLSL
		EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

		EShLanguage stage = findLanguage(shader_type);
		glslang::TShader* shader = new glslang::TShader(stage);

		shaderStrings[0] = pshader;
		shader->setStrings(shaderStrings, 1);

		GLSLOutput * output = nullptr;
		if (!shader->parse(&Resources, 100, false, messages)) {
			puts(shader->getInfoLog());
			puts(shader->getInfoDebugLog());
			output = new GLSLOutput();
			output->m_Result = EShaderCompileResult::Fail;
			output->m_ErrorMsg = program.getInfoLog();
			return output;
		}
		program.addShader(shader);
		if (!program.link(messages)) {
			puts(program.getInfoLog());
			puts(program.getInfoDebugLog());
			output = new GLSLOutput();
			output->m_Result = EShaderCompileResult::Fail;
			output->m_ErrorMsg = program.getInfoLog();
			return output;
		}
		std::vector<unsigned int> spirv;
		glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);
		output = new GLSLOutput(std::move(spirv));
		output->m_Result = EShaderCompileResult::Success;
		return output;
	}


	static void sInitializeGlSlang()
	{
		static bool sGlSlangIntialized = false;
		if (!sGlSlangIntialized) {
			glslang::InitializeProcess();
			//sGlSlangIntialized = true;
		}
	}

	static void sFinializeGlSlang()
	{
		static bool sGlSlangFinalized = false;
		if (!sGlSlangFinalized) {
			glslang::FinalizeProcess();
			//sGlSlangFinalized = true;
		}
	}

	GLSLCompiler::GLSLCompiler()
	{
		sInitializeGlSlang();
	}

	GLSLCompiler::~GLSLCompiler()
	{
		sFinializeGlSlang();
	}
}
