#!/bin/bash
for case in ./input/*
do
    result = ${case//"in"/"out"}
    ./multisum < ${case} > ${result}
done