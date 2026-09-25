# write_liberty_binary records the source file and its hash for traceability.
source helpers.tcl

# Report an error message with the results directory stripped.
proc puts_error { msg } {
  global result_dir
  puts [string map [list "$result_dir/" ""] $msg]
}

# FNV-1a 64-bit hash of a file's bytes, as 16 hex digits.
proc file_hash { filename } {
  set stream [open $filename rb]
  set bytes [read $stream]
  close $stream
  binary scan $bytes cu* byte_list
  set hash 14695981039346656037
  foreach byte $byte_list {
    set hash [expr { (($hash ^ $byte) * 1099511628211) & 0xFFFFFFFFFFFFFFFF }]
  }
  return [format %08x%08x [expr { $hash >> 32 }] [expr { $hash & 0xFFFFFFFF }]]
}

set blib_file [make_result_file liberty_binary_source.blib]
write_liberty_binary liberty_float_as_str.lib $blib_file
set info [liberty_binary_info $blib_file]
puts "version: [dict get $info version]"
set source_file [dict get $info source_file]
puts "source file: [file tail $source_file]"
puts "source file is absolute: [expr { [file pathtype $source_file] eq "absolute" }]"
puts "source file exists: [file exists $source_file]"
puts "source hash matches: [expr { [dict get $info source_hash] eq [file_hash liberty_float_as_str.lib] }]"

# A gzipped source is hashed as stored on disk.
set gz_blib [make_result_file liberty_binary_source_gz.blib]
write_liberty_binary ../examples/asap7_small_ff.lib.gz $gz_blib
set gz_info [liberty_binary_info $gz_blib]
puts "gzipped source file: [file tail [dict get $gz_info source_file]]"
puts "gzipped source hash matches: [expr { [dict get $gz_info source_hash] eq [file_hash ../examples/asap7_small_ff.lib.gz] }]"

# Only binary liberty files have a header to report.
catch { liberty_binary_info liberty_float_as_str.lib } result
puts_error $result
catch { liberty_binary_info [make_result_file does_not_exist.blib] } result
puts_error $result
