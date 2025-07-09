project(
  'riplay',
  'c',
  meson_version : '>= 1.3.0',
  version : '0.1',
  license: 'GPL-3.0-or-later',
  default_options: [
    'warning_level=2',
    'werror=false',
    'c_std=gnu11',
  ],
)

src_dir = 'src'
data_dir = 'data'
include_dir = include_directories('include')


m = meson.get_compiler('c').find_library('m', required: false)
gtk4 = dependency('gtk4')
gnome = import('gnome')
mpg123_dep = dependency('libmpg123', required : true)
sdl2_dep = dependency('sdl2', required : true)

# all .c files in src_dir to be compiled 
sources = run_command(
  'find', src_dir, '-name', '*.c', 
  check: true
).stdout().strip().split('\n')

# all blueprint files to gen .xml files (ui)
blueprint_files = run_command(
  'find', 'data/ui', '-name', '*.blp', 
  check: true
).stdout().strip().split('\n') 

blueprints = custom_target('blueprints',
  input: files(blueprint_files),
  output: '.',
  command: [
    find_program('blueprint-compiler'),
    'batch-compile',
    '@OUTPUT@',
    '@CURRENT_SOURCE_DIR@',
    '@INPUT@'
  ],
)

resources_xml = files(data_dir / 'resources.gresource.xml')
resources = gnome.compile_resources(
  'resources',
  resources_xml,
  dependencies: blueprints,
  source_dir: data_dir,
  c_name: 'resources'
)

dependencies = [
  gtk4,m,sdl2_dep,mpg123_dep
]

exe = executable(
  'riplay',
  sources,
  resources,
  include_directories: include_dir,
  dependencies : dependencies,
  install : true,
)

test('basic', exe)
