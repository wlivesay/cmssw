files=()
for f in Billy/root_files/*.root; do
  [[ "$f" != *output* ]] && continue
  files+=("$f")
done

echo "Enter a string to filter file1 options (or press Enter to skip):"
read filter1
filtered1=()
for f in "${files[@]}"; do
  [[ -n "$filter1" && "$f" != *$filter1* ]] && continue
  filtered1+=("$f")
done

echo "Select file1:"
select file1 in "${filtered1[@]}"; do
  [[ -n "$file1" ]] && break
done

echo "Enter a string to filter file2 options (or press Enter to skip):"
read filter2
filtered2=()
for f in "${files[@]}"; do
  [[ -n "$filter2" && "$f" != *$filter2* ]] && continue
  filtered2+=("$f")
done

echo "Select file2:"
select file2 in "${filtered2[@]}"; do
  [[ -n "$file2" ]] && break
done

echo "File1: $file1"
echo "File2: $file2"

echo "File1: $file1"
echo "File2: $file2"
echo "Enter data type (resVsEta_z0...) from list below:"
rootls "$file1"
read data_type
echo "Include ratio subplot? (y/n):"
read subplot
echo ".L Billy/overlay/overlay.C++"
echo "overlay(\"$data_type\",\"$file1\",\"$file2\", \"$subplot\")"
root -l
echo "mv Billy/overlay/outputs/*.pdf /eos/user/w/wlivesay/Summer/Data/pre4/NEW/overlay"

