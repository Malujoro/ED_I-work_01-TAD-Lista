project('teste', 'c')

add_global_arguments( 
    [
        '-DSOME_TOKEN=value',
        '-g'
    ], language: 'c'
)

gtkdep = dependency('gtk4')

cc = meson.get_compiler('c')
py3_dep = cc.find_library('python3.12', required: true)
py3_include = include_directories('/usr/include/python3.12')
local_include = include_directories('.')

executable('main', ['main.c', 'image.c'], dependencies : [gtkdep, py3_dep], include_directories: [py3_include, local_include])