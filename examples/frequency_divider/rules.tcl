#Layer definitions
layer NW        -map {-layer 1  -datatype 0}
layer NP        -map {-layer 2  -datatype 0}
layer PP        -map {-layer 3  -datatype 0}
layer M1        -map {-layer 4  -datatype 0}
layer M1_PIN    -map {-layer 5  -datatype 0}
layer VIA1      -map {-layer 6  -datatype 0}
layer M2        -map {-layer 7  -datatype 0}
layer M2_PIN    -map {-layer 8  -datatype 0}
layer VIA2      -map {-layer 9  -datatype 0}
layer M3        -map {-layer 10 -datatype 0}
layer M3_PIN    -map {-layer 11 -datatype 0}
layer VIA3      -map {-layer 12 -datatype 0}
layer M4        -map {-layer 13 -datatype 0}
layer M4_PIN    -map {-layer 14 -datatype 0}
layer PO        -map {-layer 15 -datatype 0}
layer OD        -map {-layer 16 -datatype 0}
layer CO        -map {-layer 17 -datatype 0}
layer PDK       -map {-layer 18 -datatype 0}
layer VTL_N     -map {-layer 19 -datatype 0}
layer VTL_P     -map {-layer 20 -datatype 0}
layer prBoundary -map {-layer 21 -datatype 0}

#Rule definitions
rule min_spacing -layer NW -value 0.47
rule min_width -layer NW -value 0.47

rule min_spacing -layer PO -value 0.12
rule min_width -layer PO -value 0.06
rule min_enclosure -inner PO -options {{-outer PP -all_sides 0.15} {-outer NP -all_sides 0.15}}

rule min_spacing -layer NP -value 0.18
rule min_width -layer NP -value 0.18

rule min_spacing -layer CO -value 0.11
rule min_width -layer CO -value 0.09
rule min_enclosure -inner CO -options {{-outer PO -all_sides 0.03 -first_pair 0.01 -second_pair 0.04} {-outer OD -all_sides 0.03 -first_pair 0.015 -second_pair 0.03}}
rule min_enclosure -inner CO -options {{-outer M1 -all_sides 0.25 -first_pair 0.00 -second_pair 0.04}}

rule min_spacing -layer M1 -value 0.09
rule min_width -layer M1 -value 0.09

rule min_spacing -layer M2 -value 0.1
rule min_width -layer M2 -value 0.1

rule min_spacing -layer VIA1 -value 0.1
rule min_width -layer VIA1 -value 0.1

rule min_enclosure -inner VIA1 -options {{-outer M1 -all_sides 0.3 -first_pair 0.00 -second_pair 0.04}}

#rule density -layer M1 -limit minimum -value 0.10 -window_size 75 -window_step 37.5
#rule density -layer M1 -limit minimum -value 0.8 -window_size 100 -window_step 50