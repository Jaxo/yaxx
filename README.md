Note: this readme is far to be finished and I hope to complete it over time&hellip;
![Image](../import_feature/docs/rexxdroid.png?raw=true)
<a href="testRel/myLib/README.md">myLib/README.md</a>
#What is YAXX?
One could consider YAXX as a collection of many pieces of code that have been
developed along a period of 20 years of fun at doing programming.

- **toolslib** gathers C++ objects that were missing from the original C++
libraries (and, in my opinion, the gap was never filled
the way Java designers did it later.)

- **decnblib** is Mike Cowlishaw's decimal number library, getting rid of the
16-bit arithmetic barrier

- **stdiolib** makes a bridge from the C-stdio to java, easing the exchange of
data via streams.

- **yasp3lib** is an SGML Parser (SGML being the precursor of XML)

- **rexxlib** provides all objects and methods required by the implementation
of the Rexx interpreter (Restructured extended editor)

Fortunately, YAXX is not just a miscellany of heterogeneous (although usefull)
routines.  It provides the means to build a _REXX interpreter_, and this is how
most people will understand YAXX.

#Running REXX on my Android Device
You say *"I don't want to build Rexxoid, I just want to install it on my device"*.<br/>
OK, then get the already-built Rexxoid.apk
<br/><br/>
Assuming you read this README from <a href="https://github.com/Jaxo/yaxx" target="_blank">the Jaxo github</a>,
<ol>
<li>in the list above this README, find <i>Rexxoid.apk</i> and click on it;
<li>in the page that appears, click the <i>Raw</i> tab;
<li>this should open a dialog offering a <i>Save File</i> option (at least <a href="www.mozilla.org/en-US/firefox/" target="_blank">Firefox</a> does it!)
<li>Save the file and install it on your device.<br/>The easiest way is to mail the <i>Rexxoid.apk</i> file to your email account on your android device.  Then, you can install straight from the mail.
</ol>
###Any doc?
https://github.com/Jaxo/yaxx/blob/master/yaxx/docs/foo.md
https://github.com/Jaxo/yaxx/blob/master/docs/foo.md
You will find <a href="../import_feature/docs/foo.md">here</a>

#Building REXX
The libraries have been tested in several OS environments, which ascertains
their portability and robustness.
Besides classical OSes (Linux, MS Windows), Palm OS was one of the first
candidate for writing "REXX for Palm OS" at the beginning of the 2000's.
In 2010, REXX was ported to Android...

##Rexx on Android
You need the appropriate android / eclipse project, and to build the .so libraries.
<ul>
<li>The Android Eclipse project &mdash; the GUI layer &mdash; is named *Rexxoid*.
<li>YAXX (this GIT) only builds the .SO libraries, which represents 95% of the
whole edifice.
</ul>

###Summary, only for experts
<pre>
cd ~/yaxx/android
/path/to/android_ndk/ndk-build
</pre>
This produces the .SO files in <code>~/yaxx/android/libs/armeabi</code><br/>
"libs/armeabi" needs to be <i>ln -s'ed</i> from your Android project directory.
<br/>
<br/><u>My Personal Cheatsheet</u>
<ul><li>the ~/yaxx/android/jni directory contains the android makefiles;
<br/>
<li>to examine .so entries, do
<pre>
   <code>nm -gC libtoolslib.so</code>     <i>or,</i>
   <code>readelf -Ws libtoolslib.so | awk "{print $8}"</code>
</pre>
<li>to start an emulator from the console:
<pre>
   android create avd --force -n petrus -t 10 -c 2G
   emulator -avd petrus
</pre>
</ul>


###Detailed Instructions

<ul>
<li><i>These instructions have been tested on Linux Ubuntu 12.04</i>
<br>
<li><i>In what follows, you must replace the symbol <b>[$home]</b> with the fully qualified name of your home directory.<br/>
Ex: <i>[$home]</i> -> /home/myuserid
</ul>

#### Building the .so files
<ol>
<li>First is to get <i>yaxx</i> from this git
<pre>
cd ~
git clone https://github.com/Jaxo/yaxx.git
</pre>
creates the yaxx directory in your home (<i>i.e.:</i>&nbsp; <code><i>[$home]<i>/yaxx</code>)
<br/>
<li>Install Eclipse.&nbsp; you can download it from <a href="http://www.eclipse.orgs/downloads/" target="_blank">here</a>
<br/>
<li>Install the Android plugin for Eclipse.&nbsp; Go to <a href="http://developer.android.com/sdk/installing/installing-adt.html" target="_blank">that page</a> and carefully follow the instructions.
<br/>
<li>Install the Android NDK: android-ndk-r9-linux-x86.tar.bz2 from <a href="http://developer.android.com/tools/sdk/ndk/index.html" target="_blank">here</a>
<br/><br/><i>"NDK" &mdash; not to be confused with "SDK" &mdash; is a complementary tool.</i>&nbsp; <br/>To install it, after download, I did:
<pre>cd /usr/local
sudo mv ~/Downloads/android-ndk-r9-linux-x86.tar.bz2 .
sudo tar -xjvf android-ndk-r9-linux-x86.tar.bz2
sudo rm android-ndk-r9-linux-x86.tar.bz2
cd ~</pre>
<li>Build the .so:
<pre>
cd ~/yaxx/android
/usr/local/android-ndk-r9/ndk-build
</pre>
The ".so" files are created in <code>~/yaxx/android/libs/armeabi</code>.&nbsp;
Later, these will be symbolically linked from the Eclipse Android project.
</ol>

####Create and run the "Rexxoid" project in Eclipse
Before to start, you may want you get acquainted to Android debug mechanism at reading
<a href="http://developer.android.com/tools/index.html" target="_blank"> this document</a>.
<br/>
<ol>
<li>Start Eclipse,  File -> Import&hellip; -> Android -> Existing Android Code Into Workspace -> Next
<br/>
<li>In the dialog <i>Import Projects</i>:
<pre>  Root Directory:  <b><i>[$home]</i>/yaxx/android</b>
  <i>Copy projects into workspace</i> stays unchecked
  <i>Add project to working sets</i> stays unchecked
</pre>
Press <i>Finish</i>.
<br/>
<li>
Right click on the project name "rexxoid" in the Package Explorer pane, then Debug As -> Android Application.<br/>
It is <i>Android Application</i>, do <b>not</b> choose <i>Android Native Application</i>
<br/>
<li>You probably will fall on the <i>Android Device Chooser</i> dialog since you didn't tell what Android Virtual Device you wanted to run with.
<ul>
<li>check <i>Launch a new Android Virtual Device</i>
<li>Press the <i>Manager&hellip;</i> button
<li>In the <i>Android Virtual Device Manager</i> dialog, press the <i>New&hellip;</i> button
<li>In the <i>Create a new Android Virtual Device (AVD)</i> dialog, press the <i>New&hellip;</i> button, and enter:
<pre>
   AVD Name:    <b>RexxDevice</b>
   Device:      <b>3.2" QVGA (ADP2)</b>
   Target:      <b>Android 2.2 - API Level 8</b>
   SD Card:     <b>10MiB</b>
</pre>
The 3rd parameter (Target) is important: you <b>must</b> select a device with min API level 8!  For the other parameters, it's up to your taste.
<li>Press <i>OK</i> and close the <i>Create a new Android Virtual Device (AVD)</i> dialog
<li>Back to the <i>Android Device Chooser</i> dialog, press <i>Refresh</i>, select the newly created device, press OK
<li>Wait 2 or 3 minutes until the Android device gets ready
</ul><br/>
<li>You must then see the Rexx <i>Android King</i>.  Run <i>qheure</i> to test that everything is OK.
</ol>
####Create the APK
<ol>
<li>
Right click on the project name "rexxoid" in the Package Explorer pane, then Android Tools -> Export Signed Application Package.
<li>If you don't yet have a <i>keystore</i>, select <i>Create new keystore</i>
<li>I suggest the target directory of your <i>rexxoid.apk</i> be <i>[$home]</i>/yaxx/android/bin
</ol>
####Install the APK on your device
Use the Android <i>adb</i> tool, from your <i>[android sdk root]</i>/platform-tools directory.<br/>
With your phone device adb-connected to your computer,
<pre>
cd ~/android-sdks/platform-tools
cp ~/yaxx/android/bin/rexxoid.apk .
adb install rexxoid.apk
rm Rexxoid.apk
</pre>

