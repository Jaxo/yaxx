/*
* $Id: Speaker.java,v 1.5 2012-03-10 16:40:53 pgr Exp $
*
* (C) Copyright 2011 Jaxo Inc.  All rights reserved.
* This work contains confidential trade secrets of Jaxo.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo.
*
* Author:  Pierre G. Richard
* Written: 8/15/2011
*/
package com.jaxo.android.rexx;

import java.util.HashMap;
import java.util.Locale;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.speech.tts.TextToSpeech;
import android.util.Log;

/*-- class Speaker --+
*//**
*
* @author  Pierre G. Richard
* @version $Id: Speaker.java,v 1.5 2012-03-10 16:40:53 pgr Exp $
*/
public class Speaker implements
TextToSpeech.OnInitListener,
TextToSpeech.OnUtteranceCompletedListener,
Runnable
{
   private static final String TAG = "Speaker";
   private TextToSpeech m_tts;
   private Handler m_handler;
   private Locale m_locale;
   private Context m_context;
   private Thread m_worker;
   private Object m_lock;
   private boolean m_isInited;
   private boolean m_isTtsAvailable;
   private boolean m_isSomethingSaid;
   private boolean m_isCloseRequested;
   private static final int SPEAK = 0;
   private static final int CLOSE = 1;
   private static final String UTTERANCE_ID = "jaxoSpeakerClosed";

   /*-----------------------------------------------------------------Speaker-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   Speaker(Context context) { this(context, Locale.getDefault()); }

   /*-----------------------------------------------------------------Speaker-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   Speaker(Context context, Locale locale) {
      m_locale = locale;
      m_lock = new Object();
      m_context = context;
      m_worker = new Thread(this);
      m_worker.start();
      // ensure the thread handler is ready to accept messages
      synchronized (this) { try { wait(); } catch (InterruptedException e) {} }
   }

   /*---------------------------------------------------------------------say-+
   *//**
   * The JNI side is deemed to have public access to this method
   *//*
   +-------------------------------------------------------------------------*/
   public void say(String sArg)
   {
      if (!m_isCloseRequested) {
         String s = new StringBuffer(sArg).toString(); // deep copy
         synchronized (m_lock) {
            if (!m_isCloseRequested) {
               m_handler.sendMessage(m_handler.obtainMessage(SPEAK, s));
            }
         }
      }
   }

   /*---------------------------------------------------------------------run-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   public void run() {
      try {
         Looper.prepare();
         m_handler = new Handler() {
            public void handleMessage(Message msg) {
               if (!m_isInited) {
                  synchronized (m_lock) {
                     while (!m_isInited) {
                        // wait for TextToSpeech initialization
                        try { m_lock.wait(); } catch (InterruptedException e) {}
                     }
                  }
               }
               if (msg.what == SPEAK) {
                  if (m_isTtsAvailable) {
                     m_tts.speak(msg.obj.toString(), TextToSpeech.QUEUE_ADD, null);
                     m_isSomethingSaid = true;
                  }
               }else {
                  if (m_isSomethingSaid) {
                     HashMap<String, String>parms = new HashMap<String, String>();
                     parms.put(
                        TextToSpeech.Engine.KEY_PARAM_UTTERANCE_ID,
                        UTTERANCE_ID
                     );
                     m_tts.setOnUtteranceCompletedListener(Speaker.this);
                     m_tts.speak("", TextToSpeech.QUEUE_ADD, parms);
                  }else {
                     m_tts.stop();
                     m_tts.shutdown();
                  }
                  getLooper().quit();
               }
            }
         };
         synchronized (this) { notify(); } // handler is ready
         TextToSpeech.OnInitListener b = this;
         m_tts = new TextToSpeech(m_context, b); // "this": OnInitListener
         Looper.loop();
      }catch (Throwable t) {
         Log.e(TAG, "Looper exception ", t);
      }
   }

   /*----------------------------------------------------onUtteranceCompleted-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   public void onUtteranceCompleted(String uttId) {
      if (uttId.equals(UTTERANCE_ID)) {
         m_tts.stop();
         m_tts.shutdown();
      }
   }

   /*------------------------------------------------------------------onInit-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   public void onInit(int status) {
      synchronized (m_lock) {
         m_isInited = true;
         if (status == TextToSpeech.SUCCESS) {
            if (m_tts.setLanguage(m_locale) < 0) m_tts.setLanguage(Locale.US);
            m_isTtsAvailable = true;
         }
         m_lock.notifyAll();  // in case a "say" is waiting...
      }
   }

   /*-------------------------------------------------------------------close-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   public void close() {
      Log.i(TAG, "close() - " + whatState());
      if (!m_isCloseRequested) {
         synchronized (m_lock) {
            if (m_isCloseRequested) return;
            m_isCloseRequested = true;
            m_handler.sendMessage(m_handler.obtainMessage(CLOSE));
            m_lock.notifyAll();
         }
         try {
            m_worker.join(12000);
            Log.i(TAG, "close(): closed");
         }catch (InterruptedException e) {
            Log.i(TAG, "close(): close forced **");
            m_tts.stop();
            m_tts.shutdown();
         }
      }
   }

   /*---------------------------------------------------------------whatState-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   private String whatState() {
      return (
        "status:\n\tInited:\t" + m_isInited +
        "\n\tTTS OK:\t" + m_isTtsAvailable +
        "\n\tUsed:  \t" + m_isSomethingSaid +
        "\n\tClosed:\t" + m_isCloseRequested
      );
   }
}
/*===========================================================================*/
