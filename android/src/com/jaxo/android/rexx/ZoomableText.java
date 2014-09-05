/*
* (C) Copyright 2014 Jaxo Inc.  All rights reserved.
* This work contains confidential trade secrets of Jaxo Inc.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo.
*
* Author:  Pierre G. Richard
* Written: 8/11/2014
*/
package com.jaxo.android.rexx;

import android.content.Context;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;

/*-- class ZoomableText --+
*//**
* @author  Pierre G. Richard
*/
public class ZoomableText extends android.widget.EditText {
   private PinchZoomer m_zoomer;
   public ZoomableText(Context context) {
      super(context);
      m_zoomer = this.new PinchZoomer(context);
   }
   public ZoomableText(Context context, AttributeSet attrs) {
      super(context, attrs);
      m_zoomer = this.new PinchZoomer(context);
   }
   public ZoomableText(Context context, AttributeSet attrs, int defStyle) {
      super(context, attrs, defStyle);
      m_zoomer = this.new PinchZoomer(context);
   }

   /*
   | Don't pay attention to the weirdness of this code.  Android (IMHO)
   | suffers from a design flaw in the event architecture, probably
   | due to legacy.  A bozo naively thought it could be fixed by lint.
   | And now, I have to cheat with lint so it doesn't nag at me.
   */
   @Override
   public boolean performClick() {
      return super.performClick();
   }

   public boolean onTouchEvent(MotionEvent event) {
      m_zoomer.onTouchEvent(event);
      if (m_zoomer == null) performClick();  // shut up, clueless lint!
      return super.onTouchEvent(event);
   }
   public float getScale() { return m_zoomer.m_scale; }
   public void setScale(float scale) { m_zoomer.m_scale = scale; }

   class PinchZoomer extends ScaleGestureDetector.SimpleOnScaleGestureListener
   {
      public float m_scale;
      private float m_baseSize;
      private ScaleGestureDetector m_detector;

      PinchZoomer(Context context) {
         m_scale = 1.f;
         m_baseSize = Preferences.getTextSizeInSp(context);
         setTextSize(m_baseSize);
         m_detector = new ScaleGestureDetector(context, this);
      }
      public boolean onTouchEvent(MotionEvent event) {
         return m_detector.onTouchEvent(event);
      }
      @Override
      public boolean onScale(ScaleGestureDetector detector) {
         m_scale *= detector.getScaleFactor();
         if (m_scale < 0.1f) {
            m_scale = 0.1f;
         }else if (m_scale > 5.0f) {
            m_scale = 5.0f;
         }
         setTextSize(m_scale * m_baseSize);
         return true;
      }
   }
}
/*===========================================================================*/
