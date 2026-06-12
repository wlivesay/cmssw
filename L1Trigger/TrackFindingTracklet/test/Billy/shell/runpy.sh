#I used to have gotocms4 and cmsenv here but I moved to start.sh
recompile
cd L1Trigger/TrackFindingTracklet/test/
cmsRun L1TrackNtupleMaker_cfg.py
echo "New root file name: MUST INCLUDE .root"
read new_name
path="Billy/root_files/$new_name"
cp L1TrkNtuple.root "$path"
rm Billy/root_files/cancel
echo ".L L1TrackNtuplePlot.C++"
echo "L1TrackNtuplePlot(\"L1TrkNtuple\")"
echo "L1TrackNtuplePlot(\"$new_name\",\"Billy/root_files/\")"
root -l
echo "mv TrkPlots/*.pdf /eos/user/w/wlivesay/Summer/Data/pre4/NEW/ttbar/PU/Hybrid/500"

