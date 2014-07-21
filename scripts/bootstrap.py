#!/usr/bin/env python

import argparse
import os
import sys
import subprocess

from config import LIBCHROMIUMCONTENT_COMMIT, BASE_URL


SOURCE_ROOT = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))
VENDOR_DIR = os.path.join(SOURCE_ROOT, 'vendor')


def execute(argv):
  try:
    return subprocess.check_output(argv, stderr=subprocess.STDOUT)
  except subprocess.CalledProcessError as e:
    print e.output
    raise e


def main():
  os.chdir(SOURCE_ROOT)

  args = parse_args()
  update_submodules()
  bootstrap_brightray(args.url)
  create_chrome_version_h()


def parse_args():
  parser = argparse.ArgumentParser(description='Bootstrap this project')
  parser.add_argument('-u', '--url',
                      help='The base URL from which to download '
                      'libchromiumcontent (i.e., the URL you passed to '
                      'libchromiumcontent\'s script/upload script',
                      default=BASE_URL,
                      required=False)
  return parser.parse_args()


def update_submodules():
  execute(['git', 'submodule', 'sync'])
  execute(['git', 'submodule', 'update', '--init', '--recursive'])


def bootstrap_brightray(url):
  bootstrap = os.path.join(VENDOR_DIR, 'brightray', 'script', 'bootstrap')
  execute([sys.executable, bootstrap, '--commit', LIBCHROMIUMCONTENT_COMMIT,
           url])

def create_chrome_version_h():
  version_file = os.path.join(SOURCE_ROOT, 'vendor', 'brightray', 'vendor',
                              'libchromiumcontent', 'VERSION')
  target_file = os.path.join(SOURCE_ROOT, 'src', 'common', 'chrome_version.h')
  template_file = os.path.join(SOURCE_ROOT, 'scripts', 'chrome_version.h.in')

  with open(version_file, 'r') as f:
    version = f.read()
  with open(template_file, 'r') as f:
    template = f.read()
  with open(target_file, 'w+') as f:
    content = template.replace('{PLACEHOLDER}', version.strip())
    if f.read() != content:
      f.write(content)


if __name__ == '__main__':
  sys.exit(main())
