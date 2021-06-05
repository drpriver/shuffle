project('shuffle', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

 add_project_arguments('-march=native', language: 'c')

cc = meson.get_compiler('c')

if cc.get_id() == 'clang'
  add_project_arguments(
      '-Wno-nullability-extension', # We guard this with an ifdef so it's fine.
      language : 'c')
endif

executable('shuffle',
           'shuffle.c',
           install : true)
