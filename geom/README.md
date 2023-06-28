# Geometry

```
*From: *<timon.meier@berkeley.edu>
*Subject: **Sponge Example Geometry*
*Date: *May 9, 2023 at 00:37:28 EDT
*To: *"'Koumoutsakos, Petros'" <petros@seas.harvard.edu>, "'Pascal Weber'" <webepasc@seas.harvard.edu>, "'Costas P. GRIGOROPOULOS'" <cgrigoro@berkeley.edu>, "Jacky Li" <runxuan.li@berkeley.edu>

Hi all,
As discussed, attached the code for a sample geometry for the deep-sea sponge that we use in Ansys to perform mechanical simulations.
```

`geom.py` from ChatGPT:

> The code you provided appears to be written in a finite element analysis
> software called ANSYS APDL (ANSYS Parametric Design Language). It is used for
> performing simulations and analyses in various engineering fields. The code
> seems to define parameters and then create a three-dimensional structure
> consisting of vertical beams and circumferential beams.


```
from:	Timon Meier <timon.meier@berkeley.edu>
to:	Sergey Litvinov <slitvinov@gmail.com>
date:	Jun 28, 2023, 4:14 AM
subject:	Sponge Geometry

Hi Sergey,

Attached a jupyter notebook and python file that we use to describe
the sponge geometry. The geometry creation works in python even if
only the free Ansys student version is installed in case this is
relevant at one point.

I also attached a pdf describing step by step what happens in which
code lines and can add more details if needed.

 
Note: The geometry plots only show lines but do not represent the
actual cross section. As you can see on the snipped below some beam
elements have a rectangular cross section while the large spiral has
an ellipsoidal cross section (which is not visible in the PyAnsys
plots but considered in the FEA model).

I will also update and share the github the next days.  Let me know
what you think about the format, and I am happy to discuss how to move
forward and how to automate the integration of the geometry into the
CFD simulations.
```