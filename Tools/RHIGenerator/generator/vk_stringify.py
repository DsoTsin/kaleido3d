import urllib2
from xml.etree import ElementTree

str_file = open('vulkan.cpp', 'w')

def print_node(node):
    if node.attrib.has_key("name") > 0:
        if node.attrib['name'] == 'VkFormat':
            str_file.write('const char * StrVkFormat(VkFormat format)\n{\n  switch(format)\n  {\n')
            for child in node.getchildren():
                print child.attrib['name']
                str_file.write('  case {0}: return \"{0}\";\n'.format(child.attrib['name']))
            str_file.write('  }\n}')
            
def read_xml(text):
    root = ElementTree.fromstring(text)  
      
    lst_node = root.getiterator("enums")  
    for node in lst_node:  
        print_node(node)

if __name__ == '__main__':
    response = urllib2.urlopen('https://raw.githubusercontent.com/KhronosGroup/Vulkan-Docs/1.0/src/spec/vk.xml')
    html = response.read()
    read_xml(html)  