from distutils.core import setup, Extension

source_list = [
    'py-limpid.c',
    '../helpers/json.c',
    '../helpers/string.c',
    '../helpers/read-line.c',
    '../src/core.c',
    '../src/json.c',
    '../src/cli.c'
]

ext = Extension(
    'limpid',
    source_list,
    extra_compile_args=[ '-I../include' ]
)

setup(
    name='limpid',
    version='1.0',
    ext_modules=[ext]
)
