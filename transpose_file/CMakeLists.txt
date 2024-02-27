project(transpose_file LANGUAGES C)

add_executable(transpose_file src/main.c)
strip_section_headers(transpose_file)
