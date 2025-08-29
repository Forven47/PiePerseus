import argparse
import glob
import multiprocessing
import os
import shutil
import time
import zipfile
import itertools
from subprocess import Popen, PIPE, STDOUT, run
import logging
import sys

logging.basicConfig(
    format='%(asctime)s,%(msecs)03d %(levelname)-8s [%(filename)s:%(funcName)s:%(lineno)d] - %(message)s',
    datefmt='%Y-%m-%d:%H:%M:%S',
    level=logging.DEBUG)

pkg = 'com.YoStarEN.AzurLane'
pkg_version = '0'
rootdir = os.getcwd()

skip = False
clean = False
quick_rebuild = False


def is_windows() -> bool:
    return os.name in ['nt']


def mkcd(d):
    if not os.path.isdir(d):
        os.mkdir(d)
    os.chdir(d)


def executable_path(e, absolute=True):
    e = os.path.join(rootdir if absolute else '', 'bin', e)
    if is_windows():
        e += '.exe'
    return e


def bbox(cmd):
    # peak laziness
    busybox = os.path.join(rootdir, 'bin', 'busybox')
    if is_windows():
        busybox += '.exe'

    logging.info(f'{busybox} {cmd}')
    os.system(f'{busybox} {cmd}')


def get_version():
    logging.info(f'fetching {pkg} versions')
    global pkg_version
    line = ''
    with Popen([executable_path('apkeep'), '-a', pkg, '-l', 'unused.txt'],
               stdout=PIPE, stderr=STDOUT, text=True) as proc:
        for outputline in proc.stdout:
            line = outputline
    v = line.split(' ')[-1].strip()
    logging.info(f'found {pkg} version {v}')
    pkg_version = v


def download_azurlane():
    if skip and os.path.isfile(f'{pkg}.xapk'):
        logging.info(f'{pkg}.xapk already exists, skipping')
        return
    logging.info(f'downloading {pkg}.xapk')
    os.system(f'{executable_path("apkeep")} -a {pkg} .')


def build_perseus_lib(do_clean=False):
    logging.info(f'{"cleaning" if do_clean else "building"} perseus libs')

    # thx n0k0m3
    os.chdir('PerseusLib')

    cmd = [f'ndk-build{".cmd" if is_windows() else ""}',
           'NDK_PROJECT_PATH=./src',
           'NDK_APPLICATION_MK=./src/Application.mk',
           'APP_BUILD_SCRIPT=./src/Android.mk',
           'APP_PLATFORM=android-21',
           f'-j{multiprocessing.cpu_count()}'] + (['clean'] if do_clean else [])

    ndk_proc = run(cmd, capture_output=True, text=True)

    output = ndk_proc.stdout
    return_code = ndk_proc.returncode

    if return_code != 0:
        logging.error('ndk-build failed')

        print("======== ndk-build stdout ========", file=sys.stderr)
        print(output, file=sys.stderr)
        print("======== ndk-build stderr ========", file=sys.stderr)
        print(ndk_proc.stderr, file=sys.stderr)
        exit(1)

    os.chdir('..')


def extract_xapk():
    if skip and os.path.isfile(f'{pkg}.apk'):
        logging.info(f'{pkg}.apk is already extracted, skipping')
        return

    logging.info(f'extracting {pkg}.xapk')
    bbox(f'unzip -q -o ../{pkg}.xapk -d . {pkg}.apk')


def decompile_apk():
    if skip and os.path.isdir(pkg):
        logging.info(f'{pkg}.apk is already decompiled, skipping')
        return

    logging.info(f'decompiling {pkg}.apk')
    if os.path.isdir(pkg):
        shutil.rmtree(pkg)
    os.system(f'java -jar {os.path.join(rootdir, "bin", "apktool.jar")} -q -f d {pkg}.apk')


def copy_perseus_libs():
    logging.info('copying Perseus libs')
    bbox(f'sh -c "cp -r ../PerseusLib/src/libs/* {pkg}/lib"')


def patch():
    logging.info('patching decompiled sources')
    bbox(f"sh {os.path.join(rootdir, 'scripts', 'patch.sh')}")


def rebuild():
    newpkg = f'{pkg}-{pkg_version}.patched.apk'
    newzip = newpkg + '.zip'
    if quick_rebuild and os.path.isfile(newpkg):
        logging.info(f'rebuiling {pkg}.apk quickly')
        
        shutil.move(newpkg, newzip)
        
        libs = [f'{pkg}/lib/{arch}/libPerseus.so' for arch in ['arm64-v8a', 'armeabi-v7a', 'x86']]
        libs_renamed = [lib.removeprefix(f'{pkg}/') for lib in libs]

        proc1 = run([executable_path('7zz'), '-y', 'd', newzip] + libs_renamed, stdout=PIPE)
        logging.info(f'deleting libs in archive, ret={proc1.returncode}')

        proc2 = run([executable_path('7zz'), '-y', 'a', newzip] + libs, stdout=PIPE)
        logging.info(f'adding libs to archive, ret={proc2.returncode}')
        
        proc3 = run([executable_path('7zz'), '-y', 'rn', newzip] + list(itertools.chain.from_iterable(zip(libs, libs_renamed))), stdout=PIPE)
        logging.info(f'renaming libs in archive, ret={proc3.returncode}')
        
        shutil.move(newzip, newpkg)

        return

    logging.info(f'rebuilding {pkg}.apk with apktool')
    os.system(f'java -jar {os.path.join(rootdir, "bin", "apktool.jar")} -q -f b {pkg} -o {newpkg}')


def sign_apk():
    f = f'{pkg}-{pkg_version}.patched.apk'
    shutil.move(f, f + '.unsigned')

    logging.info('zipaligning apk')
    zipalign = f'zipalign{".exe" if is_windows() else ""}'
    os.system(f'{zipalign} -pf 4 {f}.unsigned {f}')
    os.remove(f'{f}.unsigned')

    logging.info('signing apk')
    apksigner = f'apksigner{".bat" if is_windows() else ""}'
    key = os.path.join(rootdir, 'signing', 'testkey.pk8')
    cert = os.path.join(rootdir, 'signing', 'testkey.x509.pem')
    os.system(f'{apksigner} sign --key {key} --cert {cert} {f}')

    os.remove(f'{f}.idsig')


def main():
    global skip, clean, quick_rebuild

    parser = argparse.ArgumentParser(
        prog='perseus apk builder',
        description='builds apk for you (this is the default behaviour if called with no arguments)')

    parser.add_argument('--skip', 
                        help='skip decompile and extracting if possible', 
                        default=True,
                        action=argparse.BooleanOptionalAction)
    parser.add_argument('--quick-rebuild',
                        help='rebuild apk by replacing libs in the apk instead of using apktool (saves 40s)',
                        default=True,
                        action=argparse.BooleanOptionalAction)
    parser.add_argument('--clean',
                        help='delete built apk, decompiled sources, compiled perseus libs and xapk',
                        default=False,
                        action=argparse.BooleanOptionalAction)
    args = parser.parse_args()

    skip = args.skip
    clean = args.clean
    quick_rebuild = args.quick_rebuild

    if clean:
        logging.info('cleaning')
        for f in glob.glob('*.xapk'):
            os.remove(path=f)
        if os.path.isdir('apk_build'):
            shutil.rmtree('apk_build')
        build_perseus_lib(do_clean=True)

    else:
        start = time.time()
        get_version()
        download_azurlane()
        build_perseus_lib()
        mkcd('apk_build')
        extract_xapk()
        decompile_apk()
        copy_perseus_libs()
        patch()
        rebuild()
        sign_apk()
        end = time.time()

        logging.info(f"built apk in {os.path.join(rootdir, 'apk_build', f'{pkg}-{pkg_version}.patched.apk')}")
        logging.info(f"done in {round(end - start, 2)} seconds")


if __name__ == '__main__':
    main()
