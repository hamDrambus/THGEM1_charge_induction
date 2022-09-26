version legend:
v[geometry version].[meshing version].[elmer solution version]
v00.x.x -  0.5mm thickness 28% THGEM0, dl from THGEM to cathode/anonde == 3mm
vx.01.x - normal meshing, finer mesh near TGHEM0 hole
vx.x.01 - V0 = 20kV (E/N EL = 8.35 Td), V1(on THGEM1 divider) = 2650 V

v00.00:
==================================================================================================
+-----------+        .  .      +------+.......+     ----.
|           |          /|\     |------|-------|  .  ____| t
+___.        \          |      |      |-    - | / \ 
     \        \         |a     |      |_  -  _|  T
      \         *____+  |  X   |______|___-___| \./_____.
       *             |  |      +______+.......+    _____| t
       |             |  |
.      +_____________+ \./
<--------a┐/3'--------->
			Y
28% Optical transparency
r = 0.25 mm
R = 0.35 mm
a = P/2 = 0.45 mm
t = 0.03 mm
T = 0.4 mm - FR4
dz_bot = 3 mm - the distance from the bottom THGEM1 surface to the bottom plane creating the external field (cathode)
dz_top = 3 mm - the distance from the top THGEM1 surface to the top plane creating the external field (anode)
==================================================================================================
Surfaces:
Cathode (Z-): 1
Anode (Z+): 2
X+ LAr: 3	X+ diel: 7
X- LAr: 4	X- diel: 8
Y- LAr: 5	Y- diel: 9
Y+ LAr: 6	Y+ diel: 10
Cu top: 11, 12, 13, 14, 15, 16, 17, 18
Cu bot: 19, 20, 21, 22, 23, 24, 25, 26

Volumes:	1 - LAr	
		2 - dielectric				
		3 - top Cu
		4 - bot Cu
e_gas_Ar = 1.0
e_LAr = 1.54
e_FR4 (fiber glass) = 4.4
e_acrylic = 3.6

-----------------------			0 kV	 	 Ground
		5mm						  ______		.
						 	 ____|__R4__|__o V1 (+)
`|   |```|   |```|   |`  <- .|.	 1.2 MOhm		.
--| |-----| |-----| |--  	| |					. THGEM1
_|   |___|   |___|   |_  	|_|R5 = 8.6 MOhm	.
						 <-	 |    ______		.
						 	 |___|__R6__|__o Ground
		9.2 mm gap 				  1 MOhm		.
												.
	(22 - 9.2) mm of LAr    	 ______			.
~~~~~~~~~~~~~~~~~~~~~~~     ____|__R4__|__o	 Ground
~	~	~  	~   ~   ~	 <-.|.	 600 MOhm		.
`| ~ |```|   |```| ~ |`    | |					.
--| |-----| |-----| |--    |_|R3 = 0 Ohm(mesh)	. THGEM0 or mesh
_|   |___| ~ |___|   |_  <- |					.
~     ~		~			   .|.					.
	~	~				   | |3R2 = 120 MOhm	.
   ~	     ~     ~	   |_|   ______			.
-----------------------   <--|__|__R1__|__o  V0 (-)
			     				80 MOhm			.


==================================================================================================
==================================================================================================
==================================================================================================
v00.01 - same as v00.00, but with meshing parameters:
Field[1] = Box;
Field[1].VIn = 0.10;
Field[1].VOut = 10;
Field[1].XMax = 1;
Field[1].XMin = -1;
Field[1].YMax = 1;
Field[1].YMin = -1;
Field[1].ZMax = 5;
Field[1].ZMin = -5;
//+
Field[2] = Box;
Field[2].VIn = 0.015;
Field[2].VOut = 10;
Field[2].XMax = 1;
Field[2].XMin = -1;
Field[2].YMax = 1;
Field[2].YMin = -1;
Field[2].ZMax = 0.3;
Field[2].ZMin = -0.3;
//+
Field[3] = Box;
Field[3].VIn = 0.03;
Field[3].VOut = 10;
Field[3].XMax = 1;
Field[3].XMin = -1;
Field[3].YMax = 1;
Field[3].YMin = -1;
Field[3].ZMax = 0.5;
Field[3].ZMin = -0.5;

Field[4] = Min;
Field[4].FieldsList = {1, 2, 3};

Background Field = 4;

Elmer v01:
	V0 = 20kV
	V1 = 2650V  Vthgem1 = 2650*8.6/10.8 = 2110 V
	//field below THGEM1
	Edrift = (20 kV * 600/800 + 2650 V * 1/10.8) * 1.54/(1.54*0.92 cm + (2.2-0.92) cm) = 871 V/mm
	Vcathode = -871 * 3mm = -2613 V;
	V_bot_THGEM1 = 0 V
	V_top_THGEM1 = 2650 * 8.6/10.8 = +2110 V
	//field above THGEM1
	Eemission = 2650 * 9.6/10.8 / 5 mm = 471.1 V/mm
	V_anode = V_top_THGEM1 - 471.1 *3 = +696.7 V


