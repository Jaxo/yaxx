/*
* (C) Copyright 2014 Jaxo Inc.  All rights reserved.
* This work contains confidential trade secrets of Jaxo.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo.
*
* Author:  Pierre G. Richard
* Written: 7/28/2014
*/
package com.jaxo.android.rexx;

import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Locale;

import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.speech.tts.TextToSpeech;
import android.util.Log;

/*-- class Speech --+
*//**
*
* @author  Pierre G. Richard
*/
public class Speech
extends Service
implements TextToSpeech.OnInitListener, Runnable
{
   private static final String TAG = "Speech";
   private TextToSpeech m_tts;
   private Handler m_serviceHandler;
   private Looper m_serviceLooper;
   private Thread m_worker;
   private Object m_lock;
   private String m_language;
   private boolean m_isInited;
   private boolean m_isTtsAvailable;
   private boolean m_isSomethingSaid;
   private boolean m_isCloseRequested;
   public static final int SPEAK = 0;
   private static final int CLOSE = 1;
   private static final String UTTERANCE_ID = "jaxoSpeechClosed";

   @Override
   /*----------------------------------------------------------------onCreate-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   public void onCreate() {
      super.onCreate();
      Log.i(TAG, "onCreate");
      synchronized (this) {
         if (m_serviceLooper == null) {
            m_lock = new Object();
            m_worker = new Thread(this);
            m_worker.start();
            // make sure the service thread started...
            try { wait(); } catch (InterruptedException e) {}
         }
      }
   }

   @Override
   /*---------------------------------------------------------------onDestroy-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   public void onDestroy() {
      super.onDestroy();
      Log.i(TAG, "onDestroy - " + whatState());
      if (!m_isCloseRequested) {
         synchronized (m_lock) {
            if (m_isCloseRequested) return;
            m_isCloseRequested = true;
            m_serviceHandler.sendMessage(m_serviceHandler.obtainMessage(CLOSE));
            Log.i(TAG, "onDestroy - closing");
            try {
               m_lock.wait(10000);
               Log.i(TAG, "onDestroy - closed");
            }catch (InterruptedException e) {
               Log.i(TAG, "onDestroy - close forced");
               m_tts.stop();
               m_tts.shutdown();
            }
         }
      }
   }

   /*--------------------------------------------------------------------bind-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   static void bind(
      Context ctxt,
      final MonitorListener listener
   ) {
      ctxt.bindService(
         new Intent(ctxt, Speech.class),
         new ServiceConnection() {
            public void onServiceConnected(
               ComponentName className, IBinder binder
            ) {
               listener.onSpeechReady(
                  this, ((LocalBinder)binder).getService()
               );
            }
            public void onServiceDisconnected(ComponentName className) {
               listener.onSpeechEnded();
            }
         },
         Context.BIND_AUTO_CREATE
      );
   }

   /*------------------------------------------------------------------onBind-+
   * Hide as much as possible the intricacies on Service binding
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   @Override
   public IBinder onBind(Intent intent) {
      return new LocalBinder();
   }
   private class LocalBinder extends Binder {
      Handler getService() { return m_serviceHandler; }
   }
   interface MonitorListener {
      void onSpeechReady(ServiceConnection monitor, Handler conn);
      void onSpeechEnded();
   }

   /*-----------------------------------------------------------SpeechHandler-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   private static class SpeechHandler extends Handler {
      private final WeakReference<Speech> m_speaker;
      SpeechHandler(Speech speaker) {
         m_speaker = new WeakReference<Speech>(speaker);
      }
      public void handleMessage(Message msg) {
         Speech speaker = m_speaker.get();
         if (speaker != null) speaker.handleMessage(msg);
      }
   }

   /*---------------------------------------------------------------------run-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   public void run() {
      Looper.prepare(); // prepare the looper before creating the handler
      m_serviceLooper = Looper.myLooper();
      m_serviceHandler = new SpeechHandler(this);
      synchronized (this) { notify(); }       // handler is ready
      TextToSpeech.OnInitListener oil = this;
      Context ctxt = this;
      m_tts = new TextToSpeech(ctxt, oil);
      Looper.loop();
      m_tts.stop();
      m_tts.shutdown();
      synchronized (m_lock) { m_lock.notifyAll(); } // handler is finished
   }

   /*-----------------------------------------------------------handleMessage-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
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
            String[] args = (String[])msg.obj;
            setLanguage(args[1]);
            m_tts.speak(args[0], TextToSpeech.QUEUE_ADD, null);
            m_isSomethingSaid = true;
         }
      }else {   // CLOSE
         if (m_isSomethingSaid) {
            HashMap<String, String>parms = new HashMap<String, String>();
            parms.put(
               TextToSpeech.Engine.KEY_PARAM_UTTERANCE_ID,
               UTTERANCE_ID
            );
            m_tts.setOnUtteranceCompletedListener(
               new TextToSpeech.OnUtteranceCompletedListener() {
                  @Override
                  public void onUtteranceCompleted(String utteranceId) {
                     if (utteranceId.equals(UTTERANCE_ID)) {
                        m_serviceLooper.quit();  // delay
                     }
                  }
               }
            );
            m_tts.speak("", TextToSpeech.QUEUE_ADD, parms);
         }else {
            m_serviceLooper.quit();
         }
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
            m_isTtsAvailable = true;
         }
         m_lock.notifyAll();  // in case a "say" is waiting...
      }
   }

   /*-------------------------------------------------------------setLanguage-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   private void setLanguage(String language) {
      boolean isChangeRequired = false;
      if (m_language == null) {
         if (
            (language.length() > 0) &&
            !language.equals(getDefaultLanguage())
         ) {
            isChangeRequired = true;
            m_language = language;
         }
      }else if (language.length() == 0) {
         isChangeRequired = true;
         m_language = null;
      }else if (!m_language.equals(language)) {
         isChangeRequired = true;
         if (language.equals(getDefaultLanguage())) {
            m_language = null;
         }else {
            m_language = language;
         }
      }
      if (isChangeRequired) {
         Locale locale;
         if (m_language == null) {
            locale = Locale.getDefault();
         }else {
            int p = m_language.indexOf('-');
            if (p >= 0) {
               locale = new Locale(
                  m_language.substring(0, p),
                  m_language.substring(p+1)
               );
            }else {
               locale = new Locale(m_language);
            }
         }
         m_tts.setLanguage(locale);
      }
   }

   /*------------------------------------------------------getDefaultLanguage-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   private String getDefaultLanguage() {
      Locale deflt = Locale.getDefault();
      String defltLang = deflt.getLanguage();
      if (deflt.getCountry().length() > 0) {
         defltLang += "-" + deflt.getCountry();
      }
      return defltLang;
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
