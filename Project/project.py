# -*- coding: utf-8 -*-

"""
Platform Specifies
"""
import platform
osbits, pos = platform.architecture()
osplat = platform.system()

print osbits, osplat

"""
Project
"""
solution_name = "k3dvs2013"
project_name = ['kaleido3d']
project_dir = 'E:/project/kaleido3d_g4'

file_filters = {"*.h", "*.cc", "*.cpp", "**.hpp"}
print file_filters

include_dirs = {project_dir+'/Include', project_dir+'/Source', project_dir+'/3rdParty/JsonCpp/include'}
print include_dirs

import os, uuid
from xml.dom import minidom

def createTextNode(dom, node_name, text):
    node = dom.createElement(node_name)
    txtnode = dom.createTextNode(text)
    node.appendChild(txtnode)
    return node

def createVSPrjConf(vs_dom, config, node):
    for _config in config:
        proj_config = vs_dom.createElement('ProjectConfigurations')
        proj_config.setAttribute('Include', _config)
        
        if _config.find('Debug') & _config.find('Win32'):
            ci = createTextNode(vs_dom, 'Configuration', 'Debug')
            cp = createTextNode(vs_dom, 'Platform', 'Win32')
            proj_config.appendChild(ci)
            proj_config.appendChild(cp)
            
        elif _config.find('Debug') & _config.find('x64'):
            ci = createTextNode(vs_dom, 'Configuration', 'Debug')
            cp = createTextNode(vs_dom, 'Platform', 'x64')
            proj_config.appendChild(ci)
            proj_config.appendChild(cp)
        elif _config.find('Release') & _config.find('x64'):
            ci = createTextNode(vs_dom, 'Configuration', 'Release')
            cp = createTextNode(vs_dom, 'Platform', 'x64')
            proj_config.appendChild(ci)
            proj_config.appendChild(cp)
        else :
            ci = createTextNode(vs_dom, 'Configuration', 'Release')
            cp = createTextNode(vs_dom, 'Platform', 'Win32')
            proj_config.appendChild(ci)
            proj_config.appendChild(cp)
            
        node.appendChild(proj_config)
        
def addCompileSources(vs_dom, item_group, sources):
    for source in sources:
        src = vs_dom.createElement('ClCompile')
        src.setAttribute('Include', source)
        item_group.appendChild(src)

def addPropertyGroupConditionConfs(vs_dom, node, configs):
    for conf in configs:
        pgc = vs_dom.createElement('PropertyGroup')
        pgc.setAttribute('Condition','\'$(Configuration)|$(Platform)\'==\''+conf+'\'')
        pgc.setAttribute('Label', 'Configuration')
        
        cf = createTextNode(vs_dom, 'ConfigurationType', 'Application')
        pt = createTextNode(vs_dom, 'PlatformToolSet', 'v120')
        cs = createTextNode(vs_dom, 'CharacterSet', 'Unicode')
        
        pgc.appendChild(cf)
        pgc.appendChild(pt)
        pgc.appendChild(cs)
        
        node.appendChild(pgc)

def createItemDefinitionGroup(vs_dom, config):
    node = vs_dom.createElement('ItemDefinitionGroup')
    node.setAttribute('Condition', '\'$(Configuration)|$(Platform)\'==\''+config+'\'')
    
    clc = vs_dom.createElement('ClCompile')
    wl = createTextNode(vs_dom, 'WarningLevel', 'Level3')
    op = createTextNode(vs_dom, 'Optimization', 'Disabled')
    pp = createTextNode(vs_dom, 'PreprocessorDefinitions', '%(PreprocessorDefinitions)')
    clc.appendChild(wl)
    clc.appendChild(op)
    clc.appendChild(pp)
    
    node.appendChild(clc)
    
    link = vs_dom.createElement('Link')
    subs = createTextNode(vs_dom, 'SubSystem', 'Console')
    link.appendChild(subs)
    
    node.appendChild(link)
    
    return node
    
class Project:
    def __init__(self, proj_name, proj_kind):
        self.projName = proj_name
        self.kind = proj_kind
        self.includeDirs = []
        self.libDirs = []
        self.libs = []
        self.defines = []
        self.sources = []
        self.pch = []
        guid = str(uuid.uuid1())
        self.uuid = '{'+guid.upper()+'}'
        
    def addSourceFile(self, filepath):
        self.sources.append(filepath)
        
    def addDefine(self, preprocessor):
        self.defines.append(preprocessor)
        
    def addIncludePath(self, inc):
        self.includeDirs.append(inc)
    
    def addLibPath(self, libpath):
        self.libDirs.append(libpath)
        
    def addLib(self, libname):
        self.libDirs.append(libname)
        
    def generateVS2013ProjectFile(self):
        prj_file = open(self.projName+'.vcxproj', 'w')    
        vs_xml = minidom.getDOMImplementation()
        vs_dom = vs_xml.createDocument(None, None, None)
        root = vs_dom.createElement("Project")
        root.setAttribute('DefaultTarget', 'Build')
        root.setAttribute('ToolsVersion', '12.0')
        root.setAttribute('xmlns', 'http://schemas.microsoft.com/developer/msbuild/2003')
        
        label = vs_dom.createElement('ItemGroup')
        label.setAttribute('Label','ProjectConfigurations')
        # CreatePrjConfs
        config = ['Debug|Win32', 'Release|Win32', 'Debug|x64', 'Release|x64']
        createVSPrjConf(vs_dom, config, label)
        root.appendChild(label)
        # Create PropertyGroups
        property_group = vs_dom.createElement('PropertyGroup')
        property_group.setAttribute('Label', 'Globals')        
        proj_guid = vs_dom.createElement('ProjectGuid')
        guid = vs_dom.createTextNode(self.uuid) 
        proj_guid.appendChild(guid)        
        root_ns = vs_dom.createElement('RootNamespace')        
        ns = vs_dom.createTextNode(self.projName)
        root_ns.appendChild(ns)        
        property_group.appendChild(proj_guid)
        property_group.appendChild(root_ns)
        root.appendChild(property_group)        
        
        import_proj = vs_dom.createElement('ItemGroup')
        import_proj.setAttribute('Project','$(VCTargetsPath)\\Microsoft.Cpp.Default.props')
        root.appendChild(import_proj)
        
        addPropertyGroupConditionConfs(vs_dom, root, config)        
        
        for _conf in config:
            idg = createItemDefinitionGroup(vs_dom, _conf)
            root.appendChild(idg)
        
        source_item_group = vs_dom.createElement('ItemGroup')
        addCompileSources(vs_dom, source_item_group, self.sources)
        root.appendChild(source_item_group)        
        
        vs_dom.appendChild( root )
        vs_dom.writexml(prj_file,'',' ','\n','utf-8')
        prj_file.close()



def fun(path):
    for root, dirs, files in os.walk(path):
        for fn in files:
            print root, fn
            
def generateQtProFiles( project_file ):
    print project_file
    
def generateVS2013Solution():
    solution_file = open(solution_name+'.sln', 'w')
    solution_file.writelines(['Microsoft Visual Studio Solution File, Format Version 12.00\n','# Visual Studio 2013\n'])
    solution_file.writelines(['VisualStudioVersion = 12.0.21005.1\n','MinimumVisualStudioVersion = 10.0.40219.1\n'])
    
    solution_file.close()
    
    
if osplat == 'Windows':
    print 'Generating Visual Studio 2013 Solution...'

    pro = Project('k3d','Console')
    pro.generateVS2013ProjectFile()