
#!/bin/bash
##
# We compile all examples using arduino-cli in order to identify compile errors 
# The return codes are made available in the build-examples-log.txt file. 
# -> rc=0: success
# -> rc=1: error
##
#arduino-cli lib upgrade
#arduino-cli lib linstall 
git -C .. pull
git -C ../../ESP32-A2DP pull

function compile_example {
  ARCH=$1
  FILES=$2
  for f in $FILES
  do
    echo "Processing $f ..."
    # take action on each file. $f store current file name
    #arduino-cli compile  -b "$ARCH"  "$f"
    arduino-cli compile  -b "$ARCH"  --build-property "build.partitions=huge_app" --build-property "upload.maximum_size=3145728" "$f"
    EC=$?
    #if [ $EC -ne 0 ]; then
      #break
      echo -e "$f -> rc=$EC" >> "build-examples-log.txt"
    #fi
  done
}

compile_example "esp32:esp32:esp32" "../examples/examples-basic-api/base*"
compile_example "esp32:esp32:esp32" "../examples/examples-player/player*"
compile_example "esp32:esp32:esp32" "../examples/examples-webserver/str*"
compile_example "esp32:esp32:esp32" "../examples/examples-stream/streams*"
compile_example "esp32:esp32:esp32" "../examples/examples-audiokit/*"
compile_example "esp32:esp32:esp32" "../examples/examples-maximilian/*"
#compile_example "esp32:esp32:esp32" "../examples/tests/test*"

