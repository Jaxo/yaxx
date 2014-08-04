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

import java.util.Vector;

import android.content.Context;
import android.content.ServiceConnection;
import android.os.Handler;

/*-- class Speaker --+
*//**
*
* @author  Pierre G. Richard
* @version $Id: Speaker.java,v 1.5 2012-03-10 16:40:53 pgr Exp $
*/
class Speaker implements Speech.MonitorListener
{
   // private static final String TAG = "Speaker";
   private Context m_context;
   private Vector<String[]> m_msgQueue;
   private ServiceConnection m_speechMonitor;
   private Handler m_speech;

   /*-----------------------------------------------------------------Speaker-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   Speaker(Context context) {
      m_context = context;
      m_msgQueue = new Vector<String[]>(10);
      Speech.bind(context, this);
   }

   /*---------------------------------------------------------------------say-+
   *//**
   * The JNI side is deemed to have public access to this method
   *//*
   +-------------------------------------------------------------------------*/
   public void say(String what, String how)
   {
      String[] args = {
         new StringBuffer(what).toString(),  // deep copies
         new StringBuffer(how).toString()
      };
      if (m_speech == null) {
         synchronized (m_msgQueue) {
            if (m_speech == null) {
               m_msgQueue.add(args);
               return;
            }
         }
      }
      m_speech.sendMessage(m_speech.obtainMessage(Speech.SPEAK, args));
   }

   /*-------------------------------------------------------------------close-+
   *//**
   *  Stop the connection and release the resources
   *//*
   +-------------------------------------------------------------------------*/
   public void close() {
      if (m_speechMonitor != null) {
         m_context.unbindService(m_speechMonitor);
         m_speechMonitor = null;
         m_speech = null;
      }
   }

   @Override
   /*-----------------------------------------------------------onSpeechReady-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   public void onSpeechReady(ServiceConnection monitor, Handler speech) {
      synchronized (m_msgQueue) {
         m_speech = speech;
         m_speechMonitor = monitor;
         // destack all pending messages
         while (m_msgQueue.size() > 0) {
            String[] args = m_msgQueue.get(0);
            m_msgQueue.remove(0);
            m_speech.sendMessage(m_speech.obtainMessage(Speech.SPEAK, args));
         }
      }
   }

   @Override
   /*-----------------------------------------------------------onSpeechEnded-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   public void onSpeechEnded() {
      m_speechMonitor = null;
      m_speech = null;
   }
}
/*===========================================================================*/
