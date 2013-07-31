/*
* $Id: About.java,v 1.1 2011-08-12 16:47:01 pgr Exp $
*
* (C) Copyright 2011 Jaxo Inc.  All rights reserved.
* This work contains confidential trade secrets of Jaxo.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo.
*
* Author:  Pierre G. Richard
* Written: 8/12/2011
*/
package com.jaxo.android.rexx;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;

/*-- class About --+
*//**
*
* @author  Pierre G. Richard
* @version $Id: About.java,v 1.1 2011-08-12 16:47:01 pgr Exp $
*/
public class About extends Activity {
   @Override
   public void onCreate(Bundle savedInstanceState) {
      super.onCreate(savedInstanceState);
      setContentView(R.layout.about);
      try {
         ((TextView)findViewById(R.id.edition)).setText(
            String.format(
               getString(R.string.Edition), getString(R.string.versionname)
            )
         );
         ((TextView)findViewById(R.id.forandroid)).setText(
            String.format(
               getString(R.string.forandroid),
               android.os.Build.VERSION.RELEASE
            )
         );
      }catch (Exception e) {}
   }
}
/*===========================================================================*/
