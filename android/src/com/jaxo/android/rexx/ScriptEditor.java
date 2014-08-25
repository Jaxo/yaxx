/*
* $Id: ScriptEditor.java,v 1.7 2011-08-23 08:38:11 pgr Exp $
*
* (C) Copyright 2011 Jaxo Inc.  All rights reserved.
* This work contains confidential trade secrets of Jaxo.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo.
*
* Author:  Pierre G. Richard
* Written: 8/6/2011
*/
package com.jaxo.android.rexx;

import java.util.Date;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.ContentValues;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.DialogInterface.OnClickListener;
import android.os.Bundle;
import android.view.View;
import android.view.WindowManager.LayoutParams;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

/*-- class ScriptEditor --+
*//**
*
* @author  Pierre G. Richard
* @version $Id: ScriptEditor.java,v 1.7 2011-08-23 08:38:11 pgr Exp $
*/
public class ScriptEditor extends Activity
{
   public static final String CONTENT = "Content";
   public static final String FOR_IMMEDIATE_RUN = "NowRun";
   private RexxDatabase m_rexxDb;
   private ContentValues m_content;
   private Long m_id;
   private boolean m_isForImmediateRun;
   private EditText m_editTitle;
   private EditText m_editBody;

   @Override
   /*----------------------------------------------------------------onCreate-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   protected void onCreate(Bundle savedInstanceState)
   {
      super.onCreate(savedInstanceState);
      m_rexxDb = new RexxDatabase(this);
      setContentView(R.layout.script_editor);
      setTitle(R.string.ScriptEditor);
      m_editTitle = (EditText)findViewById(R.id.title);
      m_editBody = (EditText)findViewById(R.id.body);
      if (savedInstanceState != null) {
         m_content = (ContentValues)savedInstanceState.getParcelable(CONTENT);
         if (savedInstanceState.containsKey(RexxDatabase._ID)) {
            m_id = savedInstanceState.getLong(RexxDatabase._ID);
         }
      }else {
         Bundle extras = getIntent().getExtras();
         if ((extras != null) && extras.containsKey(RexxDatabase._ID)) {
            m_id = extras.getLong(RexxDatabase._ID);
            m_content = m_rexxDb.queryScript(m_id);
            if (extras.containsKey(FOR_IMMEDIATE_RUN)) {
               m_isForImmediateRun = extras.getBoolean(FOR_IMMEDIATE_RUN);
            }
         }else {
            m_content = new ContentValues();
            m_content.put(RexxDatabase.TITLE, "");
            m_content.put(RexxDatabase.BODY, "");
            m_content.put(RexxDatabase.STATUS, RexxDatabase.STATUS_UNKNOWN);
         }
      }
      ((Button)findViewById(R.id.done)).setOnClickListener(
         new View.OnClickListener() {
            public void onClick(View view) {
               confirmQuit();
            }
         }
      );

      ((Button)findViewById(R.id.save)).setOnClickListener(
         new View.OnClickListener() {
            public void onClick(View view) {
               saveContent();
               m_id = m_rexxDb.saveScript(m_id, m_content);
            }
         }
      );
      ((Button)findViewById(R.id.saveAs)).setOnClickListener(
         new View.OnClickListener() {
            public void onClick(View view) {
               saveContent();
               m_id = m_rexxDb.saveScript(null, m_content);
            }
         }
      );
      ((Button)findViewById(R.id.revert)).setOnClickListener(
         new View.OnClickListener() {
            public void onClick(View view) {
               if (m_id != null) {
                  m_content = m_rexxDb.queryScript(m_id);
                  showContent();
               }
            }
         }
      );
      ((Button)findViewById(R.id.run)).setOnClickListener(
         new View.OnClickListener() {
            public void onClick(View view) {
               runScript();
            }
         }
      );
      showContent();
      if (m_isForImmediateRun) runScript();
   }

   @Override
   /*-----------------------------------------------------------onBackPressed-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   public void onBackPressed() {
      // super.onBackPressed();
      confirmQuit();
   }

   /*-------------------------------------------------------------confirmQuit-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   private void confirmQuit()
   {
      setResult(RESULT_OK);
      if (isModified()) {
         AlertDialog.Builder builder = new AlertDialog.Builder(this);
         builder.setMessage(R.string.SaveChanges);
         builder.setPositiveButton(
            R.string.CancelQuit,
            new OnClickListener() {
               public void onClick(DialogInterface dialog, int id) {
                  dialog.cancel();
               }
            }
         );
         builder.setNegativeButton(
            R.string.ReallyQuit,
            new OnClickListener() {
               public void onClick(DialogInterface dialog, int id) {
                  dialog.cancel();
                  finish();
               }
            }
         );
         builder.setIcon(R.drawable.icon);
         builder.show();
      }else {
         finish();
      }
   }

   /*---------------------------------------------------------------runScript-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   public void runScript() {
      Intent intent = new Intent(this, Rexx.class);
      saveContent();
      intent.putExtra(
         Rexx.SCRIPT_CONTENT_KEY, m_editBody.getText().toString()
      );
      startActivityForResult(intent, 0);
   }

   @Override
   /*--------------------------------------------------------onActivityResult-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   protected void onActivityResult(
      int requestCode, int resultCode, Intent intent
   ) {
      super.onActivityResult(requestCode, resultCode, intent);
      if (intent == null) return; // probably canceled
      Bundle extras = intent.getExtras();
      if (extras != null) {
         if (resultCode == Rexx.RESULTCODE_OK) {
            // TODO: implement the return of the REXX Result
            // String result = extras.getString(Rexx.RESULT_KEY);
            m_content.put(RexxDatabase.STATUS, RexxDatabase.STATUS_COMPILED);
            if (m_isForImmediateRun) {
               isModified();  // only for setting the status (content -> DB)
               finish();
            }
         }else {
            m_isForImmediateRun = false; // b/c user may now modify it
            m_content.put(RexxDatabase.STATUS, RexxDatabase.STATUS_ERRONEOUS);
            showContent();
            final String message = extras.getString(Rexx.REXX_MESSAGE_KEY);
            final String title = (
               (resultCode == Rexx.RESULTCODE_EXCEPTION_THROWN)?
               getString(R.string.RexxException) :
               String.format(
                  getString(R.string.RexxError),
                  extras.getInt(Rexx.REXX_ERRORCODE_KEY)
               )
            );
            new Dialog(this) {
               @Override
               public void onCreate(Bundle savedInstanceState) {
                  super.onCreate(savedInstanceState);
                  setContentView(R.layout.message);
                  getWindow().setLayout(
                     LayoutParams.MATCH_PARENT,
                     LayoutParams.WRAP_CONTENT
                  );
                  setTitle(title);
                  ((TextView)findViewById(R.id.message)).setText(message);
               }
            }.show();
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
      m_rexxDb.close();
   }

   @Override
   /*-----------------------------------------------------onSaveInstanceState-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   protected void onSaveInstanceState(Bundle outState) {
      super.onSaveInstanceState(outState);
      outState.putParcelable(CONTENT, m_content);
      if (m_id != null) {
         outState.putLong(RexxDatabase._ID, m_id);
      }
   }

   /*-----------------------------------------------------------------onPause-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   @Override
   protected void onPause() {
      super.onPause();
      saveContent();
   }

   /*----------------------------------------------------------------onResume-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   @Override
   protected void onResume() {
      super.onResume();
      showContent();
   }

   /*-------------------------------------------------------------saveContent-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   private void saveContent() {
      m_content.put(RexxDatabase.UPDATE_DATE, new Date().getTime());
      m_content.put(RexxDatabase.TITLE, m_editTitle.getText().toString());
      m_content.put(RexxDatabase.BODY, m_editBody.getText().toString());
   }

   /*-------------------------------------------------------------showContent-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   private void showContent() {
      m_editTitle.setText(m_content.getAsString(RexxDatabase.TITLE));
      m_editTitle.setCompoundDrawablesWithIntrinsicBounds(
         0, 0,
         RexxDatabase.statusToBulletImage(
            m_content.getAsInteger(RexxDatabase.STATUS)
         ), 0
      );
      m_editBody.setText(m_content.getAsString(RexxDatabase.BODY));
   }

   /*--------------------------------------------------------------isModified-+
   *//**
   * isModified() silently updates the status when the script was unchanged
   *//*
   +-------------------------------------------------------------------------*/
   private boolean isModified()
   {
      String title;
      String body;
      int status;
      saveContent();
      if (m_id == null) {
         title = "";
         body = "";
         status = RexxDatabase.STATUS_UNKNOWN;
      }else {
         ContentValues dbContent = m_rexxDb.queryScript(m_id);
         title = dbContent.getAsString(RexxDatabase.TITLE);
         body = dbContent.getAsString(RexxDatabase.BODY);
         status =  dbContent.getAsInteger(RexxDatabase.STATUS);
      }
      if (
         m_content.getAsString(RexxDatabase.TITLE).equals(title) &&
         m_content.getAsString(RexxDatabase.BODY).equals(body)
      ) {
         /*
         | Not modified: identical to what's in the database, or empty record.
         | Database does not contain empty records, hence:
         | - if m_id is null, it's not in the database, and won't be.
         | - otherwise, force the status to be saved.
         */
         if (
            (m_id != null) && //
            (m_content.getAsInteger(RexxDatabase.STATUS) != status)
         ) {
            m_id = m_rexxDb.saveScript(m_id, m_content);
         }
         return false;
      }else {
         return true;
      }
   }
}
/*===========================================================================*/
