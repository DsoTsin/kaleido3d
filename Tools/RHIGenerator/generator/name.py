import re

def make_name_AxxBxx(name):
    parts = name.split(' ')
    mp = []
    if parts:
        for p in parts:
            a_p = p[0:1].upper()
            b_p = p[1:]
            mp.append(a_p + b_p)
    else:
        a_p = name[0:1].upper()
        b_p = name[1:]
        mp.append(a_p + b_p)
    return ''.join(mp)

def make_name_axxBxx(name):
    parts = name.split(' ')
    mp = []
    if parts:
        first_sec = parts[0]
        a_p = first_sec[0:1].lower()
        b_p = first_sec[1:]
        mp.append(a_p + b_p)
        for p in parts[1:]:
            a_p = p[0:1].upper()
            b_p = p[1:]
            mp.append(a_p + b_p)
    else:
        a_p = name[0:1].lower()
        b_p = name[1:]
        mp.append(a_p + b_p)
    return ''.join(mp)

def make_name_XXX_XXX(name):
    parts = name.split(' ')
    mp = []
    if parts:
        for p in parts:
            mp.append(p.upper())
    else:
        mp.append(name.upper())
    return '_'.join(mp)

def make_enum_name(prefix, name):
    return prefix + make_name_AxxBxx(name)

def make_interface_name(prefix, name):
    if prefix != '':
        return prefix + make_name_AxxBxx(name)
    else:
        return make_name_AxxBxx(name)
        
def make_class_name(prefix, name):
    if prefix != '':
        return prefix.upper() + make_name_AxxBxx(name)
    else:
        return make_name_AxxBxx(name)

def make_struct_name(prefix, name):
    return prefix + make_name_AxxBxx(name)

def make_enum_value(prefix, name):
    if prefix:
        return '_'.join([prefix.upper(), make_name_XXX_XXX(name)])
    else:
        return make_name_XXX_XXX(name)

def make_cpp_enum_name(prefix, name):
    return make_name_AxxBxx(name)

def make_cpp_struct_name(prefix, name):
    return make_name_AxxBxx(name)

def make_cpp_interface_name(prefix, name):
    if prefix != '':
        return make_name_AxxBxx(name)
    else:
        return make_name_AxxBxx(name)

NameUtil = {
    'enum': make_enum_name,
    'struct': make_struct_name,
    'interface': make_interface_name,
    'function': make_struct_name,
    'class': make_class_name,
    'cpp_enum': make_cpp_enum_name,
    'cpp_struct': make_cpp_struct_name,
    'cpp_interface': make_cpp_interface_name,
    'cpp_function': make_struct_name,
}

# type regex
TypeRegEx = re.compile(r'\s*(?P<const>(const\s+)?)(?P<type>[\w\s]*\w)(?P<pointer>[\*\s]*)', re.I)