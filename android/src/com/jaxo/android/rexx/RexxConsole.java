/*
* $Id: RexxConsole.java,v 1.6 2012-03-10 16:40:27 pgr Exp $
*
* (C) Copyright 2011 Jaxo Inc.  All rights reserved.
* This work contains confidential trade secrets of Jaxo.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo.
*
* Author:  Pierre G. Richard
* Written: 8/11/2011
*/
package com.jaxo.android.rexx;

import java.io.InputStream;
import java.net.URI;
import java.util.Vector;

import android.app.Activity;
import android.text.Editable;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnKeyListener;
import android.widget.TextView;

/*-- class RexxConsole --+
*//**
* See "JConsole" as the C++/JNI counterpart class.
*
* @author  Pierre G. Richard
* @version $Id: RexxConsole.java,v 1.6 2012-03-10 16:40:27 pgr Exp $
*/
public class RexxConsole implements OnKeyListener
{
   Activity m_activity;
   URI m_baseUri;
   Object m_lock;
   Editable m_outBuf;
   StringBuilder m_inBuf;
   public String m_result;     // JNI assumes public access to this field

   private static final int TMP_BUFSIZE = 1024;
   private byte[] m_tmpBuf;
   private int m_tmpBufPos;

   /*-----------------------------------------------------------------Console-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   public RexxConsole(Activity activity, URI baseUri, TextView view) {
      m_activity = activity;
      m_baseUri = baseUri;
      m_lock = new Object();
      m_outBuf = view.getEditableText();
      view.setOnKeyListener(this);
      m_inBuf = new StringBuilder(100);
      m_tmpBuf = new byte[TMP_BUFSIZE];
      m_result = "";
   }

   /*------------------------------------------------------------------system-+
   *//**
   * The JNI side is deemed to have public access to this method
   *
   * See if this is a shortcut for am start ...
   * For now (7/30/14) we only consider x.rexx being passed
   * to a new instance of the interpreter:
   * Ex:
   *   "xyz.rexx a b c"
   * will go:
   *   "am start -a android.intent.action.view " +
   *   "-d file:///sdcard/download/xyz.rexx a b c" +
   *   "-n com.jaxo.android.rexx/Rexx "
   *
   * am command line parameters (burried deeply in the doc, order matters!)
   * ^^^^^^^^^^^^^^^^^^^^^^^^^^
   * [-a [ACTION]] [-d [DATA_URI]] [-t [MIME_TYPE]]
   * [-c [CATEGORY] [-c [CATEGORY]] ...]
   * [-e|--es [EXTRA_KEY] [EXTRA_STRING_VALUE] ...]
   * [--esn [EXTRA_KEY] ...]
   * [--ez [EXTRA_KEY] [EXTRA_BOOLEAN_VALUE] ...]
   * [--ei [EXTRA_KEY] [EXTRA_INT_VALUE] ...]
   * [--el [EXTRA_KEY] [EXTRA_LONG_VALUE] ...]
   * [--eu [EXTRA_KEY] [EXTRA_URI_VALUE] ...]
   * [--eia [EXTRA_KEY] [EXTRA_INT_VALUE][,[EXTRA_INT_VALUE...]]
   * [--ela [EXTRA_KEY] [EXTRA_LONG_VALUE][,[EXTRA_LONG_VALUE...]]
   * [-n [COMPONENT]] [-f [FLAGS]]
   * [--grant-read-uri-permission] [--grant-write-uri-permission]
   * [--debug-log-resolution] [--exclude-stopped-packages]
   * [--include-stopped-packages]
   * [--activity-brought-to-front] [--activity-clear-top]
   * [--activity-clear-when-task-reset] [--activity-exclude-from-recents]
   * [--activity-launched-from-history] [--activity-multiple-task]
   * [--activity-no-animation] [--activity-no-history]
   * [--activity-no-user-action] [--activity-previous-is-top]
   * [--activity-reorder-to-front] [--activity-reset-task-if-needed]
   * [--activity-single-top] [--activity-clear-task]
   * [--activity-task-on-home]
   * [--receiver-registered-only] [--receiver-replace-pending]
   * [--selector]
   *//*
   +-------------------------------------------------------------------------*/
   public int system(String cmdLine) {
      Exploder exploder = new Exploder(cmdLine);
      int rc = -1;
      flush();
      if (exploder.hasNext()) {
         Vector<String> args = new Vector<String>();
         String arg =  exploder.next();
         if (arg.endsWith(".rexx")) {
            URI script = m_baseUri.resolve(arg);
            if (script.getPath().endsWith(".rexx")) {
               args.addElement("am");
               args.addElement("start");
               args.addElement("-a");
               args.addElement("android.intent.action.view");
               args.addElement("-d");
               args.addElement(script.toString());
               if (exploder.hasNext()) {
                  args.addElement("-e");
                  args.addElement(Rexx.SCRIPT_ARGS_KEY);
                  args.addElement(exploder.remainder());
               }
               args.addElement("-n");
               args.addElement("com.jaxo.android.rexx/.Rexx");
               args.addElement("--activity-single-top");
            }
         }else {
            args.add(arg);
            while (exploder.hasNext()) args.add(exploder.next());
         }
         try {
            Process process = new ProcessBuilder().
            command(args).
            redirectErrorStream(true).
            start();
            rc = process.waitFor();
            InputStream in = process.getInputStream();
            while ((m_tmpBufPos = in.read(m_tmpBuf)) != -1) {
               flush();
            }
            m_tmpBufPos = 0;
            // process.destroy(); => No! it throws "NoSuchProcessException"
         }catch (Exception e) {
            Log.e("RexxConsole", "system (Process)", e);
         }
      }
      return rc;
   }

   /*---------------------------------------------------------------------get-+
   *//**
   * The JNI side is deemed to have public access to this method
   *//*
   +-------------------------------------------------------------------------*/
   public int get() {
      // runs on REXX interpreter thread
      flush();
      synchronized (m_lock) {
         char ch = '?';
         for (;;) {
            if (m_inBuf.length() > 0) {
               ch = m_inBuf.charAt(0);
               m_inBuf.deleteCharAt(0);
               break;
            }
            try {
               m_lock.wait(10000);
            }catch (Exception e) {
               e.printStackTrace();
               break;
            }
         }
         return ch;
      }
   }

   /*---------------------------------------------------------------------put-+
   *//**
   * The JNI side is deemed to have public access to this method
   *//*
   +-------------------------------------------------------------------------*/
   public void put(int ch) {
      // runs on REXX interpreter thread
      if (m_tmpBufPos >= TMP_BUFSIZE) flush();
      m_tmpBuf[m_tmpBufPos++] = (byte)ch;
   }

   /*-------------------------------------------------------------------flush-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   public void flush() {
      if (m_tmpBufPos > 0) {
         try {
            final String out = new String(m_tmpBuf, 0, m_tmpBufPos, "UTF8");
            m_tmpBufPos = 0;
            m_activity.runOnUiThread(
               new Runnable() {
                  public void run() {
                     m_outBuf.append(out);
                  }
               }
            );
         }catch (Exception e) {} // UnsupportedEncodingException's can't occur
      }
   }

   /*-------------------------------------------------------------------onKey-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   public boolean onKey(View view, int keyCode, KeyEvent event) {
      // runs on the main UI thread
      if (event.getAction() != KeyEvent.ACTION_DOWN) return false;
      int ch = event.getUnicodeChar();
      if (ch == 0) {
         return false;
      }else {
         synchronized(m_lock) {
            m_inBuf.append((char)ch);
            m_lock.notify();
         }
         put(ch);
         flush();
         return true;
      }
   }
}
/*===========================================================================*/
