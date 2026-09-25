# Binary liberty files are recognized by their contents.
source helpers.tcl

set renamed_file [make_result_file liberty_binary_detect.lib]
write_liberty_binary liberty_float_as_str.lib $renamed_file
read_liberty $renamed_file
report_units
puts "cells: [llength [get_lib_cells *]]"

# A text liberty file named .blib is read as text.
set text_blib [make_result_file liberty_binary_detect_text.blib]
file copy -force liberty_backslash_eol.lib $text_blib
read_liberty $text_blib
puts "text cells: [llength [get_lib_cells liberty_backslash_eol/*]]"
