The _Makefile_ in Yaxx main directory, does the versioning assuming
that the following instructions have been understood and applied.
The _Makefile_ calls underneath the C-shell script _version_ which
may be used independently to display what is currently checked-out.

## Publishing a patched release, or a new release

Github has a "publish" feature which tags the code with a version number
and a title.
For YAXX, the title is a "code name", taken out of the names
of Colorado Summits, see http://www.100summits.com/full-summit-list.

Unfortunately, Github tags are only lightweight, nor signed,
nor annotated. Signatures inforce security, annotations allow to
add important information, as, for instance the code name of a
particular release.  This code name appears in many sections of
our source code, and doing the change manually is tedious while
error prone.

In what follows:
<ul><li>
_v1.x.y_ has to be replaced by its real value, that is
<code>v1.5.2</code> for example, where 1 is the MAJOR, 5 the MINOR and
2 the PATCH (no patch => no y!)
</li><li>
_codeName_ is the "code name", that is <code>Silverheels</code> for example
</li></ul>

<h3>Step I</h3>
For a <b>new release only</b> (hence a new _codeName_),
<ul><li>
annotate the branch:
<pre>
git branch --edit-description
</pre>
You'll be presented with a nano editor screen. First line must be
the new code name: _codeName_.
</li><li>
increment the _android:versionCode="?"_ in yaxx/android/AndroidManifest.xml
</li></ul>

<h3>Step II</h3>
Make sure everything is up-to-date:
<pre>
cd $HOME/yaxx
# Rexx for Linux:
make clean rexx
# The android part:
cd android
/usr/local/android-ndk-r10/ndk-build
$HOME/android-sdks/tools/android update project --path .  #optional
ant clean release
# check that the remote origin is in sync, commit and push if needed
git status
</pre>

<h3>Step III</h3>
Run the script $HOME/yaxx/version
<ul><li>
For a <b>patched release</b>,
note the version (_v1.x.y_), where 'y' is the patch no
</li><li>
For a <b>new release</b>,
the version is _v1.n_, where 'n' is the version no + 1
</li></ul>

<h3>Step IV</h3>
<ol><li>
Go to the <a href="https://github.com/Jaxo/yaxx"> Yaxx repository on Github</a>
</li><li>
In the header at the top, click <b>releases</b>.
</li><li>
Click <b>Draft a new release</b>.
</li><li>
The version number is _v1.x.y_
</li><li>
Select the branch that contains the project to release.
Usually, it's against the master branch, unless for releasing beta software.
</li><li>
The title is the _codeName_ (see above)
</li><li>
In the description panel, describe briefly what has been changed,
and end with the following:
<pre>
The &lt;a href="README.md">README file &lt;/a> may contain more infos.

Download either
&lt;ul>
&lt;li>rexx.out  the linux version (rename it just 'rexx')
&lt;li>rexx_32.exe  the windows version
&lt;li>Rexxoid.apk the Android version
&lt;/ul>
</li><li>
Click <b>Publish release</b>.
</li></ol>

<h3>Step V</h3>
After version v1.x.y has been published on github, first thing
to do is make it an annotated tag from which the proper code name
is extracted each time the revision is rebuilt.
<pre>
git pull
git checkout v1.x.y
git tag -d v1.x.y
setenv GIT_COMMITTER_DATE `git show --format=%aD | head -1`
</pre>
Before to start the signing process. Make sure that your gpg matches!
<pre>
# check 'gpg --list-key' (doesn't work)
git tag -s -a v1.x.y
</pre>
You'll be presented with a nano editor screen. First line must be
the code name, matching the title that you gave at publish time.
The next lines is whatever you judge to be appropriate.

Then, replace the tag on github:
<pre>
git push origin :refs/tags/v1.x.y
git push --tags
</pre>
back to the master branch:
<pre>
git checkout master
</pre>
rebuild the binaries:
<pre>
cd $HOME/yaxx
# Rexx for Linux:
make clean rexx
# Rexx for Win 32:
make os=win32 clean rexx
# The android part:
cd android
ant clean release
</pre>

<h3>Step VI</h3>
Upload the binaries to the release distribution.
<pre>
mv $HOME/yaxx/android/bin/Rexxoid-release.apk $HOME/yaxx/Rexxoid.apk
</pre>
<ol><li>
Go to the <a href="https://github.com/Jaxo/yaxx"> Yaxx repository on Github</a>
</li><li>
In the header at the top, click <b>releases</b>.
</li><li>
Click <b>Edit</b> inside the box describing the release.
It opens a new page width a "binaries" box.
</li><li>
Manually, select these files to enter in the binaries box:
<ul><li>
$HOME/yaxx/Rexxoid.apk
</li><li>
$HOME/yaxx/rexx/rexx.out
</li><li>
$HOME/yaxx/rexx/rexx_32.exe
</li></ul>
</li><li>
Click <b>Publish release</b>.
</li></ol>

<h3>Step VII</h3>
This is the last step: update Rexxoid on Google Play.

## Working on a branch ##

On development branches (i.e.: not the master branch) there should
be no tags -- or it must be exceptionnal. Therefore, the "codename"
is indicated in the first line of the branch's annotation, until it
later becomes the release's code name when merged back to master.

To add an annotation to the branch, do:
<pre>
git branch --edit-description
</pre>
You'll be presented with a nano editor screen. First line must be
the code name. Note that the branch name might differ, although it
is better to have some intuitive correspondance between both.
