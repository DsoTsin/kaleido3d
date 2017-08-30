from ctypes import *
import os
cur_dir = os.path.dirname(__file__)

class NodeType:
    Global = 0
    Namespace = 1
    Enum = 2
    EnumValue = 3
    Struct = 4
    StructMember = 5
    Function = 6
    FunctionRet = 7
    FunctionParam = 8
    Interface = 9

class NodeFlag:
    Invalid = 0xff00
    _None = 0
    Pointer = 1
    PointerOfAddress = 2
    Reference = 4
    Constant = 8
    Template = 16

xige = cdll.LoadLibrary(os.path.join(cur_dir, 'xige.dll'))

node_begin = CFUNCTYPE(None, c_void_p, c_int, c_void_p)
node_end = CFUNCTYPE(None, c_void_p, c_void_p)
node_err = CFUNCTYPE(None, c_char_p, c_void_p)

class Node(object):
    def __init__(self, node, dll):
        self._node = node
        self._dll = dll

    def get_type(self):
        rt = self._dll.node_get_type(self._node)
        return Node(rt, self._dll) if rt > 0 else None

    def get_name(self):
        cname = self._dll.node_get_name(self._node)
        name = c_char_p(cname).value
        return name

    def get_attrib(self, name):
        cval = self._dll.node_get_attribute(self._node, name)
        return cval

    def get_type_flag(self):
        return self._dll.node_get_type_flag(self._node)

    def is_forward_decl(self):
        return self._dll.node_is_forward_decl(self._node)
    
    def get_base_type(self):
        base = self._dll.node_get_base_type(self._node)
        return Node(base, self._dll) if base > 0 else None

    def is_function_const(self):
        return self._dll.node_is_function_const(self._node)

    def num_template_params(self):
        return self._dll.node_get_template_param_count(self._node)

    def template_param_at(self, index):
        n = self._dll.node_get_template_param(self._node, index)
        return Node(n, self._dll) if n > 0 else None

class InterfaceGenerator(object):
    
    def __init__(self):
        self.stack = []
        self.source = ''
        self.dll = cdll.LoadLibrary(os.path.join(cur_dir, 'xige.dll'))
        self.generator = None

    def set_generator(self, gen):
        self.generator = gen

    def add_intrinsic(self, intrin):
        self.dll.add_instrinsic(intrin)

    def load(self, path):
        self.dll.node_load_callback(path, 
            node_begin(InterfaceGenerator.begin_fn), 
            node_end(InterfaceGenerator.end_fn),
            node_err(InterfaceGenerator.on_error),
            py_object(self))

    def on_begin(self, node, type):
        pNode = Node(node, self.dll)
        self.generator.on_begin(pNode, type)

    def on_end(self, node):
        pNode = Node(node, self.dll)
        self.generator.on_end(pNode)

    @staticmethod
    def on_error(msg, p_arg):
        print c_char_p(msg).value

    @staticmethod
    def begin_fn(p_node, c_int, p_arg):
        gen = cast(p_arg, py_object).value
        gen.on_begin(p_node, c_int)

    @staticmethod
    def end_fn(p_node, p_arg):
        gen = cast(p_arg, py_object).value
        gen.on_end(p_node)
