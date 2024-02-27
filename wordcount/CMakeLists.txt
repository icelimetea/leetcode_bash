project(wordcount LANGUAGES C)

include_directories(include libs)

add_executable(wordcount src/dict.c src/util.c src/main.c)
strip_section_headers(wordcount)
