(* project a single orbit *)
(* -- takes a list of 3D *points* and returns a list of pairs {r, x}, *)
(*    where r is the 3D distance from the viewer and x is the 2D *)
(*    location in the image plane. *)
project[dist_, alpha_, beta_, pts_] :=
    With[{
        (* location of the camera (assumed to be looking at the origin) *)
        camera = dist {Cos[beta] Cos[alpha], Cos[beta] Sin[alpha], Sin[beta]},
        (* mat1 rotates through -beta about the y axis *)
        mat1 = {
            {Cos[-beta], 0, -Sin[-beta]},
            {0, 1, 0},
            {Sin[-beta], 0, Cos[-beta]}},
        (* mat2 rotates through -alpha about the z axis *)
        mat2 = {
            {Cos[-alpha], -Sin[-alpha], 0},
            {Sin[-alpha], Cos[-alpha], 0},
            {0, 0, 1}}},
         (* loop over points: *)
         (* - first rotate through z, then y, to align x-axis with camera axis. *)
         (* - then project onto y-z plane and scale like 1/r. *)
         Map[Block[{v1 = mat1.mat2.#, v2 = mat1.mat2.camera, r},
                   r = Abs[First[v1 - v2]];
                   {r, (1/r) {{0, 1, 0}, {0, 0, 1}}.v1}] &, pts]]

(* image the orbit *)
(* -- take a list of 3D points and returns a list of projected line *)
(*    segments.  the width and color both scale with the 3D distance *)
(*    to the viewer.  *)
(* -- the 3D distance is also prepended so the segments can be sorted *)
(*    later.  the closer ones should be plotted last. *)
image[dist_, alpha_, beta_, pts_, color_] :=
    Module[{newpts, rvals},
           newpts = project[dist, alpha, beta, pts];
           newpts = Partition[newpts, 2, 1];
           rvals = Map[Mean[First[Transpose[#]]] &, newpts];
           Map[Apply[
               With[{r = Mean[#1]},
                    {r,
                     Thickness[fact*(Max[rvals]/r)^(expt)],
                     Blend[{color, Black}, 
                           0.0 + 0.75 (r - Min[rvals])/(Max[rvals] - Min[rvals] + 0.0001)],
                     Line[#2]}] &,
               Transpose[#]] &, newpts]]

plotproject[dist_, alpha_, beta_, orbits_, defaultcolor_: False] :=
    Block[{pts, colors},
          colors = Table[defaultcolor, {i, Length[orbits]}];
          If[defaultcolor == False,
             colors = Table[ColorData["TemperatureMap"][x], 
                            {x, 0.0, 1.0, 1.0/(Length[orbits] - 1)}]];
          pts = MapThread[
              image[dist, alpha, beta, #1, #2] &, {orbits, colors}];

          (* sort by 1/r so closer things are plotted last *)
          pts = Flatten[pts, 1];
          pts = SortBy[pts, 1.0/First[#] &];

          (* drop the r's, then convert to a graphics object *)  
          pts = Map[Drop[#, 1] &, pts];
          Graphics[pts]]

(* make a set of "orbits" that trace out a box to use in the plots below *)
frame = Tuples[Tuples[{1, -1}, 3], 2];
frame = Select[frame, Norm[First[#] - Last[#]] == 2.0 &];


fancyplot[dist_, alpha_, beta_, orbits_] :=
    Show[{
        Block[{},
              fact = 0.001;
              expt = 0.0;
              plotproject[dist, alpha, beta, frame, Black]],
        Block[{},
              fact = 0.006;
              expt = 5.0;
              plotproject[dist, alpha, beta, orbits]]}]

mkfig[fname_] :=
    Module[{data = Import[fname, "Table"], ndata, orbits},
           
           ndata = Split[data, Length[#] == 3 &];
           ndata = Map[DeleteCases[#, {}] &, ndata];
           ndata = DeleteCases[ndata, {}];
           orbits = ndata;

           Export[fname <> ".png",
                  Show[fancyplot[3.0, -1.2 Pi, 0.6, orbits], ImageSize->768]]]

If[Length[ARGV] < 2,
  Print["usage: mash " <> ARGV[[1]] <> " file1 [file2 file3 ...]"];
  Exit[1]];

Map[mkfig, Drop[ARGV,1]]

(* Local Variables: *)
(* mode: mathematica *)
(* End: *)
