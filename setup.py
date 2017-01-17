#!/usr/bin/env python

"""
Tangram Python:

"""

from distutils.core import setup, Extension
from distutils import sysconfig
import sys

def get_platform():
    if sys.platform[:6] == "darwin":
        return "darwin"
    elif sys.platform[:5] == "linux":
        return "linux"
    else:
        return "unsupported"

# include directories
includes = """tangram/src/
              tangram/tangram-es/core/src/
              tangram/tangram-es/core/src/data/
              tangram/tangram-es/core/src/debug/
              tangram/tangram-es/core/src/gl/
              tangram/tangram-es/core/src/labels/
              tangram/tangram-es/core/src/marker/
              tangram/tangram-es/core/src/scene/
              tangram/tangram-es/core/src/style/
              tangram/tangram-es/core/src/text/
              tangram/tangram-es/core/src/tile/
              tangram/tangram-es/core/src/util/
              tangram/tangram-es/core/src/view/""".split()

# source files
sources = """tangram/tangram.i""".split()

link_args = """-fPIC
               tangram/lib/libalfons.a
               tangram/lib/libcore.a
               tangram/lib/libcss-color-parser-cpp.a
               tangram/lib/libduktape.a
               tangram/lib/libfreetype.a
               tangram/lib/libgeojson-vt-cpp.a
               tangram/lib/libglfw3.a
               tangram/lib/libharfbuzz.a
               tangram/lib/libicucommon.a
               tangram/lib/liblinebreak.a
               tangram/lib/libtangram.a
               tangram/lib/libyaml-cpp.a""".split()


doc_lines = __doc__.split("\n")
tangram_module = Extension('tangram/_tangram',
                           sources=sources,
                           include_dirs = includes,
                           extra_link_args = link_args,
                           swig_opts = ['-c++'])
setup(name='tangram',
      version='0.1',
      description='A Python wrapper for the 2D/3D map engine Tangram-ES',
      long_description=open('README.md').read(),
      url='https://github.com/tangrams/tangram-py',
      author='Mapzen',
      maintainer="Patricio Gonzalez Vivo",
      maintainer_email="patricio@mapzen.com",
      platforms = ["Mac OS-X", "Linux"],
      license='MIT',
      ext_modules = [tangram_module],
      py_modules = ["tangram"],
      packages=['tangram'])
