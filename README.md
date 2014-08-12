<table border="0" align="top">
<tr><td><img src="rexx/rexx.png"/></td>
<td>
<b><a href="https://github.com/Jaxo/yaxx/releases/latest">LATEST RELEASE</a></b> August, 8th 2014 - "Emerald"</td></tr></table>
<h1>What is YAXX?</h1>
One could consider YAXX as a collection of many pieces of code that have been developed along a period of 20 years of fun at doing programming.

- **toolslib** gathers C++ objects that were missing from the original C++ libraries (and, in my opinion, the gap was never filled the way Java designers did it later.)

- **decnblib** is Mike Cowlishaw's decimal number library, getting rid of the 16-bit arithmetic barrier

- **stdiolib** makes a bridge from the C-stdio to java, easing the  exchange of data via streams.

- **yasp3lib** is an SGML Parser (SGML being the precursor of XML)

- **rexxlib** provides all objects and methods required by the implementation of the Rexx interpreter (Restructured extended editor)

Fortunately, YAXX is not just a miscellany of heterogeneous (although usefull)
routines.  It provides the means to build a _REXX interpreter_, and this is
how most people will understand YAXX.
<h3>Any doc?</h3>
No real detailed documentation. After all, Rexx is &hellip; Rexx, a well documented programming language and the Rexxoid application should be enough intuitive to get you up and running quite immediately. Nevertheless, you will find <a href="docs/android/rexx.md">here</a> a series of screenshots describing in short how Rexx shows up on an Android device.
<h1>Installing REXX</h1>
If you don't want to build Rexx or Rexxoid, but just install it, pre-built
executables are provided for Android, Linux and Windows.
Also, Rexxoid -- Rexx for Android -- is available on Google Play Store.
<ul>
<li>Navigate to the <a href="https://github.com/Jaxo/yaxx/releases/latest">releases page</a>
<li>Download the appropriate version and file
<li>Install it on your device.
<br/>For Rexxoid (REXX on Android), the easiest way is to mail the <i>Rexxoid.apk</i> file to your email account on your android device.  Then, you can install straight from the mail.
</ul>
If the proposed pre-builts do not match your OS versions, you will have to build it by yourself. As explained below, the repository provides the appropriate makefiles and it's not so difficult to do.
<h2>Importing my beloved REXX scripts in Android</h2>
<ul>
<li>Insert a comment in the first line of each script, with a couple of words identifying the script.
<li>Send the script(s) as mail attachments to your email account on your android device, open the mail, save the attachment.
<li>Start REXX and press to the (hardware) left button on your phone. In the options menu, choose import.
</ul>
Note that tapping on a file or on an email attachment with a <b>.rexx</b> extension automatically launch Android REXX.
<h1>Building REXX</h1>
The libraries have been tested in several OS environments, which ascertains their portability and robustness.  Besides classical OS's (Linux, MS Windows), Palm OS was one of the first candidate for writing "REXX for Palm OS" at the beginning of the 2000's. In 2010, REXX was ported to Android...

<h2>Rexx on Android</h2>
<ul>
<li>First is to create the .so libraries required by the Java Native Interface.
<li>Then, to build the <code>rexxoid.apk</code>. It adds the Android's Graphic User Interface
(GUI layer) leading to the final and only piece to upload to the Android device.
We will make this Application PacKage by creating an Android Project
under the Eclipse IDE.
</ul>

<h3>Summary, only for experts</h3>
<pre>
cd ~/yaxx/android
<i>(your path to the android ndk)</i>/ndk-build
</pre>
This produces the .SO files in <code>~/yaxx/android/libs/armeabi</code><br/>
"libs/armeabi" needs to be <i>ln -s'ed</i> from your Android project directory.
<pre>
cd ~/yaxx/android
ant clean release
</pre>
produces the final Rexxoid-release.apk in the android/bin directory.
<br/><br/><u>My Personal Cheatsheet</u>
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
<h3>Detailed Instructions</h3>
<ul>
<li><i>These instructions have been tested on Linux Ubuntu 12.04</i>
<br>
<li><i>In what follows, you must replace the symbol <b>[\$home]</b> with the fully qualified name of your home directory.<br></i>
Ex: <i>[\$home]</i> -> /home/myuserid
</ul>

<h4>Building the .so files</h4>
<ol>
<li>First is to get <i>yaxx</i> from this git
<pre>
cd ~
git clone https://github.com/Jaxo/yaxx.git
</pre>
creates the yaxx directory in your home (<i>i.e.:</i>&nbsp; <code><i>[$home]</i>/yaxx</code>)
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

<h4>Create and run the "Rexxoid" project in Eclipse</h4>
Before to start, you may want you get acquainted to Android debug mechanism at reading
<a href="http://developer.android.com/tools/index.html" target="_blank"> this document</a>.
<br/>
<ol>
<li>Install Eclipse.&nbsp; you can download it from <a href="http://www.eclipse.orgs/downloads/" target="_blank">here</a>
<br/>
<li>Install the Android plugin for Eclipse.&nbsp; Go to <a href="http://developer.android.com/sdk/installing/installing-adt.html" target="_blank">that page</a> and carefully follow the instructions.
<br/>
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
<li>
If you get interrupted by this infamous message:
<pre>Unable to execute dex: java.nio.BufferOverflowException.</pre>
(from http://stackoverflow.com/questions/20778767)
<ol>
<li>Right click on project and go to properties
<li>Go to Java Build Path Screen (from left menu)
<li>Select Libraries Pane
<li>Highlight Android dependencies
<li>Click Remove
<li>Click Ok and restart at previous step.
</ol>
<li>The <i>Android Device Chooser</i> dialog will probably show up, since you didn't tell what Android Virtual Device you wanted to run with.
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
<h4>Create the APK</h4>
<ol>
<li>
Right click on the project name "rexxoid" in the Package Explorer pane, then Android Tools -> Export Signed Application Package.
<li>If you don't yet have a <i>keystore</i>, select <i>Create new keystore</i>
<li>I suggest the target directory of your <i>rexxoid.apk</i> be <i>[$home]</i>/yaxx/android/bin
</ol>
If you prefer, the <code>android</code> tool provides a mean to create a
build.xml file for Apache Ant. Such one can be found under the android
directory.  You could recreate it by entering
<pre>
(path to your SDK/tools directory)android update project -p (your eclipse project directory)</code>
</pre>
<h4>Install the APK on your device</h4>
Use the Android <i>adb</i> tool, from your <i>[android sdk root]</i>/platform-tools directory.<br/>
With your phone device adb-connected to your computer, do
<pre>
cd ~/android-sdks/platform-tools
cp ~/yaxx/android/bin/rexxoid.apk .
adb install rexxoid.apk
rm rexxoid.apk
</pre>
<h2>Rexx on Linux, Windows, &hellip;</h2>
On a Linux build machine, to produce the Linux-executable REXX module requires the build tools to be installed first:
<pre>
sudo apt-get update
sudo apt-get install build-essential
</pre>
Then, enter:
<pre>
make clean rexx
</pre>
This is it!

To produce a WIN32 executable, (aka <i><b>rexx.exe</b></i>) is also doable. You need a cross-compiler, that is, <i>mingw-w64</i>:
<pre>
sudo apt-get install mingw-w64
make os=win32 clean rexx
</pre>

On a MS Windows machine, you might be able to produce an executable
using MS development tools.
The repository has still the dsp/dsw files required by Visual C++
and the like. I didn't test it -- I no more use MS-Windows since years -- but
it has chances to work. The "makeall.bat" file might help.


  [1]: file:///home/pgr/yaxx/rexx/rexx.png
