project('shuffle', 'c',
  version : '1.0',
  default_options : ['warning_level=3'])

cc = meson.get_compiler('c')

if cc.get_id() == 'clang'
  add_project_arguments(
      '-Wno-nullability-extension', # We guard this with an ifdef so it's fine.
      language : 'c')
endif

if host_machine.system() == 'windows'
  add_project_arguments(
    '-D_CRT_SECURE_NO_DEPRECATE=1',
    language: 'c')
endif

executable('shuffle',
           'shuffle.c',
           install : true)
