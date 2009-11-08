#! /usr/bin/env python


from glob import glob


VERSION = '0.9'
APPNAME = 'minidjvu'

srcdir = '.'
blddir = 'build'


def set_options(opt):
    pass


def configure(conf):
    conf.check_tool('gcc g++')

    conf.check(header_name='math.h',  lib='m', uselib='M')

    conf.check(header_name='tiffio.h',  lib='tiff', uselib='TIFF',
               define_name='HAVE_TIFF', mandatory=False)

    conf.check(header_name='libintl.h', 
               define_name='HAVE_I18N', mandatory=False)

    conf.check(header_name='stdint.h', 
               define_name='HAVE_STDINT', mandatory=False)


def build(bld):
    bld.new_task_gen(
        features = 'cc cxx cshlib cstaticlib',
        source = bld.glob('src/*/*.c') + bld.glob('src/*/*.cpp'),
        target = 'minidjvu',
        includes = 'include',
        uselib = 'TIFF'
    )
    
    bld.new_task_gen(
        features = "cc cxx cprogram",
        source = "tools/minidjvu.c",
        target = "minidjvu",
        includes = "include",
        install_path = "${PREFIX}/bin",
        uselib = 'M TIFF',
        uselib_local = "minidjvu"
    )
    
    bld.install_files('include/minidjvu', '${PREFIX}/include')
