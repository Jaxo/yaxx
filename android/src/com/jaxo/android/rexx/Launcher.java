/*
* $Id: Launcher.java,v 1.2 2011-08-30 07:16:13 pgr Exp $
*
* (C) Copyright 2011 Jaxo Inc.  All rights reserved.
* This work contains confidential trade secrets of Jaxo.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo.
*
* Author:  Pierre G. Richard
* Written: 8/10/2011
*/
package com.jaxo.android.rexx;

import java.lang.ref.WeakReference;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;

/*-- class Launcher --+
*//**
*
* @author  Pierre G. Richard
* @version $Id: Launcher.java,v 1.2 2011-08-30 07:16:13 pgr Exp $
*/
public class Launcher extends Activity
{
   static private final int STOP_SPLASH = 1;

   @Override
   /*----------------------------------------------------------------onCreate-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   public void onCreate(Bundle savedInstanceState)
   {
      super.onCreate(savedInstanceState);
      setContentView(R.layout.splash);
      Handler handler = new LaunchHandler(this);
//    handler.sendMessageDelayed(handler.obtainMessage(STOP_SPLASH), 5000);
      handler.sendMessage(handler.obtainMessage(STOP_SPLASH));
   }

   /*-----------------------------------------------------------handleMessage-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   public void handleMessage(Message msg) {
      if (msg.what == STOP_SPLASH) {
         finish();
         startActivity(new Intent(Launcher.this, ScriptsList.class));
      }
   }

   /*-----------------------------------------------------------LaunchHandler-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   private static class LaunchHandler extends Handler {
      private final WeakReference<Launcher> m_launcher;
      LaunchHandler(Launcher launcher) {
         m_launcher = new WeakReference<Launcher>(launcher);
      }
      @Override
      public void handleMessage(Message msg) {
         Launcher launcher = m_launcher.get();
         if (launcher != null) launcher.handleMessage(msg);
      }
   }
}
/*===========================================================================*/
