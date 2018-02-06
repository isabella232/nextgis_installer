#!/usr/bin/env python
#############################################################################
##
## Copyright (C) 2017 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the Qt Installer Framework.
##
## $QT_BEGIN_LICENSE:GPL-EXCEPT$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 3 as published by the Free Software
## Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

import argparse
import os
import shutil
import string
import subprocess
import sys

args = {}

src_dir = ''
build_dir = ''
package_dir = ''
archive_file = ''
target_path = ''

def parse_arguments():
    global args

    parser = argparse.ArgumentParser(description='Build installer for installer framework.')
    parser.add_argument('--clean', dest='clean', action='store_true', help='delete all previous build artifacts')
    parser.add_argument('--qtdir', dest='qt_dir', required=True, help='path to qmake that will be used to build the tools')
    parser.add_argument('--make', dest='make', required=True, help='make command')
    # if sys.platform == 'darwin':
    #     parser.add_argument('--qt_menu_nib', dest='menu_nib', required=True, help='location of qt_menu.nib (usually src/gui/mac/qt_menu.nib)')

    args = parser.parse_args()

def run(args):
    print 'calling ' + string.join(args)
    subprocess.check_call(args)

def init():
    global src_dir
    global build_dir
    global package_dir
    global target_path
    global qmake

    src_dir = os.path.dirname(os.getcwd())
    root_dir = os.path.dirname(src_dir)
    basename = os.path.basename(src_dir)
    build_dir = os.path.join(root_dir, basename + '_build')
    package_dir = os.path.join(root_dir, basename + '_pkg')
    target_path = os.path.join(build_dir, 'Qt Installer Framework')

    # This is buildbot run. Get absolute path.
    qt_dir = os.path.dirname(os.path.dirname(root_dir))
    qt_dir = os.path.join(qt_dir, args.qt_dir)
    abs_qt_dir = os.path.abspath(qt_dir)
    print 'abs_qt_dir: ' + abs_qt_dir
    for subdir in os.listdir(abs_qt_dir):
        test_path = os.path.join(abs_qt_dir, subdir, 'bin')
        if os.path.isdir(test_path):
            qmake = os.path.join(test_path, 'qmake')
            break

    print 'qmake: ' + qmake
    print 'source dir: ' + src_dir
    print 'build dir: ' + build_dir
    print 'package dir: ' + package_dir
    print 'target path: ' + target_path

    if args.clean and os.path.exists(build_dir):
        print 'delete existing build dir ...'
        shutil.rmtree(build_dir)
    if not os.path.exists(build_dir):
        os.makedirs(build_dir)

    if os.path.exists(target_path):
        print 'delete existing target dir ...'
        shutil.rmtree(target_path)
    os.makedirs(target_path)

    if os.path.exists(package_dir):
        print 'delete existing package dir ...'
        shutil.rmtree(package_dir)
    os.makedirs(package_dir)

def build():
    print 'building sources ...'
    os.chdir(build_dir)
    run((qmake, src_dir))
    run((args.make))

def package():
    global package_dir
    print 'package ...'
    os.chdir(package_dir)
    shutil.copytree(os.path.join(build_dir, 'bin'), os.path.join(package_dir, 'bin'), ignore = shutil.ignore_patterns("*.exe.manifest","*.exp","*.lib"))
    if sys.platform == 'linux2':
        run(('strip',os.path.join(package_dir, 'bin/archivegen')))
        run(('strip',os.path.join(package_dir, 'bin/binarycreator')))
        run(('strip',os.path.join(package_dir, 'bin/devtool')))
        run(('strip',os.path.join(package_dir, 'bin/installerbase')))
        run(('strip',os.path.join(package_dir, 'bin/repogen')))
    # shutil.copytree(os.path.join(build_dir, 'doc'), os.path.join(package_dir, 'doc'))
    # shutil.copytree(os.path.join(src_dir, 'examples'), os.path.join(package_dir, 'examples'))
    shutil.copy(os.path.join(src_dir, 'README'), package_dir)
    # create 7z
    archive_file = os.path.join(src_dir, 'dist', 'packages', 'org.qtproject.ifw.binaries', 'data', 'data.7z')
    if not os.path.exists(os.path.dirname(archive_file)):
        os.makedirs(os.path.dirname(archive_file))
    run((os.path.join(package_dir, 'bin', 'archivegen'), archive_file, '*'))
    # run installer
    binary_creator = os.path.join(build_dir, 'bin', 'binarycreator')
    config_file = os.path.join(src_dir, 'dist', 'config', 'config.xml')
    package_dir = os.path.join(src_dir, 'dist', 'packages')
    installer_path = os.path.join(src_dir, 'dist', 'packages')
    run((binary_creator, '--offline-only', '-c', config_file, '-p', package_dir, target_path))
    # if sys.platform == 'darwin':
    #     shutil.copytree(args.menu_nib, target_path + '.app/Contents/Resources/qt_menu.nib')


parse_arguments()
init()
build()
package()

print 'DONE, installer is at ' + target_path
