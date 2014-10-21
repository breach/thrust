#!/usr/bin/env python

import argparse
import os
import re
import shutil
import subprocess
import sys
import tarfile

from config import LIBCHROMIUMCONTENT_COMMIT, BASE_URL, TARGET_PLATFORM, \
                   DIST_ARCH
from util import scoped_cwd, rm_rf, get_thrust_version, make_zip, \
                 safe_mkdir, execute


THRUST_SHELL_VERSION = get_thrust_version()

SOURCE_ROOT = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))
DIST_DIR = os.path.join(SOURCE_ROOT, 'dist')
OUT_DIR = os.path.join(SOURCE_ROOT, 'out', 'Release')

SYMBOL_NAME = {
  'darwin': 'libchromiumcontent.dylib.dSYM',
  'linux': 'libchromiumcontent.so.dbg',
  'win32': 'chromiumcontent.dll.pdb',
}[TARGET_PLATFORM]

TARGET_BINARIES = {
  'darwin': [
  ],
  'win32': [
    'thrust_shell.exe',
    'chromiumcontent.dll',
    'content_shell.pak',
    'ffmpegsumo.dll',
    'icudtl.dat',
    'libEGL.dll',
    'libGLESv2.dll',
    'ui_resources_200_percent.pak',
    'webkit_resources_200_percent.pak',
  ],
  'linux': [
    'thrust_shell',
    'content_shell.pak',
    'icudtl.dat',
    'libchromiumcontent.so',
    'libffmpegsumo.so',
  ],
}
TARGET_DIRECTORIES = {
  'darwin': [
    'ThrustShell.app',
  ],
  'win32': [
#    'resources',
    'locales',
  ],
  'linux': [
#    'resources',
    'locales',
  ],
}

SYSTEM_LIBRARIES = [
  'libudev.so',
  'libgcrypt.so',
  'libnotify.so',
]

HEADERS_SUFFIX = [
  '.h',
  '.gypi',
]
HEADERS_DIRS = [
  'src',
]
HEADERS_FILES = [
  'common.gypi',
  'config.gypi',
]


def main():
  rm_rf(DIST_DIR)
  os.makedirs(DIST_DIR)

  args = parse_args()

  force_build()
  copy_binaries()
  copy_license()

  if TARGET_PLATFORM == 'linux':
    copy_system_libraries()

  create_version()
  create_dist_zip()


def parse_args():
  parser = argparse.ArgumentParser(description='Create distributions')
  parser.add_argument('-u', '--url',
                      help='The base URL from which to download '
                      'libchromiumcontent (i.e., the URL you passed to '
                      'libchromiumcontent\'s script/upload script',
                      default=BASE_URL,
                      required=False)
  return parser.parse_args()


def force_build():
  build = os.path.join(SOURCE_ROOT, 'scripts', 'build.py')
  execute([sys.executable, build, '-c', 'Release'])


def copy_binaries():
  for binary in TARGET_BINARIES[TARGET_PLATFORM]:
    shutil.copy2(os.path.join(OUT_DIR, binary), DIST_DIR)

  for directory in TARGET_DIRECTORIES[TARGET_PLATFORM]:
    shutil.copytree(os.path.join(OUT_DIR, directory),
                    os.path.join(DIST_DIR, directory),
                    symlinks=True)

def copy_license():
  shutil.copy2(os.path.join(SOURCE_ROOT, 'LICENSE'), DIST_DIR)
  shutil.copy2(os.path.join(SOURCE_ROOT, 'LICENSE-BRIGHTRAY'), DIST_DIR)
  shutil.copy2(os.path.join(SOURCE_ROOT, 'LICENSE-CHROMIUM'), DIST_DIR)


def copy_system_libraries():
  ldd = execute(['ldd', os.path.join(OUT_DIR, 'thrust_shell')])
  lib_re = re.compile('\t(.*) => (.+) \(.*\)$')
  for line in ldd.splitlines():
    m = lib_re.match(line)
    if not m:
      continue
    for i, library in enumerate(SYSTEM_LIBRARIES):
      real_library = m.group(1)
      if real_library.startswith(library):
        shutil.copyfile(m.group(2), os.path.join(DIST_DIR, real_library))
        SYSTEM_LIBRARIES[i] = real_library


def create_version():
  version_path = os.path.join(SOURCE_ROOT, 'dist', 'version')
  with open(version_path, 'w') as version_file:
    version_file.write(THRUST_SHELL_VERSION)


def create_dist_zip():
  dist_name = 'thrust-{0}-{1}-{2}.zip'.format(THRUST_SHELL_VERSION,
                                              TARGET_PLATFORM, 
                                              DIST_ARCH)
  zip_file = os.path.join(SOURCE_ROOT, 'dist', dist_name)

  with scoped_cwd(DIST_DIR):
    files = TARGET_BINARIES[TARGET_PLATFORM] +  ['LICENSE', 'version']
    if TARGET_PLATFORM == 'linux':
      files += SYSTEM_LIBRARIES
    dirs = TARGET_DIRECTORIES[TARGET_PLATFORM]
    make_zip(zip_file, files, dirs)

if __name__ == '__main__':
  sys.exit(main())
