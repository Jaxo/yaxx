/*
 * yaxx.h
 *
 * header file for yaxx
 *
 * This wizard-generated code is based on code adapted from the
 * stationery files distributed as part of the Palm OS SDK 4.0.
 *
 * Copyright (c) 1999-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 */
 
#ifndef YAXX_H_
#define YAXX_H_

/*********************************************************************
 * Internal Structures
 *********************************************************************/

typedef struct yaxxPreferenceType
{
	UInt8 replaceme;
} yaxxPreferenceType;

/*********************************************************************
 * Global variables
 *********************************************************************/

extern yaxxPreferenceType g_prefs;

/*********************************************************************
 * Internal Constants
 *********************************************************************/

#define appFileCreator			'STRT'
#define appName					"yaxx"
#define appVersionNum			0x01
#define appPrefID				0x00
#define appPrefVersionNum		0x01

/*********************************************************************
 * Helper template functions
 *********************************************************************/

/* use this template like:
 * ControlType *button; GetObjectPtr(button, MainOKButton); */

template <class T>
void GetObjectPtr(typename T * &ptr, UInt16 id)
{
	FormType * frmP;

	frmP = FrmGetActiveForm();
	ptr = static_cast<T *>(
		FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, id)));
}

/* use this template like:
 * ControlType *button = 
 *     GetObjectPtr<ControlType>(MainOKButton); */

template <class T>
typename T * GetObjectPtr(UInt16 id)
{
	FormType * frmP;

	frmP = FrmGetActiveForm();
	return static_cast<T *>(
		FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, id)));
}

#endif /* YAXX_H_ */
