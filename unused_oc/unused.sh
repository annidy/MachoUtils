#!/bin/sh
cd MachoUnused/src/com/annidy
javac -classpath ../.. Main.java
cd ../..
java com.annidy.Main $1