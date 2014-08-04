/*
* (C) Copyright 2014 Jaxo Inc.  All rights reserved.
* This work contains confidential trade secrets of Jaxo.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo.
*
* Author:  Pierre G. Richard
* Written: 7/31/2014
*/
package com.jaxo.android.rexx;
import java.util.Iterator;
import java.util.NoSuchElementException;

/*-- class Exploder --+
*//**
*
* @author  Pierre G. Richard
*/
public class Exploder implements Iterator<String> {
   private char[] m_line;
   private final int m_len;
   private int m_ix;
   private char[] m_buf;
   public Exploder(String s) {
      m_line = s.toCharArray();
      m_len = m_line.length;
      m_ix = 0;
      m_buf = new char[m_len];
   }
   public boolean hasNext() {
      while ((m_ix < m_len) && (m_line[m_ix] <= (char)0x20)) ++m_ix; // trim
      return (m_ix < m_len);
   }
   public String remainder() {
      hasNext();
      return new String(m_line, m_ix, m_len-m_ix);
   }
   public String next() {
      if (!hasNext()) throw new NoSuchElementException();
      boolean isBkslash = false;
      char chQuoteStart = 0;
      int ixArg = 0;
      for (; m_ix < m_len; ++m_ix) {
         char ch = m_line[m_ix];
         if (isBkslash) {
            isBkslash = false;
            m_buf[ixArg++] = ch;
         }else if (ch == '\\') {
            isBkslash = true;
         }else if (chQuoteStart != 0) {
            if (ch == chQuoteStart) {
               chQuoteStart = 0;
            }else {
               m_buf[ixArg++] = ch;
            }
         }else if ((ch == '\'') || (ch == '"')) {
            chQuoteStart = ch;
         }else if (ch > 0x20) {
            m_buf[ixArg++] = ch;
         }else {
            ++m_ix;
            break;
         }
      }
      return new String(m_buf, 0, ixArg);
   }
   public void remove() {
      throw new UnsupportedOperationException();
   }
}
/*===========================================================================*/
/*
   static Vector<String> buildArgs(String s) {
      Exploder exploder = new Exploder(s);
      Vector<String> args = new Vector<String>();
      while (exploder.hasNext()) {
         args.addElement(exploder.next());
         args.addElement(exploder.remainder());
      }
      return args;
   }

   static void show(Vector<String> arr) {
      for (int i=0; i < arr.size(); ++i) {
         System.out.println(
            "==> \"" + arr.elementAt(i) + "\" R: \"" + arr.elementAt(++i) + "\""
         );
      }
   }

   static public void main(String[] s) {
      Vector<String> arr;
      if (s.length == 0) {
         arr = buildArgs("/sdcard/download/launchee.rexx hello Bob");
         show(arr);
         arr = buildArgs("/sdcard/download/qdate.rexx");
         show(arr);
      }else {
         for (int i=0, max=s.length; i < max; ++i) {
            arr = buildArgs(s[0]);
            show(arr);
         }
      }
      for (;;) {
         arr = buildArgs(System.console().readLine());
         show(arr);
      }
   }
*/