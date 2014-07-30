The _Makefile_ in Yaxx main directory, does the versioning assuming
that the following instructions have been understood and applied.
The _Makefile_ calls underneath the C-shell script _version_ which
may be used independently to display what is currently checked-out.

#### Publishing a new release ####

Releases use the "publish" feature of Github which tags the code:
with a version number "v1.x" and a "title". For YAXX, the title
is a "code name", taken out of the names of Colorado Summits
see http://www.100summits.com/full-summit-list.

Unfortunately, Github tags are only lightweight, nor signed,
nor annotated. Signatures inforce security, annotations allow to
add important information, as, for instance the code name of a
particular release.  This code name appears in many sections of
our source code, and doing the change manually is tedious while
error prone.

In what follows _v1.x.y_ has te be replaced by its real value, that is
<code>v1.5.2</code> for instance, where 1 is the MAJOR, 5 the MINOR and
2 the PATCH.

After version v1.x.y has been published on github, first thing
to do is make it an annotated tag from which the proper code name
is extracted each time the revision is rebuilt.
<pre>
git checkout v1.x.y
git tag -d v1.x.y
setenv GIT_COMMITTER_DATE `git show --format=%aD | head -1`
</pre>
Before to start the signing process. Make sure that your gpg matches!
<pre>
check 'gpg --list-key'
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

#### Working on a branch ####

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


