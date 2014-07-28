## How do we find a resource to read or write ##
__In YAXX a "STREAM name" is an URI.__

First, at init, a default baseUri is defined<br/>
<code>in yaxx/android/src/com/jaxo/android/rexx/Rexx.java</code>
<pre>
String baseUri = (
   "file:///" +
   getBaseContext().getFilesDir().getAbsolutePath() + "/"
);
</pre>
<code>in yaxx/androidlib/JRexx.cpp:</code>
<pre>
char const * pBaseUri = env->GetStringUTFChars(baseUri, 0);
SystemContext context(
   android_system,
   pBaseUri,
   StdFileSchemeHandler(),
   SpeakerSchemeHandler(env, speaker)
);
</pre>
Then, when a string name is encountered (like in charout):<br/>
<code>in yaxx/rexxlib/IOMgr.cpp</code>
<pre>
IOMgr::charout(RexxString name, ...)                                    478
   UriEntry * entry = ensureWrite(name);
IOMgr::UriEntry * IOMgr::ensureWrite(RexxString & name)                 110
   return makeEntry(name, ios::out);
IOMgr::UriEntry * IOMgr::makeEntry(RexxString & name, ios__openmode om) 214
   entry = new UriEntry(name, om);
IOMgr::UriEntry::UriEntry(RexxString & name, ios__openmode om) :         21
   m_uri(name, SystemContext.getBaseUri)    <= is a RegisteredURI
   [...]
   m_pStream = m_uri.getStream(ios::in);
}
</pre>
<code>in yaxx/toolslib/RegisteredURI.cpp</code>
<pre>
RegisteredURI::RegisteredURI(                                            78
   char const * const pszUri,     // asciiz string to build the URI from
   URI const & baseURI            // the base URI - can be URI::Nil
) :
   URI(pszUri, baseURI, m_factory) {
}
</pre>
<code>in yaxx/toolslib/URI.cpp</code>
<pre>
URI::URI(                                                                52
   char const * const spec,
   URI const & baseURI,
   SchemeHandlerFactory & finder
) {
   // several lines of code here: this the the real worker
   // it computes the scheme and reveals StdFileSchemeHandler as the handler
}
</pre>
<code>in yaxx/toolslib/URI.h</code>
<pre>
inline iostream * URI::getStream(ios__openmode om) {
   return m_handler.isPresent()? m_handler->makeStream(*this, om) : 0;
}
</pre>
<code>in yaxx/toolslib/StdFileStream.cpp</code>
<pre>
iostream * StdFileSchemeHandler::Rep::makeStream(
   URI const & uri,
   ios__openmode om
) {
   char const * path = uri.getPath();
   return new StdFileStream(path);
}
</pre>



