project(phone_numbers LANGUAGES C)

add_executable(phone_numbers src/main.c)
strip_section_headers(phone_numbers)
