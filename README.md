Note: this file is far to be finished and we hope to complete it over time...

What is YAXX?
=============
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

Building REXX
=============
The libraries have been tested in several OS environments, which ascertains
their portability and robustness.
Besides classical OSes (Linux, MS Windows), Palm OS was one of the first
candidate for writing "REXX for Palm OS" at the beginning of the 2000's.
In 2010, REXX was ported to Android...

Rexx on Android
---------------
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
<BR>
<BR/><U>My Personal Cheatsheet</U>
<ul><li>the ~/yaxx/jni directory contains the android makefiles;
<li>to examine .so entries, do
<BR/>&nbsp;<code>nm -gC libtoolslib.so</code>&nbsp; &nbsp; <i>or,</i>
<BR/>&nbsp;<code>readelf -Ws libtoolslib.so | awk "{print $8}"</code> 
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
<li>Change 2 "hard-coded" values in the <i>.project</i> file.&nbsp; 
<ul>
<li>edit <i>[$home]</i>/yaxx/android/.project
<li>change (at the end of this file) <b>$HOME</b> for <b><i>[$home]</i></b>
<pre>   &lt;linkedResources>
      &lt;link>
         &lt;name>jni&lt;/name>
         &lt;type>2&lt;/type>
         &lt;location><b>/home/pgr</b>/yaxx/jni&lt;/location>
      &lt;/link>
      &lt;link>
         &lt;name>libs&lt;/name>
         &lt;type>2&lt;/type>
         &lt;location><b>/home/pgr</b>/yaxx/libs&lt;/location>
      &lt;/link>
   &lt;/linkedResources></pre></ul>
<li>Start Eclipse,  File -> Import&hellip; -> Android -> Existing Android Code Into Workspace.
<br/>Android 2.1</i> is the SDK to be used (Android Preferences.)
</ol>
