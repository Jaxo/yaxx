Note: this readme is far to be finished and I hope to complete it over time&hellip;

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
<li>in the list above this README, find *Rexxoid.apk* and click on it;
<li>in the page that appears, click the *Raw* tab;
<li>this should open a dialog offering a *Save File* option (at least <a href="www.mozilla.org/en-US/firefox/" target="_blank">‎Firefox</a> does it!)
<li>Save the file and install it on your device.<br/>The easiest way is to mail the *Rexxoid.apk* file to your email account on your android device.  Then, you can install straight from the mail. 
</ol>



#Building REXX
The libraries have been tested in several OS environments, which ascertains
their portability and robustness.
Besides classical OSes (Linux, MS Windows), Palm OS was one of the first
candidate for writing "REXX for Palm OS" at the beginning of the 2000's.
In 2010, REXX was ported to Android...

##Rexx on Android
You need the appropriate android / eclipse project, and to build the .so libraries.

The Android Eclipse project -- the GUI layer -- is named *Rexxoid*, and can be
retrieved from [NOT YET DONE].

YAXX (this GIT) only builds the .SO libraries, which represents 95% of the
whole edifice.

###Summary, only for experts
<pre>
cd ~/yaxx
/path/to/android_ndk/ndk-build
</pre>
This produces the .SO files in <code>~/yaxx/libs/armeabi</code><br/>
"libs/armeabi" needs to be <i>ln -s'ed</i> from your Android project directory.
<br/>
<br/><u>My Personal Cheatsheet</u>
<ul><li>the ~/yaxx/jni directory contains the android makefiles;
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
<li>First is to get _yaxx_ from this git
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
cd ~/yaxx
/usr/local/android-ndk-r9/ndk-build
</pre>
The ".so" files are created in <code>~/yaxx/libs/armeabi</code>.&nbsp;
Later, these will be symbolically linked from the Eclipse Android project.
</ol>

####Create and run the "Rexxoid" project in Eclipse
Before to start, you may want you get acquainted to Android debug mechanism at reading 
<a href="http://developer.android.com/tools/index.html" target="_blank"> this document</a>.
<br/>
<ol>
<li>Change 2 "hard-coded" values in the *.project* file.&nbsp;
<br>
<ul>
<li>edit *[$home]*/yaxx/android/.project
<br/>
<li>at the end of this file: 
<pre>   &lt;linkedResources>
      &lt;link>
         &lt;name>jni&lt;/name>
         &lt;type>2&lt;/type>
         &lt;location>**/home/pgr**/yaxx/jni&lt;/location>
      &lt;/link>
      &lt;link>
         &lt;name>libs&lt;/name>
         &lt;type>2&lt;/type>
         &lt;location>**/home/pgr**/yaxx/libs&lt;/location>
      &lt;/link>
   &lt;/linkedResources></pre>
change **/home/pgr** for your own home directory, aka <b> *[$home]*</b></ul>
<br/>
<li>Start Eclipse,  File -> Import&hellip; -> Android -> Existing Android Code Into Workspace -> Next
<br/>
<li>In the dialog *Import Projects*:
<pre>  Root Directory:  <b>*[$home]*/yaxx/android</b>
  *Copy projects into workspace* stays unchecked
  *Add project to working sets* stays unchecked
</pre>
Press *Finish*.
<br/>
<li>Expand the project "rexxoid" and check that the "jni" and "libs" library are not empty. Otherwise you've missed a step, restart from scratch!
<br/>
<li>
Right click on the project name "rexxoid" in the Package Explorer pane, then Debug As -> Android Application.<br/>
It is *Android Application*, do **not** choose *Android Native Application*
<br/>
<li>You probably will fall on the *Android Device Chooser* dialog since you didn't tell what Android Virtual Device you wanted to run with.
<ul>
<li>check *Launch a new Android Virtual Device*
<li>Press the *Manager&hellip;* button 
<li>In the *Android Virtual Device Manager* dialog, press the *New&hellip;* button
<li>In the *Create a new Android Virtual Device (AVD)* dialog, press the *New&hellip;* button, and enter:
<pre>
   AVD Name:    <b>RexxDevice</b>
   Device:      <b>3.2" QVGA (ADP2)</b>
   Target:      <b>Android 2.1 - API Level 7</b>
   SD Card:     <b>10MiB</b>
</pre>
The 3rd parameter (Target) is important: you **must** select a device with min API level 7!  For the other parameters, it's up to your taste.
<li>Press *OK* and close the *Create a new Android Virtual Device (AVD)* dialog
<li>Back to the *Android Device Chooser* dialog, press *Refresh*, select the newly created device, press OK
<li>Wait 2 or 3 minutes until the Android device gets ready
</ul><br/>
<li>You must then see the Rexx *Android King*.  Run *qheure* to test that everything is OK.
</ol>
####Create the APK
<ol>
<li>
Right click on the project name "rexxoid" in the Package Explorer pane, then Android Tools -> Export Signed Application Package.
<li>If you don't yet have a *keystore*, select *Create new keystore*
<li>I suggest the the target directory of your *Rexxoid.apk* be *[$home]*/yaxx/bin
</ol>
####Install the APK on your device
Use the Android *adb* tool, from your *[android sdk root]*/platform-tools directory.<br/>
With your phone device adb-connected to your computer,
<pre>
cd ~/android-sdk-linux_x86/platform-tools
cp ~/workspace/Rexxoid/bin/Rexxoid.apk .
adb install Rexxoid.apk
rm Rexxoid.apk
</pre>