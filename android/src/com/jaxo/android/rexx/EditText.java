/*
* $Id: EditText.java,v 1.1 2013-08-05 08:14:10 pgr Exp $
*
* (C) Copyright 2013 Jaxo Inc.  All rights reserved.
* This work contains confidential trade secrets of Jaxo Inc.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo.
*
* Author:  Pierre G. Richard
* Written: 8/5/2013
*/
package com.jaxo.android.rexx;

import android.content.Context;
import android.text.InputType;
import android.util.AttributeSet;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;

/*-- class EditText --+
*//**
* This class is required so that *all* key events are sent
* to the "onKey" method of the android.view.View.OnKeyListener
*
* @author  Pierre G. Richard
* @version $Id: EditText.java,v 1.1 2013-08-05 08:14:10 pgr Exp $
*/
class EditText extends android.widget.EditText {
   public EditText(Context context) {
      super(context);
   }
   public EditText(Context context, AttributeSet attrs) {
      super(context, attrs);
   }
   public EditText(Context context, AttributeSet attrs, int defStyle) {
      super(context, attrs, defStyle);
   }
   @Override
   public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
      BaseInputConnection fic = new BaseInputConnection(this, false);
      // outAttrs.actionLabel = null;
      outAttrs.inputType = InputType.TYPE_NULL;
      // outAttrs.imeOptions = EditorInfo.IME_ACTION_NEXT;
      return fic;
   }
   @Override
   public boolean onCheckIsTextEditor() {
      return true;
   }
}
/*===========================================================================*/
