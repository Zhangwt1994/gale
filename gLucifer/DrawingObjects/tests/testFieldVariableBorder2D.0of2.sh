#!/bin/sh

TEST_SCRIPT=./VMake/executableTester.sh
until test -r ${TEST_SCRIPT} ; do
        TEST_SCRIPT=../${TEST_SCRIPT}
done
. ${TEST_SCRIPT}

DIFF=./OpenGL-diff.sh
export DIFF

runAndHandleSystemTestStdLocations "testDrawingObject testFieldVariableBorder.xml" "$0" "$@"
