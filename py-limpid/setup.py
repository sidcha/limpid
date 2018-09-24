from distutils.core import setup, Extension

source_list = [
    'py-limpid.c',
    '../src/limpid-core.c',
    '../src/limpid-json.c',
    '../src/lib-json.c',
    '../src/lib-string.c',
    '../src/lib-read-line.c',
    '../src/limpid-cli.c'
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
