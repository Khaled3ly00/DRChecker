layer M1
layer VIA1

rule min_width -layer M1 -value 3
rule min_spacing -layer M1 -value 2
rule min_enclosure -inner VIA1 -outer M1 -value 1
rule density -layer M1 -limit minimum -value 0.30 -window_size 10 -window_step 5