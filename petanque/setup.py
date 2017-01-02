from setuptools import setup, Extension
import glob
import platform
import subprocess

if platform.system() in ("Linux","Darwin"):
    # This will work w/ GCC and clang
    compile_args = ['-std=c++1y','-Wno-maybe-uninitialized','-flto','-DPA_DISTUTILS']
    link_args = ['-flto']
elif platform.system() == "Windows":
    compile_args = ['/DPA_DISTUTILS','/TP', '/EHsc']
    link_args = []
else:
    raise RuntimeError("unsupported platform '%s'!" % os.platform)

module = Extension('pytanque',
                    include_dirs = ['bindings/python/distutils_pa_config','include','third-party'],
                    extra_compile_args = compile_args,
                    extra_link_args = link_args,
                    libraries = [],
                    library_dirs = [],
                    sources = glob.glob('src/*.cpp') + glob.glob('bindings/python/*.cpp'))

setup (name = 'pytanque',
    version = '0.4.0',
    description = 'petanque static python bindings',
    author = 'Adrien Guinet',
    author_email = 'aguinet@quarkslab.com',
    long_description = '''
petanque static python bindings. libpetanque is statically compiled within the
extension for an easy setup. See the documentation for compiling the python
extension by hand to link with an external shared library for libpetanque.
''',
    classifiers=[
	'Development Status :: 4 - Beta',
	'Intended Audience :: Science/Research',
	'Intended Audience :: Developers',
	'Topic :: Software Development :: Build Tools',
	'Topic :: Security',
	'Topic :: Scientific/Engineering',
	'License :: OSI Approved :: BSD License',
	'Programming Language :: Python :: 3',
	'Programming Language :: Python :: 3.2',
	'Programming Language :: Python :: 3.3',
	'Programming Language :: Python :: 3.4',
	'Programming Language :: Python :: 3.5'
    ],
    keywords='symbolic computation canonicalization boolean-arithmetic expressions',
    license='BSD',
    url = "https://github.com/quarkslab/arybo",
    download_url = "https://github.com/quarkslab/arybo/releases",
    ext_modules = [module])
