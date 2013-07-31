/*
* $Id: Preferences.java,v 1.1 2011-08-15 13:30:32 pgr Exp $
*
* (C) Copyright 2011 Jaxo.  All rights reserved.
* This work contains confidential trade secrets of Jaxo.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo.
*
* Author:  Pierre G. Richard
* Written: 8/15/2011
*/
package com.jaxo.android.rexx;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.PackageInfo;
import android.os.Bundle;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;

/*-- class Preferences --+
*//**
*
* @author  Pierre G. Richard
* @version $Id: Preferences.java,v 1.1 2011-08-15 13:30:32 pgr Exp $
*/
public class Preferences extends PreferenceActivity
{
   /*----------------------------------------------------------------onCreate-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   @Override
   protected void onCreate(Bundle savedInstanceState) {
      super.onCreate(savedInstanceState);
      addPreferencesFromResource(R.layout.preferences);
   }

   /*----------------------------------------------------------getPreferences-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   static SharedPreferences getPreferences(Context context)
   {
      SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(
         context
      );
      try {
         PackageInfo info = context.getPackageManager().getPackageInfo(
            "com.jaxo.android.rexx", 0
         );
         int lastVersionNo = info.versionCode;
         if (lastVersionNo > prefs.getInt("VERSION_NO", -1)) {
            PreferenceManager.setDefaultValues(
               context, R.layout.preferences, false
            );
            prefs.edit().putInt("VERSION_NO", lastVersionNo).commit();
         }
      }catch (Exception e) {}
      return prefs;
   }
}

/*===========================================================================*/
