#I used to have gotocms4 and cmsenv here but I moved to start.sh
recompile
cd L1Trigger/TrackFindingTracklet/test/
cmsRun L1TrackNtupleMaker_cfg.py
echo "New root file name: NO .root (remember stub#, trkstub#, TPstub#)"
read new_name
path="/eos/user/w/wlivesay/Summer/Data/pre4/OLD/old_files/root_files/$new_name.root"
cp L1TrkNtuple.root "$path"
rm /eos/user/w/wlivesay/Summer/Data/pre4/OLD/old_files/root_files/cancel
echo "Ntuple created: $path. Run runc to make plots."




#echo ".L L1TrackNtuplePlot.C++"
#echo "L1TrackNtuplePlot(\"L1TrkNtuple\")"
#echo "L1TrackNtuplePlot(\"$new_name\",\"/eos/user/w/wlivesay/Summer/Data/pre4/OLD/old_files/root_files/\")"
#root -l
#echo "mv TrkPlots/*.pdf /eos/user/w/wlivesay/Summer/Data/pre4/NEW/ttbar/PU/Hybrid/500"

