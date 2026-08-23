set script_dir [file dirname [file normalize [info script]]]

set layout1 "$script_dir/cli_multiple_shapes_layout.json"
set layout2 "$script_dir/cli_layout.json"
set rules "$script_dir/rules.tcl"

set report1 "$script_dir/automation_report1.json"
set report2 "$script_dir/automation_report2.json"

drc_run -layout $layout1 \
        -rules $rules \
        -report $report1

if {[drc_error_count] != 4} {
    error "Unexpected violation count after first DRC run"
}

drc_run -layout $layout2 \
        -rules $rules \
        -report $report2

if {[drc_error_count] != 0} {
    error "Unexpected violation count after second DRC run"
}