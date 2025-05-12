src = [
  'buffer.c',
  'image.c',
  'layer.c',
  'list.c',
  'model.c',
]

fig_lib = shared_library(
  'fig',
  src,
  include_directories: inc_dir,
  dependencies: [libjpeg_dep, m_dep],
)
