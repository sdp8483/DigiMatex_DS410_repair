(G-CODE GENERATED BY FLATCAM vUnstable - www.flatcam.org - Version Date: 2022/3/1)

(Name: 1.0mm.drl_cnc)
(Type: G-code from Excellon)
(Units: MM)

(Created on Sunday, 06 February 2022 at 19:09)

(This preprocessor is the default preprocessor used by FlatCAM.)
(It is made to work with MACH3 compatible motion controllers.)


(TOOLS DIAMETER: )
(Tool: 1 -> Dia: 1.0007599999999999)

(FEEDRATE Z: )
(Tool: 1 -> Feedrate: 90.0)

(FEEDRATE RAPIDS: )
(Tool: 1 -> Feedrate Rapids: 1500)

(Z_CUT: )
(Tool: 1 -> Z_Cut: -2.0)

(Tools Offset: )
(Tool: 1 -> Offset Z: 0.0)

(Z_MOVE: )
(Tool: 1 -> Z_Move: 2)

(Z Start: None mm)
(Z End: 15 mm)
(X,Y End: None mm)
(Steps per circle: 16)
(Preprocessor Excellon: default)

(X range: -117.4013 ...  -15.3137  mm)
(Y range:  -38.5013 ...  -12.5984  mm)

(Spindle Speed: 300 RPM)
G21
G90
G94

G01 F90.00
G00 Z2.0000
M03 S300
G00 X-15.8140 Y-38.0009
G01 Z-2.0000
G01 Z0
G00 Z2.0000
G00 X-116.9010 Y-13.0988
G01 Z-2.0000
G01 Z0
G00 Z2.0000
G00 X-114.3610 Y-13.0988
G01 Z-2.0000
G01 Z0
G00 Z2.0000
G00 X-111.8210 Y-13.0988
G01 Z-2.0000
G01 Z0
G00 Z2.0000
G00 X-105.4557 Y-13.0988
G01 Z-2.0000
G01 Z0
G00 Z2.0000
G00 X-102.9157 Y-13.0988
G01 Z-2.0000
G01 Z0
G00 Z2.0000
G00 X-100.3757 Y-13.0988
G01 Z-2.0000
G01 Z0
G00 Z2.0000
G00 X-20.8153 Y-38.0009
G01 Z-2.0000
G01 Z0
G00 Z2.0000
M05
G00 Z15.00

