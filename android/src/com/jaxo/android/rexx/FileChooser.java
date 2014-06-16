/*
* (C) Copyright 2014 Jaxo Inc.  All rights reserved.
* This work contains confidential trade secrets of Jaxo.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo.
*
* Author:  Pierre G. Richard
* Written: 6/14/2014
*/
package com.jaxo.android.rexx;

import java.io.File;
import java.io.FileFilter;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Stack;

import android.app.ListActivity;
import android.content.Context;
import android.os.Bundle;
import android.os.Environment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

/*-- class FileChooser --*/
public class FileChooser extends ListActivity {
   private Stack<File> m_dirs;
   private FileArrayAdapter m_adapter;

   @Override
   /*----------------------------------------------------------------onCreate-+
   *//**
   +-------------------------------------------------------------------------*/
   public void onCreate(Bundle savedInstanceState) {
      super.onCreate(savedInstanceState);
      getListView().setDividerHeight(0);
      m_dirs = new Stack<File>();
      push(
         Environment.getExternalStoragePublicDirectory(
            Environment.DIRECTORY_DOWNLOADS
         )
      );
   }

   @Override
   /*-----------------------------------------------------------onBackPressed-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   public void onBackPressed() {
      m_dirs.pop();
      if (m_dirs.size() > 0) {
         populate();
      }else {
         super.onBackPressed();
      }
   }

   /*--------------------------------------------------------------------push-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   private void push(File dir) {
      m_dirs.push(dir);
      populate();
   }

   /*----------------------------------------------------------------populate-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   private void populate() {
      StringBuilder sb = new StringBuilder();
      File dir = m_dirs.peek();
      if (!dir.isDirectory()) {
         sb.append('\"').append(dir.getName()).append("\" is not a directory");
      }else {
         List<File> list = Arrays.asList(
            dir.listFiles(
               new FileFilter() {
                  public boolean accept(File file) { return true; }
               }
            )
         );
         if (list.isEmpty()) {
            sb.append("Directory \"").append(dir.getName()).append("\" is empty");
         }else {
            for (int i=0, max=m_dirs.size(); i < max; ++i) {
               sb.append(m_dirs.elementAt(i).getName()).append("/");
            }
            Collections.sort(list);
            m_adapter = new FileArrayAdapter(
               this, R.layout.files_list_item, list
            );
            setListAdapter(m_adapter);
            setTitle(sb.toString());
            sb = null;
         }
      }
      if (sb != null) {
         Toast.makeText(this, sb.toString(), Toast.LENGTH_LONG).show();
      }
   }

   @Override
   /*---------------------------------------------------------onListItemClick-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   protected void onListItemClick(ListView l, View v, int position, long id) {
      super.onListItemClick(l, v, position, id);
      File file = m_adapter.getItem(position);
      if (file.isDirectory()) {
         push(new File(file.getPath()));
      }else {
         onFileChosen(file);
      }
   }

   /*------------------------------------------------------------onFileChosen-+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   private void onFileChosen(File file) {
      Toast.makeText(
         this, "chosen file: " + file.getName(),
         Toast.LENGTH_SHORT
      ).show();
   }

   /*------------------------------------------------ class FileArrayAdapter -+
   *//**
   *//*
   +-------------------------------------------------------------------------*/
   static class FileArrayAdapter extends ArrayAdapter<File> {
      public FileArrayAdapter(Context context, int res, List<File> objects) {
         super(context, res, objects);
      }
      @Override
      public View getView(int position, View view, ViewGroup parent) {
         File file = getItem(position);
         if (view == null) {
            view = LayoutInflater.from(getContext()).inflate(
               R.layout.files_list_item, parent, false
            );
         }
         ((TextView)view.findViewById(R.id.label)).setText(file.getName());
         if (file.isDirectory()) {
            ((ImageView)view.findViewById(R.id.icon)).setImageResource(
               R.drawable.folder_icon
            );
         }
         return view;
      }
   }
}
/*===========================================================================*/
