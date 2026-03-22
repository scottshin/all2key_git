// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "DictionaryParser.h"
#include "SampleIMEBaseStructure.h"

//---------------------------------------------------------------------
//
// ctor
//
//---------------------------------------------------------------------

CDictionaryParser::CDictionaryParser(LCID locale)
{
    _locale = locale;
}

//---------------------------------------------------------------------
//
// dtor
//
//---------------------------------------------------------------------

CDictionaryParser::~CDictionaryParser()
{
}

//---------------------------------------------------------------------
//
// ParseLine
//
// dwBufLen - in character count
//
//---------------------------------------------------------------------

BOOL CDictionaryParser::ParseLine(_In_reads_(dwBufLen) LPCWSTR pwszBuffer, DWORD_PTR dwBufLen, _Out_ CParserStringRange *psrgKeyword, _Inout_opt_ CSampleImeArray<CParserStringRange> *pValue)
{
    LPCWSTR pwszKeyWordDelimiter = nullptr;
    pwszKeyWordDelimiter = GetToken(pwszBuffer, dwBufLen, Global::KeywordDelimiter, psrgKeyword);
    if (!(pwszKeyWordDelimiter))
    {
        return FALSE;    // End of file
    }

    dwBufLen -= (pwszKeyWordDelimiter - pwszBuffer);
    pwszBuffer = pwszKeyWordDelimiter + 1;
    dwBufLen--;

    // Get value.
    if (pValue)
    {
        if (dwBufLen)
        {
            CParserStringRange* psrgValue = pValue->Append();
            if (!psrgValue)
            {
                return FALSE;
            }
            psrgValue->Set(pwszBuffer, dwBufLen);
            RemoveWhiteSpaceFromBegin(psrgValue);
            RemoveWhiteSpaceFromEnd(psrgValue);
            RemoveStringDelimiter(psrgValue);
        }
    }

    return TRUE;
}

//---------------------------------------------------------------------
//
// GetToken
//
// dwBufLen - in character count
//
// return   - pointer of delimiter which specified chDelimiter
//
//---------------------------------------------------------------------
_Ret_maybenull_
LPCWSTR CDictionaryParser::GetToken(_In_reads_(dwBufLen) LPCWSTR pwszBuffer, DWORD_PTR dwBufLen, _In_ const WCHAR chDelimiter, _Out_ CParserStringRange *psrgValue)
{
    WCHAR ch = '\0';

    psrgValue->Set(pwszBuffer, dwBufLen);

    ch = *pwszBuffer;
    while ((ch) && (ch != chDelimiter) && dwBufLen)
    {
        dwBufLen--;
        pwszBuffer++;

        if (ch == Global::StringDelimiter)
        {
            while (*pwszBuffer && (*pwszBuffer != Global::StringDelimiter) && dwBufLen)
            {
                dwBufLen--;
                pwszBuffer++;
            }
            if (*pwszBuffer && dwBufLen)
            {
                dwBufLen--;
                pwszBuffer++;
            }
            else
            {
                return nullptr;
            }
        }
        ch = *pwszBuffer;
    }

    if (*pwszBuffer && dwBufLen)
    {
        LPCWSTR pwszStart = psrgValue->Get();

        psrgValue->Set(pwszStart, pwszBuffer - pwszStart);

        RemoveWhiteSpaceFromBegin(psrgValue);
        RemoveWhiteSpaceFromEnd(psrgValue);
        RemoveStringDelimiter(psrgValue);

        return pwszBuffer;
    }

    RemoveWhiteSpaceFromBegin(psrgValue);
    RemoveWhiteSpaceFromEnd(psrgValue);
    RemoveStringDelimiter(psrgValue);

    return nullptr;
}

//---------------------------------------------------------------------
//
// RemoveWhiteSpaceFromBegin
// RemoveWhiteSpaceFromEnd
// RemoveStringDelimiter
//
//---------------------------------------------------------------------

BOOL CDictionaryParser::RemoveWhiteSpaceFromBegin(_Inout_opt_ CStringRange *pString)
{
    DWORD_PTR dwIndexTrace = 0;  // in char

    if (pString == nullptr)
    {
        return FALSE;
    }

    if (SkipWhiteSpace(_locale, pString->Get(), pString->GetLength(), &dwIndexTrace) != S_OK)
    {
        return FALSE;
    }

    pString->Set(pString->Get() + dwIndexTrace, pString->GetLength() - dwIndexTrace);
    return TRUE;
}

BOOL CDictionaryParser::RemoveWhiteSpaceFromEnd(_Inout_opt_ CStringRange *pString)
{
    if (pString == nullptr)
    {
        return FALSE;
    }

    DWORD_PTR dwTotalBufLen = pString->GetLength();
    LPCWSTR pwszEnd = pString->Get() + dwTotalBufLen - 1;

    while (dwTotalBufLen && (IsSpace(_locale, *pwszEnd) || *pwszEnd == L'\r' || *pwszEnd == L'\n'))
    {
        pwszEnd--;
        dwTotalBufLen--;
    }

    pString->Set(pString->Get(), dwTotalBufLen);
    return TRUE;
}

BOOL CDictionaryParser::RemoveStringDelimiter(_Inout_opt_ CStringRange *pString)
{
    if (pString == nullptr)
    {
        return FALSE;
    }

    if (pString->GetLength() >= 2)
    {
        if ((*pString->Get() == Global::StringDelimiter) && (*(pString->Get()+pString->GetLength()-1) == Global::StringDelimiter))
        {
            pString->Set(pString->Get()+1, pString->GetLength()-2);
            return TRUE;
        }
    }

    return FALSE;
}

//---------------------------------------------------------------------
//
// GetOneLine
//
// dwBufLen - in character count
//
//---------------------------------------------------------------------

DWORD_PTR CDictionaryParser::GetOneLine(_In_z_ LPCWSTR pwszBuffer, DWORD_PTR dwBufLen)
{
    DWORD_PTR dwIndexTrace = 0;     // in char

    if (FAILED(FindChar(L'\r', pwszBuffer, dwBufLen, &dwIndexTrace)))
    {
        if (FAILED(FindChar(L'\0', pwszBuffer, dwBufLen, &dwIndexTrace)))
        {
            return dwBufLen;
        }
    }

    return dwIndexTrace;
}

//---------------------------------------------------------------------
//
// LoadJsonDictionary
//
//---------------------------------------------------------------------
#if 1
BOOL CDictionaryParser::LoadJsonDictionary(_In_ CFile *pFile, _Out_ std::map<std::wstring, std::vector<std::wstring>> &dictionary)
{
    if (!pFile)
        return FALSE;

    // Read entire file
    DWORD_PTR fileSize = pFile->GetFileSize();
    if (fileSize == 0)
        return FALSE;

    WCHAR *buffer = new (std::nothrow) WCHAR[fileSize + 1];
    if (!buffer)
        return FALSE;

    DWORD bytesRead = 0;
    if (!pFile->Read(buffer, fileSize, &bytesRead))
    {
        delete[] buffer;
        return FALSE;
    }
    buffer[fileSize] = L'\0';

    // Simple JSON parser for {"key": ["val1", "val2"], ...}
    std::wstring json(buffer);
    delete[] buffer;

    size_t pos = 0;
    if (json[pos] != L'{')
        return FALSE;
    pos++;

    while (pos < json.size())
    {
        // Skip whitespace
        while (pos < json.size() && (json[pos] == L' ' || json[pos] == L'\t' || json[pos] == L'\n' || json[pos] == L'\r'))
            pos++;

        if (json[pos] == L'}')
            break;

        // Parse key
        if (json[pos] != L'"')
            return FALSE;
        pos++;
        size_t keyStart = pos;
        while (pos < json.size() && json[pos] != L'"')
            pos++;
        if (pos >= json.size())
            return FALSE;
        std::wstring key = json.substr(keyStart, pos - keyStart);
        pos++; // skip "

        // Skip to :
        while (pos < json.size() && json[pos] != L':')
            pos++;
        if (pos >= json.size())
            return FALSE;
        pos++;

        // Parse array
        if (json[pos] != L'[')
            return FALSE;
        pos++;

        std::vector<std::wstring> values;
        while (pos < json.size())
        {
            // Skip whitespace
            while (pos < json.size() && (json[pos] == L' ' || json[pos] == L'\t' || json[pos] == L'\n' || json[pos] == L'\r'))
                pos++;

            if (json[pos] == L']')
            {
                pos++;
                break;
            }

            if (json[pos] != L'"')
                return FALSE;
            pos++;
            size_t valStart = pos;
            while (pos < json.size() && json[pos] != L'"')
                pos++;
            if (pos >= json.size())
                return FALSE;
            std::wstring value = json.substr(valStart, pos - valStart);
            values.push_back(value);
            pos++; // skip "

            // Skip comma or ]
            while (pos < json.size() && (json[pos] == L' ' || json[pos] == L'\t' || json[pos] == L'\n' || json[pos] == L'\r' || json[pos] == L','))
                pos++;
        }

        dictionary[key] = values;

        // Skip comma or }
        while (pos < json.size() && (json[pos] == L' ' || json[pos] == L'\t' || json[pos] == L'\n' || json[pos] == L'\r' || json[pos] == L','))
            pos++;
    }

    return TRUE;
}

#endif