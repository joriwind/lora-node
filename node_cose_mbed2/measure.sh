log=size.log

echo "New measurement: $1" >> $log
date >> $log
mbed compile >> $log
echo "" >> $log
