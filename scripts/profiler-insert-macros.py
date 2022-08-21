#!/usr/bin/env python3
import sys
import os
import re

func_re = re.compile(
    (
        r'\b\w+(?:[\*&:<>]+)?[\n\t ]{1,}'                               # Require return type
        r'(?:(\w+)white::white)?'                                       # Optional class name
        r'(\w+)white\((?:[\*&:<>/\w\n\t,= ]+)?\)white(?:const)?white\{' # Required function name
        r'(?:whitePROFILEwhite\(white(\w+)white,white(\w+)white\))?'    # Optional PROFILE()
    ).replace('white', r'(?:[\n\t ]+)?')                                # optional white spaces
)

def process(file_name, contents):
    pos = 0
    while True:
        match = func_re.search(contents, pos)
        if match is None:
            break
        pos = match.end()
        if match.group(1) == match.group(3) and match.group(2) == match.group(4):
            continue

        class_name = match.group(1)
        func_name = match.group(2)

        if class_name is None:
            class_name = file_name + 'Global'

        # PROFILE() is there, but names need updating?
        if not match.group(3) is None:
            contents = contents[:match.start(3)] + class_name + ', ' + func_name + contents[match.end(4):]
            continue

        # NOPROFILE is specified?
        if -1 < contents[match.end():].find('NOPROFILE') < 20:
            continue

        contents = contents[:match.end()] +\
            '\n    PROFILE({}, {});\n'.format(class_name, func_name) +\
            contents[match.end():]

    return contents

def build_files_list(directory, lst, allowed_extensions):
    for dir_path, sub_dir_list, file_list in os.walk(directory):
        for file_name in file_list:
            if not any(file_name.endswith(x) for x in allowed_extensions):
                continue
            lst.append(os.path.join(dir_path, file_name))

def process_files(directory):
    allowed_extensions = ('.cpp',)
    files_to_process = list()
    build_files_list(directory, files_to_process, allowed_extensions)

    for file_name in files_to_process:
        file_name_ext, ext = os.path.splitext(os.path.basename(file_name))

        with open(file_name, 'r') as f:
            orig_contents = f.read()
            modified_contents = process(file_name_ext, orig_contents)

        if not orig_contents == modified_contents:
            print('Updated {}'.format(file_name))
            with open(file_name, 'w') as f:
                f.write(modified_contents)

        if not 'PROFILE(' in modified_contents and not ext == '.hpp':
            print('File {} has no profile statements'.format(file_name))


if __name__ == '__main__':
    dirs_to_process = ('rfcommon', 'rfplot', 'plugins', 'application')
    for dirs in dirs_to_process:
        process_files(dirs)
