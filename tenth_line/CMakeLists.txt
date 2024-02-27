project(tenth_line LANGUAGES C)

add_executable(tenth_line src/main.c)
strip_section_headers(tenth_line)
