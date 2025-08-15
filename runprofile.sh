#!/bin/bash

(
    start=$(date +%s)
    cd comparison_energy_profile/GDMLoriginal_noCppregion && \
    taskset -c 0 ../../build/gdml_sim ../../TBHGCal181Oct_original.gdml "no_region" ../../gun.mac \
    > log_original_noCppregion.txt 2>&1
    end=$(date +%s)
    echo "Execution time: $((end - start)) seconds" >> log_original_noCppregion.txt
) &

(
    start=$(date +%s)
    cd comparison_energy_profile/GDMLsubregion_noCppregion && \
    taskset -c 1 ../../build/gdml_sim ../../TBHGCal181Oct_subregionSilicon.gdml "no_region" ../../gun.mac \
    > log_subregion_noCppregion.txt 2>&1
    end=$(date +%s)
    echo "Execution time: $((end - start)) seconds" >> log_subregion_noCppregion.txt
) &

(
    start=$(date +%s)
    cd comparison_energy_profile/GDMLnoregions_Cpp_original_cuts && \
    taskset -c 2 ../../build/gdml_sim ../../TBHGCal181Oct_noregions.gdml "original_cuts" ../../gun.mac \
    > log_noregions_original_cuts.txt 2>&1
    end=$(date +%s)
    echo "Execution time: $((end - start)) seconds" >> log_noregions_original_cuts.txt
) &

(
    start=$(date +%s)
    cd comparison_energy_profile/GDMLnoregions_Cpp_new_cuts && \
    taskset -c 3 ../../build/gdml_sim ../../TBHGCal181Oct_noregions.gdml "new_cuts" ../../gun.mac \
    > log_noregions_new_cuts.txt 2>&1
    end=$(date +%s)
    echo "Execution time: $((end - start)) seconds" >> log_noregions_new_cuts.txt
) &

wait
echo "That's all folks!"

