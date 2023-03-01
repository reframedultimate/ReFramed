#!/usr/bin/env python3
import sys
import os
import re

re_func = re.compile(
    (
        r'\b\w+(?:[\*&:<>]+)?[\n\t ]{1,}'                               # Require return type
        r'(?:(\w+)white::white)?'                                       # Optional class name
        r'(\w+)white\((?:[\*&:<>/\w\n\t,= ]+)?\)white(?:const)?white\{' # Required function name
        r'(?:whitePROFILEwhite\(white(\w+)white,white(\w+)white\))?'    # Optional PROFILE()
    ).replace('white', r'(?:[\n\t ]+)?')                                # optional white spaces
)

re_fopen = re.compile(
    r"[^a-z\._]fopen ?\("
)

re_remove = re.compile(
    r"[^a-z\._]remove ?\("
)

SUCCESS = True


def check_utf8_fopen(file_name, contents):
    global SUCCESS

    # We use fopen() in the implementation of utf8_fopen_xx() on linux
    if file_name.endswith("Utf8_linux.cpp"):
        return

    for line_idx, line in enumerate(contents.split("\n")):
        match = re_fopen.search(line)
        if match is None:
            continue

        SUCCESS = False

        sys.stderr.write(os.path.abspath(file_name) + ":" + str(line_idx+1) + ":" + str(match.start()+1) + "\n")
        sys.stderr.write(line + "\n")
        sys.stderr.write(" "*match.start() + "~"*(match.end()-match.start()-1) + "^\n")
        sys.stderr.write("Using fopen() will fail on Windows if the file path contains unicode. Use utf8_fopen_rb() or utf8_fopen_wb() from <rfcommon/Utf8.hpp> instead, or consider using rfcommon::MappedFile\n\n")


def check_utf8_remove(file_name, contents):
    global SUCCESS

    # We use remove() in the implementation of utf8_remove() on linux
    if file_name.endswith("Utf8_linux.cpp"):
        return

    for line_idx, line in enumerate(contents.split("\n")):
        match = re_remove.search(line)
        if match is None:
            continue

        SUCCESS = False

        sys.stderr.write(os.path.abspath(file_name) + ":" + str(line_idx+1) + ":" + str(match.start()+1) + "\n")
        sys.stderr.write(line + "\n")
        sys.stderr.write(" "*match.start() + "~"*(match.end()-match.start()-1) + "^\n")
        sys.stderr.write("Using remove() will fail on Windows if the file path contains unicode. Use utf8_remove() from <rfcommon/Utf8.hpp> instead\n\n")


def do_lint(file_name, contents):
    check_utf8_fopen(file_name, contents)
    check_utf8_remove(file_name, contents)


def build_files_list(directory, lst, allowed_extensions):
    for dir_path, sub_dir_list, file_list in os.walk(directory):
        for file_name in file_list:
            if not any(file_name.endswith(x) for x in allowed_extensions):
                continue
            lst.append(os.path.join(dir_path, file_name))


def process_files(directory):
    allowed_extensions = ('.cpp', '.c', '.hpp', '.h')
    files_to_process = list()
    build_files_list(directory, files_to_process, allowed_extensions)

    for file_name in files_to_process:
        with open(file_name, 'rb') as f:
            contents = f.read().decode('utf-8')
            do_lint(file_name, contents)


if __name__ == '__main__':
    dirs_to_process = ('rfcommon', 'rfplot', 'plugins', 'application')
    for dirs in dirs_to_process:
        process_files(dirs)

    sys.exit(0 if SUCCESS else -1)
