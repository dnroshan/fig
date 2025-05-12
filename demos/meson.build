executable(
  'run',
  'run.c',
  link_with: fig_lib,
  include_directories: inc_dir,
  link_args: '-lm'
)

executable(
  'run_opencv',
  'run_opencv.cpp',
  include_directories: inc_dir,
  dependencies: [opencv_dep],
  link_args: '-lm',
  link_with: [fig_lib],
)
