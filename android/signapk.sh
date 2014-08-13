#!/bin/bash
# Sample usage is as follows;
# ./signapk xxxx.apk ~/keystore jaxo4 pass
# 
# param1, APK file: xxxx.apk
# param2, keystore location: ~/keystore
# param3, alias: jaxo4

USER_HOME=$(eval echo ~${SUDO_USER})

APK=$1
KEYSTORE="${2:-$USER_HOME/keystore}"
ALIAS="${3:-jaxo4}"
read -p "Password?" STOREPASS

# get the filename
APK_BASENAME=$(basename $APK)
SIGNED_APK="signed_"$APK_BASENAME

#debug
echo param1 $APK
echo param2 $KEYSTORE
echo param3 $ALIAS

# delete META-INF folder
zip -d $APK META-INF/\*

# sign APK
jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore $KEYSTORE -storepass $STOREPASS $APK $ALIAS
#verify
jarsigner -verify $APK

#zipalign
echo aligning...
~/android-sdks/build-tools/20.0.0/zipalign -v 4 $APK $SIGNED_APK
~/android-sdks/build-tools/20.0.0/zipalign -c -v 4 $SIGNED_APK
