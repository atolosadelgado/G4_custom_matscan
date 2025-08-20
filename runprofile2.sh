#!/bin/bash

# Path to your application
APP=./build/gdml_sim

# Define the runs (each entry: "geometry cut action physics gun")
runs=(
  "RunD121_HGCal.gdml original_cuts profile FTFP_BERT_EMZ gun_fulldet.mac"
  "RunD121_HGCal.gdml new_cuts profile FTFP_BERT gun_fulldet.mac"
  "TBHGCal181Oct_original.gdml no_cuts profile FTFP_BERT_EMZ gun_tb.mac"
  "TBHGCal181Oct_noregions.gdml new_cuts profile FTFP_BERT gun_tb.mac"
)

# Loop over runs and launch each in background
for args in "${runs[@]}"; do
    echo "Launching: $APP $args"
    $APP $args > /dev/null 2>&1 &
done


