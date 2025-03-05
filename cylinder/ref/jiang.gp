set term pngcairo size 1422, 597
set output "jiang.png"
set key off
set xlabel "time"
set ylabel "force coefficients, Re = 240"
set macro
s = 'w l lw 2 lc rgb'
plot [:480][-1:1.5] \
     "<sort -k 2 -g 240/0.dat" u 2:($3/25) @s "#000000", \
     "" u 2:($4/25) @s "#000000", \
     -0.76 @s "#000000" t "", \
     0.76 @s "#000000" t "", \
     1.3 @s "#000000" t "", \
     1.4 @s "#000000" t ""
