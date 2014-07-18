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
import java.util.Arrays;
import java.util.List;

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
   public RexxConsole(Activity activity, TextView view) {
      m_activity = activity;
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
   *//*
   +-------------------------------------------------------------------------*/
   public int system(String[] args)
   {
      List<String> argsList = Arrays.asList(args);
      Process process = null;
      int rc = -1;
      flush();
      try {
         process = new ProcessBuilder().
         command(argsList).
         redirectErrorStream(true).
         start();
         rc = process.waitFor();
         InputStream in = process.getInputStream();
         while (-1 != (m_tmpBufPos = in.read(m_tmpBuf))) {
            flush();
         }
         m_tmpBufPos = 0;
      }catch (Exception e) {
         Log.e("RexxConsole", "system (Process)", e);
      }
      if (process != null) process.destroy();
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
