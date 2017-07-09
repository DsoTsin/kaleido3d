from generator import ProtypeLoader, TemplateGenerator

import sys
import argparse
parser = argparse.ArgumentParser(description='test parsing arguments')

parser.add_argument('input', nargs='*', help='input file path')
parser.add_argument('-o', help='specify output file path')
arg = parser.parse_args(sys.argv[1:])

protype = ProtypeLoader(arg.input[0])
protype_gen = TemplateGenerator()
protype_gen.gen(protype, arg.o)

# # generate vulkan
# vk_protype = ProtypeLoader('vulkan.yml')
# vk_protype_gen = TemplateGenerator()
# vk_protype_gen.gen(vk_protype, 'build/vulkan_impl.h')

# # generate direct3d12
# d3d12_protype = ProtypeLoader('direct3d12.yml')
# d3d12_protype_gen = TemplateGenerator()
# d3d12_protype_gen.gen(d3d12_protype, 'build/direct3d12_impl.h')

# # generate metal
# mtl_protype = ProtypeLoader('metal.yml')
# mtl_protype_gen = TemplateGenerator()
# mtl_protype_gen.gen(mtl_protype, 'build/metal_impl.h')