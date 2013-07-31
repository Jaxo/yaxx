Building the SO files
^^^^^^^^^^^^^^^^^^^^^
The ~/yaxx/jni directory contains the makefiles...  Issue:

| cd ~/yaxx
| /u/pgr/android-ndk-r6b/ndk-build
| (or /usr/android-ndk-r6/ndk-build on tintin)

This produces the .SO files in ~/yaxx/libs/armeabi

One can examine them issuing"
| nm -gC libtoolslib.so
or
| readelf -Ws libtoolslib.so | awk "{print $8}"


Building the apk on Eclipse
^^^^^^^^^^^^^^^^^^^^^^^^^^^
 - RMB on the project name
 - Android tools
 - Export signed package
This produces the Rexxoid.apk in ~/workspace/Rexxoid/bin

Installing the APK
^^^^^^^^^^^^^^^^^^
| cd ~/android-sdk-linux_x86/platform-tools
| cp ~/workspace/Rexxoid/bin/Rexxoid.apk .
| adb install Rexxoid.apk
| rm Rexxoid.apk

Starting an emulator
^^^^^^^^^^^^^^^^^^^^
Create an AVD: android create avd --force -n petrus -t 10 -c 2G
Run it:        emulator -avd petrus
