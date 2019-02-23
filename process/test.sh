#!/bin/bash
for testCase in ./input/*
do
    outputfile=${testCase//"in"/"out"}
    ./multisum < ${testCase} > ${outputfile}
done
