project('fig', ['c', 'cpp'])

cc = meson.get_compiler('c')

libjpeg_dep = dependency('libjpeg')
opencv_dep = dependency('opencv4')
m_dep = cc.find_library('m')

inc_dir = include_directories('include')

subdir('src')
subdir('demos')
