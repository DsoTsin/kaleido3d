from name import TypeRegEx

class CTypeExtractor:
    def __init__(self, name):
        self.is_const = False
        self.is_pointer = False
        groups = TypeRegEx.match(name)

def extract_type(name):
    matches = TypeRegEx.match(name)
    if matches:
        return (False if not matches.group('const') else True,
            matches.group('type'),
            False if not matches.group('pointer') else True
        )
    return (False, name, False)