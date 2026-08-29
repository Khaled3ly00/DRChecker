rule min_spacing -layer NW -value 0.47
rule min_width -layer NW -value 0.47
rule min_enclosure -inner NP -outer NW -value 0.16

rule min_spacing -layer PO -value 0.06
rule min_width -layer PO -value 0.12
rule min_enclosure -inner PO -outer NP -value 0.15
rule min_enclosure -inner PO -outer PP -value 0.15

rule min_spacing -layer NP -value 0.18
rule min_width -layer NP -value 0.18

rule min_spacing -layer CO -value 0.11
rule min_width -layer CO -value 0.09
rule min_enclosure -inner CO -outer PO -value 0.01
rule min_enclosure -inner CO -outer OD -value 0.015
rule min_enclosure -inner CO -outer M1 -value 0.025

rule min_spacing -layer M1 -value 0.09
rule min_width -layer M1 -value 0.09

rule min_spacing -layer M2 -value 0.1
rule min_width -layer M2 -value 0.1

rule min_spacing -layer VIA1 -value 0.1
rule min_width -layer VIA1 -value 0.1

rule min_enclosure -inner VIA1 -outer M1 -value 0.03


#rule density -layer M1 -limit minimum -value 0.10 -window_size 75 -window_step 37.5
#rule density -layer M1 -limit minimum -value 0.8 -window_size 100 -window_step 50