#include "k3dShader.h"

ShaderOptions ShaderOptions::CreateVSOptions()
{
  ShaderOptions options;
  options.AddDefine<int>("VERTEX_SHADER", 1);
//  options.AddDefine<int>("ATTR_NORMAL",	semantic::Normal);
//  options.AddDefine<int>("ATTR_TEXCOORD",	semantic::TexCoord);
//  options.AddDefine<int>("ATTR_TANGENT",	semantic::Tangent);
//  options.AddDefine<int>("ATTR_COLOR",	semantic::Color);
//  options.AddDefine<int>("ATTR_BITANGENT",semantic::Bitangent);
  return options;
}

ShaderOptions ShaderOptions::CreateFSOptions(int w, int)
{
  ShaderOptions options;
  options.AddDefine<int>("FRAGMENT_SHADER", 1);
  return options;
}

void ShaderOptions::AddResolution(const std::string &name, int resX, int resY)
{
  std::stringstream option;
  option << "#define " << name << "_X " << resX << std::endl;
  option << "#define " << name << "_Y " << resY << std::endl;
  option << "#define RCP_" << name << "_X " << 1.f/float(resX) << std::endl;
  option << "#define RCP_" << name << "_Y " << 1.f/float(resY) << std::endl;
  options.push_back(option.str());
}

void ShaderOptions::AddConst(const std::string &name, int value)
{
  std::stringstream option;
  option << "const int " << name << " = " << value << ";";
  options.push_back(option.str());
}

void ShaderOptions::AddConst(const std::string &name, float value)
{
  std::stringstream option;
  option << "const float " << name << " = " << value << ";";
  options.push_back(option.str());
}

void ShaderOptions::AddConst(const std::string &name, const kMath::Vec2i &value)
{
  std::stringstream option;
  option << "const ivec2 " << name << " = ivec2(" << value[0] << "," << value[1] << ");";
  options.push_back(option.str());
}

void ShaderOptions::Include(const std::string &_string)
{
}

std::string ShaderOptions::ToString() const
{
  std::stringstream out;
  for(unsigned int i=0;i<options.size();++i)
      out << options[i] << std::endl;
  return out.str();
}

std::string ShaderOptions::Append(const std::string &source) const
{
//  std::vector<std::string> lines;
//  Split(_input, '\n',lines);

//  // Look for #version
//  std::string version		= "#version";
//  unsigned int lineIndex	= 0;
//  bool versionFound		= false;
//  while(lineIndex<lines.size() && !versionFound)
//  {
//      versionFound = lines[lineIndex].compare(0,8,version,0,8) == 0;
//      if(!versionFound) ++lineIndex;
//  }
//  assert(versionFound);

//  // Merge output lines
  std::stringstream output;
//  for(unsigned int i=0;i<=lineIndex;++i)
//      output << lines[i] << std::endl;
//  output << ToString();
//  for(unsigned int i=lineIndex+1;i<lines.size();++i)
//      output << lines[i] << std::endl;

  return output.str();
}
