#!/bin/bash
set -e  # stop if something goes bananas


taskset -c 0 hyperfine -r 3 --command-name GDMLoriginal_noCppregion --export-markdown GDMLoriginal_noCppregion.md \
  './build/gdml_silentsim TBHGCal181Oct_original.gdml "no_region" gun.mac > /dev/null 2>&1' &

taskset -c 1 hyperfine -r 3 --command-name GDMLsubregion_noCppregion --export-markdown GDMLsubregion_noCppregion.md \
  './build/gdml_silentsim TBHGCal181Oct_subregionSilicon.gdml "no_region" gun.mac > /dev/null 2>&1' &

taskset -c 2 hyperfine -r 3 --command-name GDMLnoregions_Cpp_original_cuts --export-markdown GDMLnoregions_Cpp_original_cuts.md \
  './build/gdml_silentsim TBHGCal181Oct_noregions.gdml "original_cuts" gun.mac > /dev/null 2>&1' &

taskset -c 3 hyperfine -r 3 --command-name GDMLnoregions_Cpp_new_cuts --export-markdown GDMLnoregions_Cpp_new_cuts.md \
  './build/gdml_silentsim TBHGCal181Oct_noregions.gdml "new_cuts" gun.mac > /dev/null 2>&1' &

wait
