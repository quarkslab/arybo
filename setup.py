from setuptools import setup

setup(
    name="arybo",
    version="1.1.0",
    author="Adrien Guinet",
    author_email="aguinet@quarkslab.com",
    description="Manipulation, canonicalization and identification of mixed boolean-arithmetic symbolic expressions",
    packages=["arybo", "arybo.lib", "arybo.tools"],
    install_requires=[
        "networkx==2.4",
        "pytanque >= 1.0",
        "six"
    ],
    classifiers=[
	'Development Status :: 5 - Production/Stable',
	'Intended Audience :: Science/Research',
	'Intended Audience :: Developers',
	'Topic :: Software Development :: Build Tools',
	'Topic :: Security',
	'Topic :: Scientific/Engineering',
	'License :: OSI Approved :: BSD License',
	'Programming Language :: Python :: 3',
	'Programming Language :: Python :: 3.5',
	'Programming Language :: Python :: 3.6',
	'Programming Language :: Python :: 3.7',
	'Programming Language :: Python :: 3.8',
    ],
    keywords='symbolic computation canonicalization boolean-arithmetic expressions',
    license='BSD',
    url = "https://github.com/quarkslab/arybo",
    download_url = "https://github.com/quarkslab/arybo/releases",
    scripts=["bin/iarybo"]
)
