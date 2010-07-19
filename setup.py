from distutils.core import setup, Extension

module1 = Extension('PyBarry.javaloader', sources = ['src/PyBarry/javaloader.cc'], include_dirs=['/usr/include','/opt/local/include'], libraries=['barry'], library_dirs=['/usr/lib','/opt/local/lib'])

module2 = Extension('PyBarry.simple', sources = ['src/PyBarry/simple.cc'], include_dirs=['/usr/include','/opt/local/include'], libraries=['barry'], library_dirs=['/usr/lib','/opt/local/lib'])

module3 = Extension('PyBarry.core', sources = ['src/PyBarry/core.cc'], include_dirs=['/usr/include','/opt/local/include'], libraries=['barry'], library_dirs=['/usr/lib','/opt/local/lib'])

setup (name = 'PyBarry', version = '0.1', description ='Python Module for the Barry Library', ext_modules = [module1, module2,module3], packages=['PyBarry'], package_dir={'PyBarry':'src/PyBarry'})

