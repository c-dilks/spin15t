


IMPORTANT NOTE: the code in this directory was copied from trgmon15 and modified
for RP use



Procedure
---------
These scripts pull trigger threshold information from trigger version webpage
and builds a table of run numbers together with corresponding trigger 
thresholds; this table is named `thresh.dat`

1. Obtain `runlog.root` file from trgmon results

2. Execute `RunIndex.C` to generate a run index file, `index.dat`
   - The columns are `[ run# | index# | sqrt(s) | download boolean ]`
   - To do this, run `RunIndex.C("runlog.root",sqrt(s))`
   - `doubleRunIndex` will do this for two separate root files,
     `runlog200.root` and `runlog500.root` (from `trgmon12`) and
     concatenate an `index.dat` file
   - download boolean is `true` if info needs to be downloaded

3. Execute `loopThresh`
 - This will build `thresh.dat`, a file containing trigger thresholds
   for the run numbers listed in `index.dat`
 - This runs `triggerThresholds` and `appendThreshFile` (see subroutines below)
   for all runs listed in `index.dat`; in principle, to update plots, one
   can generate a new `index.dat` file containing new runs and compare it to
   the old one so that only new threshold information is downloaded; then
   a new `thresh.root` file can be created and hadded to the old one
 - temporary `trg_*` and `th_*` files used in `triggerThresholds` will be then
   relocated to subdirectories, but are probably not useful after building
   `thresh.dat` and can be deleted 
 - Columns of `thresh.dat` (left to right) are given in `appendThreshFile`
