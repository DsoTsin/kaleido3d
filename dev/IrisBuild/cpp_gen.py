import xige, os

cur_dir = os.path.dirname(__file__)
test_file = os.path.join(cur_dir, '../../../samples/igen/ngfx.interface.hpp')

gen = xige.InterfaceGenerator()

class EnumObj(object):
    def __init__(self, n):
        self._root = n
        self._index = 0
        self._enums = []

    def append(self, enum):
        self._enums.append(enum)

    def __str__(self):
        src = 'enum class ' + self._root.get_name() + ' : uint8_t {\n'
        is_mask = self._root.get_attrib('bitmask')
        for n in self._enums:
            if is_mask:
                src += '  ' + n.get_name() + ' = (1 << ' + str(self._index) + '),\n'
                self._index = self._index + 1
            else:
                src += '  ' + n.get_name() + ',\n'
        src += '};\n'
        return src

class StructObj(object):
    def __init__(self, struct):
        self._root = struct
        self._base = None
        self._members = []

    def set_base(self, base):
        self._base = base

    def append(self, member):
        self._members.append(member)

    def replace_type(self, type):
        if type == 'array':
            return 'Vec'
        return type

    def __str__(self):
        struct_stat = 'struct ' + self._root.get_name() + ' {\n'
        for sm in self._members:
            nt = sm.get_type()
            if nt:
                struct_stat += '  '
                stf = nt.get_type_flag()
                if stf & xige.NodeFlag.Template:
                    struct_stat += self.replace_type(nt.get_name())
                    num_targs = nt.num_template_params()
                    if num_targs > 0:
                        t0 = nt.template_param_at(0)
                        struct_stat += '<'
                        if t0:
                            t0_name = t0.get_name()
                            struct_stat += t0_name + '>'
                elif stf & xige.NodeFlag.Pointer:
                    if stf & xige.NodeFlag.Constant:
                        struct_stat += 'const '
                        struct_stat += nt.get_name()
                        struct_stat += '*'
                    else:
                        struct_stat += nt.get_name() + '*'
                else:
                    struct_stat += nt.get_name()
                struct_stat += ' '
                        
                struct_stat += sm.get_name()
                struct_stat += ';\n'
        struct_stat += '};\n'
        return struct_stat

class FunctionObj(object):
    def __init__(self, fn):
        self._params = []
        self._ret = None
        self._root = fn
    
    def append(self, param):
        self._params.append(param)

    def ret(self, ret):
        self._ret = ret

    def __str__(self):
        ret_name = self._ret.get_type().get_name()
        ret_flag = self._ret.get_type_flag()
        rname = ''
        if ret_flag & xige.NodeFlag.Pointer:
            if ret_flag & xige.NodeFlag.Constant:
                rname += 'const '
                rname += ret_name
                rname += ' * '
            else:
                rname = ret_name + ' * '
        else:
            rname = ret_name + ' '
        fn_name = self._root.get_name()
        params = []
        for param in self._params:
            p_type = param.get_type()
            ptf = p_type.get_type_flag()
            p_name = ''
            if ptf & xige.NodeFlag.Pointer:
                if ptf & xige.NodeFlag.Constant:
                    p_name += 'const '
                    p_name += p_type.get_name()
                    p_name += ' * '
                else:
                    p_name = p_type.get_name() + ' * '
            else:
                p_name = p_type.get_name() + ' '

            params.append(p_name + param.get_name())
        is_const = self._root.is_function_const()
        return rname + fn_name + '(' + ', '.join(params) + ')' + (' const' if is_const else '')

class InterfaceObj(object):
    def __init__(self, int_node):
        self._root = int_node
        self._funcs = []
        self._is_forward_decl = int_node.is_forward_decl()

    def append(self, fn):
        self._funcs.append(fn)

    def __str__(self):
        int_stat = 'struct '
        int_stat += self._root.get_name()
        if not self._is_forward_decl:
            bn = self._root.get_base_type()
            if bn:
                int_stat += ' : public '
                int_stat += bn.get_name()
            elif self._root.get_attrib('refcount'):
                int_stat += ' : public Rc'
            int_stat += ' {\n'
            for method in self._funcs:
                int_stat += '  virtual ' + str(method) + ' = 0;\n'
            int_stat += '}'
        int_stat += ';\n'  
        return int_stat

class CxxGenerator(object):
    def __init__(self):
        self.stack = []
        self._source = ''
        self._funcs = []
        self._structs = []
        self._interfaces = []
        self._enums = []

        self._enum_objs = []
        self._struct_objs = []
        self._interface_objs = []
        
        self._headers = []
        self._impl_header = ''
        self._impl_source = ''

    def add_header(self, header):
        self._headers.append(header)

    def on_begin(self, node, type):
        self.stack.append(type)
        if type == xige.NodeType.Namespace:
            self._source += 'namespace '
            self._source += node.get_name()
            self._source += ' {\n'
        elif type == xige.NodeType.Enum:
            self._enums.append(EnumObj(node))
        elif type == xige.NodeType.EnumValue:
            self._enums[-1].append(node)
        elif type == xige.NodeType.Struct:
            new_struct = StructObj(node)
            bn = node.get_base_type()
            if bn:
                new_struct.set_base(bn)
            self._structs.append(new_struct)
        elif type == xige.NodeType.StructMember:
            self._structs[-1].append(node)
        elif type == xige.NodeType.Interface:
            self._interfaces.append(InterfaceObj(node))
        elif type == xige.NodeType.Function:
            self._funcs.append(FunctionObj(node))
        elif type == xige.NodeType.FunctionParam:
            self._funcs[-1].append(node)
        elif type == xige.NodeType.FunctionRet:
            self._funcs[-1].ret(node)

    def on_end(self, node):
        t = self.stack.pop()
        if t == xige.NodeType.Namespace:
            self._source += '}\n'
        elif t == xige.NodeType.Struct:
            struct = self._structs.pop()
            self._source += str(struct)
        elif t == xige.NodeType.Enum:
            enum = self._enums.pop()
            self._source += str(enum)
        elif t == xige.NodeType.Function:
            fn = self._funcs.pop()
            self._interfaces[-1].append(fn)
        elif t == xige.NodeType.Interface:
            interface = self._interfaces.pop()
            self._source += str(interface)

    def get_source(self):
        hdr_src = '#ifndef __ngfx_h__\n#define __ngfx_h__\n'
        for hdr in self._headers:
            hdr_src += '#include <' + hdr + '>\n'
        return hdr_src + self._source + '\n#endif'

    def gen_impl_header(self, backend):
        return ''

    def gen_impl_source(self, backend):
        return ''


import argparse, sys
parser = argparse.ArgumentParser(description='test parsing arguments')
parser.add_argument('input', nargs='*', help='input file path')
parser.add_argument('-o', help='specify output file path')
arg = parser.parse_args(sys.argv[1:])

# create loader and generator
cxx_gen = CxxGenerator()
gen.add_intrinsic('RefCount')
cxx_gen.add_header('ngfx_atomic.h')
cxx_gen.add_header('ngfx_ptr.h')
cxx_gen.add_header('ngfx_allocator.h')
cxx_gen.add_header('ngfx_container.h')
gen.set_generator(cxx_gen)
gen.load(arg.input[0]) # load and generate

with open(arg.o, 'w') as f:
    f.write(cxx_gen.get_source())
