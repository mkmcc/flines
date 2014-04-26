(* project a list of 3D points into the 2D image plane 

   -- takes a list of 3D *points* and returns a list of pairs {r, x},
      where r is the 3D distance from the viewer and x is the 2D
      location in the image plane.

   -- this accounts for perspective; `dist' is the distance from the
      camera to the origin in units of the focal length.

   -- this returns pairs {r, x} so you can sort the x vectors by
      distance before plotting them.  dray the closer ones last so
      they come out on top
 *)
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


(* image a 3D trajectory (e.g., an orbit or a field line)

   -- take a list of 3D points and returns a list of projected line
      segments.  the width and color both scale with the 3D distance
      to the viewer.

   -- the 3D distance is also prepended so the segments can be sorted
      later.  the closer ones should be plotted last.  sort later in
      case you want to plot multiple orbits or field lines.
*)
image[dist_, alpha_, beta_, pts_, color_] :=
    Module[{newpts, rvals},
           newpts = project[dist, alpha, beta, pts];
           newpts = Partition[newpts, 2, 1]; (* points -> line segments *)
           (* newpts now looks like this: 
              {{{r1, v1}, {r2, v2}}, 
               {{r2, v2}, {r3, v3}},
               ... }
              where r is the distance and v is the 2D vector in the image plane.
           *)

           (* transposing an element of newpts looks like this:
              {{r1, r2}, {v1, v2}}.
              I use this form below
           *)
           rvals = Map[Mean[First[Transpose[#]]] &, newpts];
           Map[Apply[
               With[{r = Mean[#1]},
                    {r,                                     (* r prepended for sorting *)
                     Thickness[fact*(Max[rvals]/r)^(expt)], (* thickness scales with r *)
                     Blend[{color, Black},                  (* darken with r *)
                           0.0 + 0.75 (r - Min[rvals])/(Max[rvals] - Min[rvals] + 0.0001)],
                     Line[#2]}] &,                          (* line segment  *)
               Transpose[#]] &, newpts]]



(* plot a set of trajectories

   -- take a list of 3D trajectories (e.g., orbits or field lines) and
      return an plot

   -- `orbits' is a list of trajectories; each trajectory is a list of
      3D points

   -- the camera points toward the origin from the angles `alpha' and
      `beta'.  the distance to the camera is `dist' in units of the
      focal length

   -- you can specify a default color; if so, all trajectories get
      that color.  otherwise, they are colored from blue to white to
      red ("TemperatureMap") in the order they are given.
*)
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


(* make a set of "orbits" which trace out a frame for the plot *)
frame = Tuples[Tuples[{1, -1}, 3], 2];
frame = Select[frame, Norm[First[#] - Last[#]] == 2.0 &];


(* fancyplot: show a frame (thin, black, no perspective), then plot
     the orbits on top of it.  

   -- see `project' or `plotproject' for the meaning of `alpha',
      `beta', and `dist'

   -- orbit is any list of lists of 3D points.  see `plotprojcet' for
      details.  
*)
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


(* mkfig: reads in a data file and saves the plot in a png image.

   -- everything before this function is general and should work with
      any list of lists of 3D points.  this is more specific to the
      "flines" project.

   -- the data file should have the same format read by gnuplot's
      `splot' command.  each line has the x, y, and z coordinates of a
      point along a trajectory.  trajectories are separated by blank
      lines.
 *)
mkfig[fname_, new_] :=
    Module[{data = Import[fname, "Table"], ndata, orbits},
           
           ndata = Split[data, Length[#] == 3 &];
           ndata = Map[DeleteCases[#, {}] &, ndata];
           ndata = DeleteCases[ndata, {}];
           orbits = ndata;

           Export[new,
                  Show[fancyplot[3.0, -1.2 Pi, 0.6, orbits], ImageSize->768]]]


(* maybemkfig: calls mkfig only if needed
 *)
maybemkfig[fname_] :=
    With[{new = fname <> ".png"},
        If[Not[FileExistsQ[new]], mkfig[fname, new]]]

Map[maybemkfig, FileNames["*.flines"]]

(* Local Variables: *)
(* mode: mathematica *)
(* End: *)
