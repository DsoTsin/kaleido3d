#pragma once

#include <ngfx.h>
#include <ngfx_shader.h>
#include <ngfx_shader_compiler.h>
#include <string>

namespace shaderlab
{
    using String = std::string;

    class MaterialProperty
    {
    public:
        enum Type {
            MPT_FLOAT_CONST,
            MPT_INT_CONST,
            MPT_VECTOR4,
            MPT_TEXTURE,
        };
        MaterialProperty() {}
        virtual ~MaterialProperty() {}

        virtual const String& GetName() const { return m_Name; }
        virtual const String& GetDisplayName() const { return m_DisplayName; }
        virtual const String& GetAnnotation() const { return m_Annotation; }

        virtual Type GetType() const = 0;
        virtual void Serialize() {}

        friend class Parser;
        friend class PropertyNode;
        
    protected:
        String  m_Name;
        String  m_DisplayName;
        String  m_Annotation;
    };

    class MaterialPropertyInt : public MaterialProperty
    {
    public:
        MaterialPropertyInt(int Value = 0);
        ~MaterialPropertyInt() override;
        Type GetType() const override { return Type::MPT_INT_CONST; }

        void SetRange(int Min, int Max) {
            m_Min = Min;
            m_Max = Max;
        }
        void SetValue(int Val) {
            m_Int = Val;
        }
    private:
        int m_Int;
        int m_Min;
        int m_Max;
    };

    class MaterialPropertyFloat : public MaterialProperty
    {
    public:
        MaterialPropertyFloat(float Value = 0.0f);
        ~MaterialPropertyFloat() override;
        
        Type GetType() const override { return Type::MPT_FLOAT_CONST; }

        void SetRange(double Min, double Max) {
            m_Min = Min;
            m_Max = Max;
        }
        void SetValue(double Val) {
            m_Float = Val;
        }
    private:
        double m_Float;
        double m_Min;
        double m_Max;
    };

    class MaterialPropertyVector4 : public MaterialProperty
    {
    public:
        MaterialPropertyVector4();
        ~MaterialPropertyVector4() override;
        Type GetType() const override { return Type::MPT_VECTOR4; }
        void SetValue(float X, float Y, float Z, float W) {
            m_X = X;
            m_Y = Y;
            m_Z = Z;
            m_W = W;
        }
    private:
        float m_X, m_Y, m_Z, m_W;
    };

    class MaterialPropertyTexture : public MaterialProperty
    {
    public:
        MaterialPropertyTexture();
        ~MaterialPropertyTexture() override;
        Type GetType() const override { return Type::MPT_TEXTURE; }
        void SetTextureName(String const& Name) {
            m_TextureName = Name;
        }
    private:
        String m_TextureName;
    };

    // ShaderLab => MetaShader
    class MetaShader;
    // MetaShader => Permutate => ShaderSnippets
    class ShaderSnippet;

    class MaterialPass;
    /*
     * A SLMaterial may have several materail shaders target different passes or definition options
     * EditMode:
     *      Material -> Real Shader String -> Shader Byte Code
     * RuntimeMode
     *      Material -> Hash -> Shader Byte Code
     */
    class Material
    {
    public:

        Material();
        virtual ~Material();

        bool IsKeywordToggled(const char* Keyword) const;
        void ToggleKeyword(const char* Keyword, bool On);

        bool IsFeatureEnabled(const char* Name) const;
        void SetFeatureEnable(const char* Name, bool bEnable);

        void SetVector(const char* Name/*, Vector4 const& Value*/);
        void SetFloat(const char* Name, float Value);
        void SetInt(const char* Name, int Value);
        void SetTexture(const char* Name/*, Texture const& TexUnit*/);

        void SetPass(int Index);
        int       GetPassCount() const;

        void SetLOD(int Lod);

        struct RenderState
        {
            ngfx::BlendState BS;
            ngfx::DepthStencilState DS;
            ngfx::RasterizerState RS;
        };

        bool Compile();

        bool IsCompiled() const;

    public:
        static Material*    Parse(String const& SLCode, String* error = nullptr);
        static Material*    ParseFile(const char* path, String* error = nullptr);
        static Material*    ParseAndCompileFile(const char* path, String* error = nullptr);
        static void         SetIncludeDirectories(ngfx::Vec<String> const& includeDir);

    protected:
        void CacheForRendering();
        void CacheForCooking();

    private:
        static ngfx::Vec<String>        s_IncludeDirectories;

        friend class ShaderNode;
        friend class ShaderIncluder;
    private:
        String                          m_Name; // Shader Name
        int                             m_ActivePass;
        ngfx::Backend                   m_GfxBackend;
        bool                            m_IsInRuntime; // Editor/Run Time
        String                          m_IncludeSource; // HLSLINCLUDE
        ngfx::VecUPtr<MaterialPass>     m_Passes; // CompileTime
        ngfx::VecUPtr<MaterialProperty> m_Props; // Properties
    };
}