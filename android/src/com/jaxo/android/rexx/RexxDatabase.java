/*
* $Id: RexxDatabase.java,v 1.8 2011-08-14 17:59:21 pgr Exp $
*
* (C) Copyright 2011 Jaxo Inc.  All rights reserved.
* This work contains confidential trade secrets of Jaxo.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo.
*
* Author:  Pierre G. Richard
* Written: 8/5/2011
*/
package com.jaxo.android.rexx;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Date;

import android.content.ContentValues;
import android.content.Context;
import android.content.res.AssetManager;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

/*-- class RexxDatabase --+
*//**
*
* @author  Pierre G. Richard
* @version $Id: RexxDatabase.java,v 1.8 2011-08-14 17:59:21 pgr Exp $
*/
public class RexxDatabase extends SQLiteOpenHelper
{
   public static final String _ID = "_id";
   public static final String TITLE = "title";          // TEXT
   public static final String BODY = "body";            // TEXT
   public static final String UPDATE_DATE = "mdate";    // INTEGER
   public static final String STATUS = "status";        // INTEGER
   public static final int STATUS_UNKNOWN = 0;
   public static final int STATUS_ERRONEOUS = 1;
   public static final int STATUS_COMPILED = 2;
   public static final String FLAGS = "flags";          // INTEGER

   private static final String SCRIPTS_TABLE_NAME = "procs";
   private static final String DATABASE_NAME = "rexx";
   private static final int DATABASE_VERSION = 1;
   private static final String ASSET_PATH = "rexx";

   private Context m_context;
   private SQLiteDatabase m_db;

   /*------------------------------------------------------------RexxDatabase-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   public RexxDatabase(Context context) throws SQLException
   {
      super(context, DATABASE_NAME, null, DATABASE_VERSION);
      m_context = context;
      m_db = getWritableDatabase();
   }

   @Override
   /*----------------------------------------------------------------onCreate-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   public void onCreate(SQLiteDatabase db) {
      createDb(db);
      try {
         populateDbFromAssets();
      }catch (Exception e) {
         e.printStackTrace();
      }
   }

   @Override
   /*---------------------------------------------------------------onUpgrade-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
      Log.w(
         "RexxDatabase",
         "Upgrading database from version " + oldVersion + " to " + newVersion
      );
      db.execSQL("DROP TABLE IF EXISTS " + SCRIPTS_TABLE_NAME);
      createDb(db);
   }

   /*----------------------------------------------------------------createDb-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   private void createDb(SQLiteDatabase db) {
      /* >>>PGR:
      | Following assignment: "m_db = db" is required: don't remove!
      | - If you think you can remove it HERE, b/c m_db is also set in the
      |   constructor, you are wrong.
      |   At the very first time, the DB is created:
      |   getWritableDataBase() -> onCreate() -> populate() -> saveScript()
      |   => saveScript *needs* m_db to be set
      | - If you think you can remove it in the constructor, wrong again
      |   When the DB is already created, onCreate is *not* called
      */
      m_db = db;
      db.execSQL(
         "CREATE TABLE IF NOT EXISTS " + SCRIPTS_TABLE_NAME + " (" +
         _ID + " INTEGER PRIMARY KEY AUTOINCREMENT," +
         TITLE + " TEXT NOT NULL," +
         UPDATE_DATE + " INTEGER, " +
         BODY + " TEXT NOT NULL," +
         STATUS + " INTEGER DEFAULT 0," +
         FLAGS + " INTEGER DEFAULT 0);"
      );
   }

   /*------------------------------------------------------------queryScripts-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   public Cursor queryScripts() {
      return m_db.query(
         SCRIPTS_TABLE_NAME,
         null, // new String[] {_ID, TITLE, BODY, UPDATE_DATE, FLAGS },
         null, null, null, null, null
      );
   }

   /*-------------------------------------------------------------queryScript-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   public ContentValues queryScript(long id) throws SQLException
   {
      Cursor cursor = m_db.query(
         true,
         SCRIPTS_TABLE_NAME,
         null, // new String[] { _ID, TITLE, BODY, UPDATE_DATE, FLAGS },
         _ID + "=" + id,
         null, null, null, null, null
      );
      ContentValues values = new ContentValues();
      cursor.moveToFirst();
      values.put(
         TITLE,
         cursor.getString(cursor.getColumnIndexOrThrow(TITLE))
      );
      values.put(
         BODY,
         cursor.getString(cursor.getColumnIndexOrThrow(BODY))
      );
      values.put(
         UPDATE_DATE,
         cursor.getLong(cursor.getColumnIndexOrThrow(UPDATE_DATE))
      );
      values.put(
         FLAGS,
         cursor.getInt(cursor.getColumnIndexOrThrow(FLAGS))
      );
      values.put(
         STATUS,
         cursor.getInt(cursor.getColumnIndexOrThrow(STATUS))
      );
      cursor.close();
      return values;
   }

   /*--------------------------------------------------------------saveScript-+
   *//**
   * Save a Rexx Script
   *//*
   +-------------------------------------------------------------------------*/
   public Long saveScript(Long id, ContentValues values) {
      if (id == null) {
         return m_db.insert(SCRIPTS_TABLE_NAME, null, values);
      }else if (m_db.update(SCRIPTS_TABLE_NAME, values, _ID + "=" + id, null) > 0) {
         return id;
      }else {
         return null;
      }
   }

   /*------------------------------------------------------------deleteScript-+
   *//**
   * Delete a Rexx script, given its id
   *//*
   +-------------------------------------------------------------------------*/
   public boolean deleteScript(long id) {
      return m_db.delete(SCRIPTS_TABLE_NAME, _ID + "=" + id, null) > 0;
   }

   /*----------------------------------------------------populateDbFromAssets-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   public void populateDbFromAssets() throws IOException {
      AssetManager manager = m_context.getAssets();
      ContentValues values = new ContentValues();
      String[] names = manager.list(ASSET_PATH);
      for (int i=0; i < names.length; ++i) {
         BufferedReader reader = new BufferedReader(
            new InputStreamReader(manager.open(ASSET_PATH + '/' + names[i]))
         );
         String title = reader.readLine();
         title = title.substring(2, title.length()-2).trim();
         StringBuffer bodyBuffer = new StringBuffer();
         String bodyLine;
         while (null != (bodyLine = reader.readLine())) {
            bodyBuffer.append(bodyLine);
            bodyBuffer.append('\n');
         }
         values.put(RexxDatabase.UPDATE_DATE, new Date().getTime());
         values.put(RexxDatabase.TITLE, title);
         values.put(RexxDatabase.BODY, bodyBuffer.toString());
         saveScript(null, values);
         values.clear();
      }
   }

   /*-----------------------------------------------------statusToBulletImage-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   public static int statusToBulletImage(int status) {
      switch (status) {
      case STATUS_UNKNOWN: return R.drawable.yellow_bullet;
      case STATUS_COMPILED: return R.drawable.green_bullet;
      // case RexxDatabase.STATUS_ERRONEOUS:
      default: return R.drawable.red_bullet;
      }
   }
}

/*===========================================================================*/
