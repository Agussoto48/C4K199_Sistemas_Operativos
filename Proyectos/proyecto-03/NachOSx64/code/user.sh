#
pwd > pwd.txt
hash=`awk -F/ -f ../user.awk pwd.txt`
rm -f pwd.txt
eval $hash > .user.dat
