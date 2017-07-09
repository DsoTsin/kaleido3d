#include <Kaleido3D.h>
#include "vulkan_glslang.h"
#include <Core/Os.h>
#include <iostream>
#include <unordered_map>

using namespace std;
using namespace ngfx;

#if _WIN32
#pragma comment(linker,"/subsystem:console")
#endif

class CommandUtil
{
public:
  CommandUtil(int argc, const char** argv)
  {
    MainArg = argv[0];
    int sId = 1;
    while (sId < argc)
    {
      if (std::string("-o") == (argv[sId]))
      {
        sId++;
        ArgValMap["-o"] = argv[sId];
      }
      else if (std::string("-c") == (argv[sId]))
      {
      }
      else if (std::string("-I") == (argv[sId]))
      {
        sId++;
        ArgValMap["-I"] = argv[sId];
      }
      else
      {
        ArgValMap["-c"] = argv[sId];
      }
      sId++;
    }
    if (ArgValMap.find("-o") == ArgValMap.end())
    {
      cerr << "Error: -o option missing" << endl;
      PrintUsage();
    }
    if (ArgValMap.find("-c") == ArgValMap.end())
    {
      cerr << "Error: -c option missing" << endl;
      PrintUsage();
    }
  }
  
  std::string GetArg(std::string const& name) const
  {
    return ArgValMap.at(name);
  }

  void PrintUsage()
  {
    cout << "usage: vulkanc -o [output file] -c [input source]" << endl;
  }

  bool Valid() { return !ArgValMap.empty(); }

private:
  std::string MainArg;
  std::unordered_map<std::string, std::string> ArgValMap;
};

/*
 * vulkanc -c [[ -o [output file] ]] {shader files}
 */
int main(int argc, const char* argv[])
{
  CommandUtil Util(argc, argv);
  if (!Util.Valid())
    return -4;

  FunctionMap DataBlob;
  std::string Error;

  Os::File SrcFile;
  if(!SrcFile.Open(Util.GetArg("-c").c_str(), IORead))
  {
    cerr << "Unable To Open File: " << Util.GetArg("-c").c_str() << endl;
    return -1;
  }
  uint64 SzFile = SrcFile.GetSize();
  char* SrcData = new char[SzFile + 1];
  SrcFile.Read(SrcData, SzFile);
  SrcData[SzFile] = 0;
  CompileFromSource(ngfx::CompileOption(), SrcData, DataBlob, Error);
  delete SrcData;
  if (!Error.empty())
  {
    cerr << Error << endl;
    return -2;
  }
  SerializeLibrary(DataBlob, Util.GetArg("-o").c_str());
  return 0;
}