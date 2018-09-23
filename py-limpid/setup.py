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

setup(name='limpid',
      version='1.0',
      ext_modules=[Extension('limpid', source_list, extra_compile_args=[ '-I../include' ])])
