cd cut0.03mm
nohup time ../build/hgcal_sim ../RunD121_HGCal.gdml "original_cuts" ../gun.mac \
    > myout.txt 2>&1 &
cd ..

cd cutNew
nohup time ../build/hgcal_sim ../RunD121_HGCal.gdml "new_cuts" ../gun.mac \
    > myout.txt 2>&1 &
cd ..
