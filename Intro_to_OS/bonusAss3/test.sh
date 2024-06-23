make
echo "TESTCASE1"
mv testcase1/process.file ./
mv testcase1/queue.cfg ./
./Scheduler
diff output.log testcase1/output.log
echo "TESTCASE2"
mv testcase2/process.file ./
mv testcase2/queue.cfg ./
./Scheduler
diff output.log testcase2/output.log
echo "TESTCASE3"
mv testcase3/process.file ./
mv testcase3/queue.cfg ./
./Scheduler
diff output.log testcase3/output.log
echo "TESTCASE4"
mv testcase4/process.file ./
mv testcase4/queue.cfg ./
./Scheduler
diff output.log testcase4/output.log
echo "TESTCASE5"
mv testcase5/process.file ./
mv testcase5/queue.cfg ./
./Scheduler
diff output.log testcase5/output.log


