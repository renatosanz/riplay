project(
  'riplay',
  'cpp',
  meson_version: '>= 1.3.0',
  version: '1.0',
  license: 'GPL-3.0-or-later',
  default_options: [
    'warning_level=2',
    'werror=false',
    'c_std=gnu11',
  ],
)

src_dir = 'src'
data_dir = 'data'
include_dir = include_directories(['src', 'src/utils', 'src/views', 'src/graphics', 'src/logic'])

m = meson.get_compiler('cpp').find_library('m', required: false)
gtk4 = dependency('gtk4')
gnome = import('gnome')
mpg123_dep = dependency('libmpg123', required: true)
# gst_dep = dependency('gstreamer-1.0', required: true)
# gst_base_dep = dependency('gstreamer-base-1.0', required: true)
# gst_app_dep = dependency('gstreamer-app-1.0', required: true)
# gst_audio_dep = dependency('gstreamer-audio-1.0', required: true)
# libavformat = dependency('libavformat', required: true)
taglib = dependency('taglib_c', required: true)
# sdl2_dep = dependency('sdl2', required: true)

# all .c files in src_dir to be compiled

subdir('src/graphics')
subdir('src/views')
subdir('src/utils')
subdir('src/metadata')
subdir('src/logic')

sources = ['src/main.cpp']
sources += view_sources + utils_sources + graphics_sources + metadata_source + logic_sources

# all blueprint files to gen .xml files (ui)
blueprint_files = run_command('find', 'data/ui', '-name', '*.blp', check: true).stdout().strip().split('\n')

blueprints = custom_target(
  'blueprints',
  input: files(blueprint_files),
  output: '.',
  command: [
    find_program('blueprint-compiler'),
    'batch-compile',
    '@OUTPUT@',
    '@CURRENT_SOURCE_DIR@',
    '@INPUT@',
  ],
)

resources_xml = files(data_dir / 'resources.gresource.xml')
resources = gnome.compile_resources(
  'resources',
  resources_xml,
  dependencies: blueprints,
  source_dir: data_dir,
  c_name: 'resources',
)

dependencies = [
  gtk4,
  m,
  # sdl2_dep,
  mpg123_dep,
  taglib,
  # gst_dep,
  # gst_base_dep,
  # gst_app_dep,
  # gst_audio_dep,
  # dependency('libavformat'),
  # dependency('libavcodec'),
  # dependency('libavutil'),
]

exe = executable(
  'riplay',
  sources,
  resources,
  include_directories: include_dir,
  dependencies: dependencies,
  install: true,
)

test('basic', exe)
