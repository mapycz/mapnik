/*
 *******************************************************************************
 *
 *   Copyright (C) 1999-2003, International Business Machines
 *   Corporation and others.  All Rights Reserved.
 *
 *******************************************************************************
 *   file name:  scrptrun.h
 *
 *   created on: 10/17/2001
 *   created by: Eric R. Mader
 *
 * NOTE: This file is copied from ICU.
 * http://source.icu-project.org/repos/icu/icu/trunk/license.html
 */

#ifndef __SCRPTRUN_H
#define __SCRPTRUN_H

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <unicode/utypes.h>
#include <unicode/uobject.h>
#include <unicode/uscript.h>
#pragma GCC diagnostic pop

struct ScriptRecord
{
    UChar32 startChar = 0;
    UChar32 endChar = 0;
    UScriptCode scriptCode = USCRIPT_INVALID_CODE;
};

struct ParenStackEntry
{
    int32_t pairIndex = 0;
    UScriptCode scriptCode = USCRIPT_INVALID_CODE;
};

class ScriptRun {
public:
    ScriptRun(const UChar chars[], int32_t length);

    int32_t getScriptStart();

    int32_t getScriptEnd();

    UScriptCode getScriptCode();

    UBool next();

private:
    static UBool sameScript(int32_t scriptOne, int32_t scriptTwo);

    int32_t charStart;
    int32_t charLimit;
    const UChar *charArray;

    int32_t scriptStart;
    int32_t scriptEnd;
    UScriptCode scriptCode;

    ParenStackEntry parenStack[128];
    int32_t parenSP;

    static int8_t highBit(int32_t value);
    static int32_t getPairIndex(UChar32 ch);

    static UChar32 pairedChars[];
    static const int32_t pairedCharCount;
    static const int32_t pairedCharPower;
    static const int32_t pairedCharExtra;
};

inline ScriptRun::ScriptRun(const UChar chars[], int32_t length)
{
    charArray = chars;

    charStart = 0;
    charLimit = length;

    scriptStart = charStart;
    scriptEnd   = charStart;
    scriptCode  = USCRIPT_INVALID_CODE;
    parenSP     = -1;
}

inline int32_t ScriptRun::getScriptStart()
{
    return scriptStart;
}

inline int32_t ScriptRun::getScriptEnd()
{
    return scriptEnd;
}

inline UScriptCode ScriptRun::getScriptCode()
{
    return scriptCode;
}

#endif
