echo "Enter keyword to search for root files:"
read keyword
echo "Matching files:"
select old_name in $(ls /eos/user/w/wlivesay/Summer/Data/pre4/OLD/old_files/root_files/ | grep -v "^output" | grep "$keyword" | sed 's/\.root//'); do
    [ -n "$old_name" ] && break
done

echo "Add extra name? (press enter for none):"
read extra_name
if [ -z "$extra_name" ]; then
    extra_name='""'
fi

echo "Min stubs? (press enter for 4):"
read minstubs
minstubs=${minstubs:-4}

echo "Displaced cuts? (y/n):"
read disp
if [ "$disp" = "y" ]; then
    disp="true"
elif [ "$disp" = "n" ]; then
    disp="false"
else
    disp="not value bool given"
fi

echo ".L L1TrackNtuplePlot.C++"

echo "(path, name, extra name, min stubs, disp cuts? (true/false), detailed plots? true/false), minpt, maxeta)"
#echo "L1TrackNtuplePlot(\"$old_name\",\"/eos/user/w/wlivesay/Summer/Data/pre4/OLD/old_files/root_files/\",$extra_name,4,true,true,3.0,2.0)"
echo "L1TrackNtuplePlot(\"$old_name\",\"/eos/user/w/wlivesay/Summer/Data/pre4/OLD/old_files/root_files/\",$extra_name,$minstubs,$disp,true,3.0,2.0)"
root -l
cp TrkPlots/*.txt /eos/user/w/wlivesay/Summer/Data/pre4/OLD/old_files/text_files
echo "mv TrkPlots/*.pdf TrkPlots/*txt /eos/user/w/wlivesay/Summer/Data/pre4/NEW/ttbar/PU/Hybrid/500"
