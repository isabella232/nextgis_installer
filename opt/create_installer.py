#!/usr/bin/env python
# -*- coding: utf-8 -*-
################################################################################
##
## Project: NextGIS online/offline installer
## Author: Dmitry Baryshnikov <dmitry.baryshnikov@nextgis.com>
## Copyright (c) 2016 NextGIS <info@nextgis.com>
## License: GPL v.2
##
################################################################################

import argparse
import os
import shutil
import string
import subprocess
import sys
import xml.etree.ElementTree as ET
import time
import pickle
import dmgbuild
import glob

args = {}
libraries_version_dict = {}

repogen_file = ''
binarycreator_file = ''

repo_config_path = ''
repo_target_path = ''
repo_source_path = ''
repo_remote_path = ''
repo_root_dir = ''
repo_new_packages_path = ''
repo_new_config_path = ''
translate_tool = ''
packages_data_source_path = ''

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    OKGRAY = '\033[0;37m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
    DGRAY='\033[1;30m'
    LRED='\033[1;31m'
    LGREEN='\033[1;32m'
    LYELLOW='\033[1;33m'
    LBLUE='\033[1;34m'
    LMAGENTA='\033[1;35m'
    LCYAN='\033[1;36m'
    WHITE='\033[1;37m'

def color_print(text, bold, color):
    if sys.platform == 'win32':
        print text
    else:
        out_text = ''
        if bold:
            out_text += bcolors.BOLD
        if color == 'GREEN':
            out_text += bcolors.OKGREEN
        elif color == 'LGREEN':
            out_text += bcolors.LGREEN
        elif color == 'LYELLOW':
            out_text += bcolors.LYELLOW
        elif color == 'LMAGENTA':
            out_text += bcolors.LMAGENTA
        elif color == 'LCYAN':
            out_text += bcolors.LCYAN
        elif color == 'LRED':
            out_text += bcolors.LRED
        elif color == 'LBLUE':
            out_text += bcolors.LBLUE
        elif color == 'DGRAY':
            out_text += bcolors.DGRAY
        elif color == 'OKGRAY':
            out_text += bcolors.OKGRAY
        else:
            out_text += bcolors.OKGRAY
        out_text += text + bcolors.ENDC
        print out_text

def parse_arguments():
    global args

    parser = argparse.ArgumentParser(description='Create NextGIS desktop software installer.')
    parser.add_argument('-q', dest='qt_bin', required=True, help='Qt binary path (path to lrelease)')
    parser.add_argument('-t', dest='target', required=True, help='target path')
    parser.add_argument('-s', dest='source', required=True, help='Packages data source path (i.e. borsch root directory)')
    parser.add_argument('-r', dest='remote', required=False, help='repositry remote url')
    parser.add_argument('-n', dest='network', action='store_true', help='online installer (the -r key should be present)')

    subparsers = parser.add_subparsers(help='command help', dest='command')
    parser_create = subparsers.add_parser('create')
    parser_update = subparsers.add_parser('update')
    args = parser.parse_args()

def run(args):
    print 'calling ' + string.join(args)
    subprocess.check_call(args)

def load_versions():
    if os.path.exists('versions.pkl'):
        global libraries_version_dict
        with open('versions.pkl', 'rb') as f:
            libraries_version_dict = pickle.load(f)

def save_versions():
    with open('versions.pkl', 'wb') as f:
        pickle.dump(libraries_version_dict, f, pickle.HIGHEST_PROTOCOL)

def init():
    color_print('Initializing ...', True, 'LYELLOW')
    global repogen_file
    global binarycreator_file
    global repo_config_path
    global repo_source_path
    global repo_target_path
    global repo_new_packages_path
    global repo_root_dir
    global repo_new_config_path
    global translate_tool
    global packages_data_source_path

    load_versions()

    repo_root_dir = os.path.dirname(os.path.abspath(os.path.dirname(sys.argv[0])))

    bin_dir = os.path.join(repo_root_dir, 'qtifw_pkg', 'bin')
    repogen_file = os.path.join(bin_dir, 'repogen')
    color_print('repogen path: ' + repogen_file, True, 'LCYAN')
    binarycreator_file = os.path.join(bin_dir, 'binarycreator')
    color_print('binarycreator path: ' + binarycreator_file, True, 'LCYAN')
    scripts_path = os.path.join(repo_root_dir, 'qtifw_scripts')
    repo_config_path = os.path.join(scripts_path, 'config')
    repo_source_path = os.path.join(scripts_path, 'packages')
    repo_target_path = os.path.abspath(args.target)
    color_print('build path: ' + repo_target_path, True, 'LCYAN')
    repo_new_packages_path = os.path.join(repo_target_path, 'packages')
    repo_new_config_path = os.path.join(repo_target_path, 'config')

    if args.network:
        global repo_remote_path
        repo_remote_path = args.remote
        print 'remote repository URL: ' + repo_remote_path

    translate_tool = os.path.join(args.qt_bin, 'lrelease')
    if not os.path.exists(translate_tool):
        sys.exit('No translate tool exists')

    packages_data_source_path = os.path.abspath(args.source)
    if not os.path.exists(packages_data_source_path):
        sys.exit('Invalid packages data source path')

def get_repository_path():
    out_path = os.path.join(repo_target_path, 'repository')
    if sys.platform == 'darwin':
        out_path += '-mac'
    elif sys.platform == 'win32':
        out_path += '-win'
    else: # This should never happened
        out_path += '-nix'
    return out_path

def prepare_config():
    os.makedirs(repo_new_config_path)
    tree = ET.parse(os.path.join(repo_config_path, 'config.xml'))
    root = tree.getroot()

    installer_tag = root
    banner_tag = installer_tag.find('Banner')
    shutil.copy(os.path.join(repo_root_dir, 'art', banner_tag.text), repo_new_config_path)

    inst_app_icon_tag = installer_tag.find('InstallerApplicationIcon')
    inst_app_icon_path = os.path.join(repo_root_dir, 'art', inst_app_icon_tag.text)
    if sys.platform == 'darwin':
        inst_app_icon_tag.text += '.icns'
        inst_app_icon_path += '.icns'
        shutil.copy(os.path.join(repo_root_dir, 'art', 'bk.png'), repo_new_config_path)
    elif sys.platform == 'win32':
        inst_app_icon_tag.text += '.ico'
        inst_app_icon_path += '.ico'
    else:
        inst_app_icon_tag.text += '.png'
        inst_app_icon_path += '.png'

    shutil.copy(inst_app_icon_path, repo_new_config_path)

    inst_wnd_icon_tag = installer_tag.find('InstallerWindowIcon')
    shutil.copy(os.path.join(repo_root_dir, 'art', inst_wnd_icon_tag.text), repo_new_config_path)
    url_tag = installer_tag.find('RemoteRepositories/Repository/Url')
    if args.network:
        url_tag.text = repo_remote_path
    else:
        url_tag.text = 'file://' + get_repository_path()

    wizard_tag = installer_tag.find('WizardStyle')
    targetdir_tag = installer_tag.find('TargetDir')
    if sys.platform == 'darwin':
        wizard_tag.text = 'Modern'#'Mac'
    #    targetdir_tag = '@RootDir@' <-- root dir is forbidden as it will be deleted on reinstall
    else:
        wizard_tag.text = 'Modern'
    targetdir_tag.text = '@ApplicationsDir@/NextGIS'

    tree.write(os.path.join(repo_new_config_path, 'config.xml'))
    shutil.copy(os.path.join(repo_config_path, 'initscript.qs'), repo_new_config_path)

def copyFiles(tag, sources_root_dir, data_path):
    for path in tag.iter('path'):
        src_path = path.attrib['src']
        dst_path = path.attrib['dst']
        src_path_full = os.path.join(sources_root_dir, src_path)
        dst_path_full = os.path.join(data_path, dst_path)

        if os.path.isdir(src_path_full):
            shutil.copytree(src_path_full, dst_path_full)
        else:
            if not os.path.exists(dst_path_full):
                os.makedirs(dst_path_full)

            if src_path_full.find('*'):
                # Copy by wildcard.
                for copy_file in glob.glob(src_path_full):
                    shutil.copy(copy_file, dst_path_full)
            else:
                if os.path.exists(src_path_full):
                    shutil.copy(src_path_full, dst_path_full)
    return

def get_version_text(sources_root_dir, component_name):
    has_changes = False
    version_file_path = os.path.join(packages_data_source_path, sources_root_dir, 'build', 'version.str')
    with open(version_file_path) as f:
        content = f.readlines()
        version_str = content[0].rstrip()
        version_file_date = content[1].rstrip()

    if component_name in libraries_version_dict:
        if libraries_version_dict[component_name]['version'] == version_str:
            if libraries_version_dict[component_name]['date'] == version_file_date:
                version_str += '-' + str(libraries_version_dict[component_name]['count'])
            else:
                count = libraries_version_dict[component_name]['count'] + 1
                version_str += '-' + str(count)
                libraries_version_dict[component_name]['count'] = count
                libraries_version_dict[component_name]['date'] = version_file_date
                has_changes = True
        else:
            libraries_version_dict[component_name]['count'] = 0
            libraries_version_dict[component_name]['date'] = version_file_date
            libraries_version_dict[component_name]['version'] = version_str
            version_str += '-0'
            has_changes = True
    else:
        libraries_version_dict[component_name] = dict(count = 0, date = version_file_date, version = version_str)
        version_str += '-0'
        has_changes = True

    return version_str, has_changes

def process_files_in_meta_dir(path_meta, new_meta_path):
    for meta_file in os.listdir(path_meta):
        if meta_file == 'package.xml':
            continue

        meta_file_path = os.path.join(path_meta, meta_file)
        filename, file_extension = os.path.splitext(os.path.basename(meta_file_path))
        if file_extension == '.ts':
            # Create translation
            output_translation_path = os.path.join(new_meta_path, filename) + '.qm'
            run((translate_tool, meta_file_path, '-qm', output_translation_path))
        elif file_extension == '.qs':
            shutil.copy(meta_file_path, new_meta_path)

def create_dest_package_dir(dir_name, version_text, updatetext_text, sources_dir, repo_new_package_path, data_root_tag, path_meta):
    os.makedirs(repo_new_package_path)
    new_data_path = os.path.join(repo_new_package_path, 'data')
    if sys.platform == 'darwin':
        mac_tag = data_root_tag.find('mac')
        if mac_tag is not None:
            copyFiles(mac_tag, sources_dir, new_data_path)
    elif sys.platform == 'win32':
        win_tag = data_root_tag.find('win')
        if win_tag is not None:
            copyFiles(win_tag, sources_dir, new_data_path)

    # Process package.xml
    tree = ET.parse(os.path.join(path_meta, 'package.xml'))
    root = tree.getroot()
    releasedate_tag = root.find('ReleaseDate')
    if releasedate_tag is None:
        releasedate_tag = ET.SubElement(root, 'ReleaseDate')
    releasedate_tag.text = time.strftime("%Y-%m-%d")
    name_tag = root.find('Name')
    if name_tag is None:
        name_tag = ET.SubElement(root, 'Name')
    name_tag.text = dir_name
    version_tag = root.find('Version')
    if version_tag is None:
        version_tag = ET.SubElement(root, 'Version')
    version_tag.text = version_text
    if updatetext_text is not None:
        updatetext_tag = root.find('UpdateText')
        if updatetext_tag is None:
            updatetext_tag = ET.SubElement(root, 'UpdateText')
        updatetext_tag.text = updatetext_text

    new_meta_path = os.path.join(repo_new_package_path, 'meta')
    os.makedirs(new_meta_path)
    tree.write(os.path.join(new_meta_path, 'package.xml'))
    process_files_in_meta_dir(path_meta, new_meta_path)


def process_directory(dir_name):
    color_print('Process ' + dir_name, True, 'LBLUE')
    path = os.path.join(repo_source_path, dir_name)
    path_meta = os.path.join(path, 'meta')
    path_data = os.path.join(path, 'data')

    if not os.path.exists(path_data):
        return

    # Parse install.xml file
    if not os.path.exists(os.path.join(path_data, 'package.xml')):
        return

    tree = ET.parse(os.path.join(path_data, 'package.xml'))
    root = tree.getroot()

    sources_root_dir = ''
    if 'root' in root.attrib:
        sources_root_dir = root.attrib['root']
        version_text, has_changes = get_version_text(sources_root_dir, dir_name)
    else:
        version_text = root.find('Version').text
        libraries_version_dict[dir_name] = dict(count = 0, date = '1900-01-01 00:00:00', version = version_text)

    updatetext_tag = root.find('UpdateText')
    updatetext_text = None
    if updatetext_tag is not None:
        updatetext_text = updatetext_tag.text

    # Copy files
    repo_new_package_path = os.path.join(repo_new_packages_path, dir_name)
    create_dest_package_dir(dir_name, version_text, updatetext_text, os.path.join(packages_data_source_path, sources_root_dir), repo_new_package_path, root, path_meta)


def prepare_packages():
    os.makedirs(repo_new_packages_path)

    # Scan for directories and files
    for subdir in os.listdir(repo_source_path):
        if os.path.isdir(os.path.join(repo_source_path, subdir)):
            process_directory(subdir)

def delete_path(path_to_delete):
    color_print('Delete existing build dir ...', True, 'LRED')
    shutil.rmtree(path_to_delete, ignore_errors=True)

def prepare():
    color_print('Preparing ...', True, 'LYELLOW')

    if os.path.exists(repo_target_path):
        delete_path(repo_target_path)
    if os.path.exists(repo_target_path):
        delete_path(repo_target_path)
    os.makedirs(repo_target_path)

    prepare_config()
    prepare_packages()

def update_directory(dir_name):
    color_print('Update ' + dir_name, True, 'LBLUE')
    path = os.path.join(repo_source_path, dir_name)
    path_meta = os.path.join(path, 'meta')
    path_data = os.path.join(path, 'data')

    if not os.path.exists(path_data):
        return

    # Check versions, if differ - delete directory and load it from sources.
    if not os.path.exists(os.path.join(path_data, 'package.xml')):
        return

    tree = ET.parse(os.path.join(path_data, 'package.xml'))
    root = tree.getroot()

    sources_root_dir = ''
    if 'root' in root.attrib:
        sources_root_dir = root.attrib['root']
        version_text, has_changes = get_version_text(sources_root_dir, dir_name)
    else:
        version_text = root.find('Version').text
        has_changes = False
        if dir_name in libraries_version_dict:
            if libraries_version_dict[dir_name]['version'] != version_text:
                has_changes = True
        else:
            has_changes = True

    repo_new_package_path = os.path.join(repo_new_packages_path, dir_name)
    if not has_changes:
        # Update translations
        new_meta_path = os.path.join(repo_new_package_path, 'meta')
        process_files_in_meta_dir(path_meta, new_meta_path)
        color_print('No changes in ' + dir_name, True, 'LCYAN')
        return

    updatetext_tag = root.find('UpdateText')
    updatetext_text = None
    if updatetext_tag is not None:
        updatetext_text = updatetext_tag.text

    if os.path.exists(repo_new_package_path):
        color_print('Delete existing dir ' + repo_new_package_path, True, 'LRED')
        shutil.rmtree(repo_new_package_path, ignore_errors=True)
    create_dest_package_dir(dir_name, version_text, updatetext_text, os.path.join(packages_data_source_path, sources_root_dir), repo_new_package_path, root, path_meta)

def update():
    # Scan for directories and files
    source_dirs = os.listdir(repo_source_path)
    for subdir in source_dirs:
        if os.path.isdir(os.path.join(repo_source_path, subdir)):
            update_directory(subdir)
    # Delete not exist directories
    for subdir in os.listdir(repo_new_packages_path):
        if not subdir in source_dirs:
            delete_path(os.path.join(repo_source_path, subdir))

def create_installer():
    run((repogen_file, '--remove', '-v', '-p', repo_new_packages_path, get_repository_path()))
    key_only = '--offline-only'
    if args.network:
        key_only = '--online-only'
    run((binarycreator_file, '-v', key_only, '-c', os.path.join(repo_new_config_path, 'config.xml'), '-p', repo_new_packages_path, os.path.join(repo_target_path, 'nextgis-setup') ))

    # Hack as <InstallerApplicationIcon> in config.xml not working
    if sys.platform == 'darwin':
        icns_path = os.path.join(repo_target_path, 'nextgis-setup.app', 'Contents', 'Resources', 'nextgis-setup.icns' )
        os.unlink(icns_path)
        shutil.copy(os.path.join(repo_new_config_path, 'nextgis-setup.icns'), icns_path)
        # Build dgm image file
        color_print('Create DMG file ...', True, 'LMAGENTA')
        dmgbuild.build_dmg(
            os.path.join(repo_target_path, 'nextgis-setup.dmg'),
            'NextGIS Setup',
            os.path.join(repo_root_dir, 'opt', 'dmg_settings.py'),
            defines=dict(badge_icon=os.path.join(repo_new_config_path, 'nextgis-setup.icns'),
                 background=os.path.join(repo_new_config_path, 'bk.png'),
                 files=[os.path.join(repo_target_path, 'nextgis-setup.app')]),
            lookForHiDPI=False)

    color_print('DONE, installer is at ' + os.path.join(repo_target_path, 'nextgis-setup'), True, 'LMAGENTA')

def update_istaller():
    run((repogen_file, '--update-new-components', '-v', '-p', repo_new_packages_path, get_repository_path()))
    color_print('DONE, installer is at ' + os.path.join(repo_target_path, 'nextgis-setup'), True, 'LMAGENTA')

parse_arguments()
init()
if args.command == 'create':
    prepare()
    create_installer()
elif args.command == 'update':
    update()
    update_istaller()
else:
    exit('Unsupported command')
save_versions()
