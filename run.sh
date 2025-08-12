
cd cut0.03mm
{ time ../build/hgcal_sim ../RunD121_HGCal.gdml "original_cuts" ../gun.mac; } > myout.txt 2>&1

cd ../cutNew
{ time ../build/hgcal_sim ../RunD121_HGCal.gdml "new_cuts" ../gun.mac; } > myout.txt 2>&1

cd ..

