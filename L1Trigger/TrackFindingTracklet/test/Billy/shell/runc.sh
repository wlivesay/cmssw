ls Billy/root_files
echo "Enter root file name without .root:"
read old_name
echo "Add extra name?(   or  _name)"
read extra_name
echo ".L L1TrackNtuplePlot.C++"

echo "(path, name, extra name, min stubs, disp cuts? (true/false), detailed plots? true/false), minpt, maxeta)"
echo "L1TrackNtuplePlot(\"$old_name\",\"Billy/root_files/\",$extra_name,4,true,true,3.0,2.0)"
root -l
cp TrkPlots/*.txt Billy/text_files
echo "mv TrkPlots/*.pdf TrkPlots/*txt /eos/user/w/wlivesay/Summer/Data/pre4/NEW/ttbar/PU/Hybrid/500"
