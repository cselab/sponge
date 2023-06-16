# Geometry

[ridges.stl](ridges.stl): R1 from [1]

# Basilisk

Follow <http://basilisk.fr/src/INSTALL>

```
sudo apt install libglu1-mesa-dev libosmesa6-dev meshlab -y
cd basilisk/src
cp config.gcc config
make
cp qcc $HOME/.local/bin/
cd basilisk/src/gl
make libglutils.a libfb_osmesa.a
```

# References

1. Falcucci, G., Amati, G., Fanelli, P., Krastev, V. K., Polverino,
G., Porfiri, M., & Succi, S. (2021). Extreme flow simulations reveal
skeletal adaptations of deep-sea sponges. Nature, 595(7868), 537-541.
[doi:10.1038/s41586-021-03658-1](https://doi.org/10.1038/s41586-021-03658-1)
[repo](https://github.com/giacomofalcucci/Euplectella_HPC)
