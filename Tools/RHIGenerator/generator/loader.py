from yaml import load
#import re

def delegate(obj_func):
    def del_func(obj, path):
        obj_func(obj, path)
        #print obj.__namespace__, path
    return del_func


class ProtypeLoader(object):
    'a protype loader'
    @delegate
    def __init__(self, path):
        self.parent = []
        self.__namespace__      = None
        self.__type_dict__      = {}
        self.__enumlist__       = []
        self.__structlist__     = []
        self.__interfacelist__  = []
        self.__classlist__      = []
        self.__functionlist__   = []
        self.__imports__        = []
        self.__type_tree__		= {}
        self.__cpp__            = False
        with open(path) as yml:
            self.__contents__ = load(yml.read())
            yml.close()

        if 'import' in self.__contents__:
            self.__imports__ = self.__contents__['import']

        ns = self.__contents__['namespace']
        self.__namespace__ = ns
        if 'cpp' in self.__contents__:
            self.__cpp__       = self.__contents__['cpp']
            
        self.__type_tree__[ns] = {}
        self.__type_tree__[ns]['enum'] = {}
        self.__type_tree__[ns]['struct'] = {}
        self.__type_tree__[ns]['interface'] = {}
        self.__type_tree__[ns]['class'] = {}
        self.__type_tree__[ns]['function'] = {}

        for element in self.__contents__['protype']:
            for (key, val) in element.items():
                self.__type_dict__[key] = val['type']
                if val['type'] == 'enum':
                    self.__enumlist__.append({
                        'enum': key,
                        'val': val
                    })
                    self.__type_tree__[ns]['enum'][key] = val
                elif val['type'] == 'function':
                    self.__functionlist__.append({
                        'function': key,
                        'val': val
                    })
                    self.__type_tree__[ns]['function'][key] = val
                elif val['type'] == 'interface':
                    self.__interfacelist__.append({
                        'interface': key,
                        'val':val
                    })
                    self.__type_tree__[ns]['interface'][key] = val
                elif val['type'] == 'struct':
                    self.__structlist__.append({
                        'struct': key,
                        'val':val
                    })
                    self.__type_tree__[ns]['struct'][key] = val
                elif val['type'] == 'class':
                    self.__classlist__.append({
                        'class': key,
                        'val':val
                    })
                    self.__type_tree__[ns]['class'][key] = val

    @property
    def enums(self):
        return self.__enumlist__
    
    @property
    def structs(self):
        return self.__structlist__
    
    @property
    def functions(self):
        return self.__functionlist__
    
    @property
    def interfaces(self):
        return self.__interfacelist__
    
    @property
    def namespace(self):
        return self.__namespace__

    @property
    def type_tree(self):
    	return self.__type_tree__

    @property
    def classes(self):
        return self.__classlist__

    @property
    def imports(self):
        return self.__imports__

    @namespace.setter
    def namespace(self, val):
        if isinstance(val, str):
            self.__namespace__ = val
        else:
            print("Error")