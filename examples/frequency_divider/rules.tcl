#Layer definitions
layer NW
layer NP
layer PP
layer M1
layer M1_PIN
layer VIA1
layer M2
layer M2_PIN
layer VIA2
layer M3
layer M3_PIN
layer VIA3
layer M4
layer M4_PIN
layer PO
layer OD
layer CO
layer PDK
layer VTL_N
layer VTL_P
layer prBoundary

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