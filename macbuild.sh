#!/bin/sh

xcodebuild -project projects/mac/pricer.xcodeproj -alltargets -configuration Release

xcodebuild -project projects/mac/pricer64.xcodeproj -alltargets -configuration Release

