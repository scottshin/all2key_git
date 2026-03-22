// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "TableDictionaryEngine.h"
#include "DictionarySearch.h"

//+---------------------------------------------------------------------------
//
// CollectWord
//
//----------------------------------------------------------------------------

VOID CTableDictionaryEngine::CollectWord(_In_ CStringRange *pKeyCode, _Inout_ CSampleImeArray<CStringRange> *pWordStrings)
{
    if (_useJson)
    {
        // Use JSON dictionary
        std::wstring key(pKeyCode->Get(), pKeyCode->GetLength());
        auto it = _jsonDictionary.find(key);
        if (it != _jsonDictionary.end())
        {
            for (const auto& value : it->second)
            {
                CStringRange* pPhrase = pWordStrings->Append();
                if (pPhrase)
                {
                    pPhrase->Set(value.c_str(), value.length());
                }
            }
        }
    }
    else
    {
        // Use text file
        CDictionaryResult* pdret = nullptr;
        CDictionarySearch dshSearch(_locale, _pDictionaryFile, pKeyCode);

        while (dshSearch.FindPhrase(&pdret))
        {
            for (UINT index = 0; index < pdret->_FindPhraseList.Count(); index++)
            {
                CStringRange* pPhrase = nullptr;
                pPhrase = pWordStrings->Append();
                if (pPhrase)
                {
                    *pPhrase = *pdret->_FindPhraseList.GetAt(index);
                }
            }

            delete pdret;
            pdret = nullptr;
        }
    }
}

VOID CTableDictionaryEngine::CollectWord(_In_ CStringRange *pKeyCode, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList)
{
    if (_useJson)
    {
        // Use JSON dictionary
        std::wstring key(pKeyCode->Get(), pKeyCode->GetLength());
        auto it = _jsonDictionary.find(key);
        if (it != _jsonDictionary.end())
        {
            for (const auto& value : it->second)
            {
                CCandidateListItem* pLI = nullptr;
                pLI = pItemList->Append();
                if (pLI)
                {
                    pLI->_ItemString.Set(value.c_str(), value.length());
                    pLI->_FindKeyCode.Set(pKeyCode->Get(), pKeyCode->GetLength());
                }
            }
        }
    }
    else
    {
        // Use text file
        CDictionaryResult* pdret = nullptr;
        CDictionarySearch dshSearch(_locale, _pDictionaryFile, pKeyCode);

        while (dshSearch.FindPhrase(&pdret))
        {
            for (UINT iIndex = 0; iIndex < pdret->_FindPhraseList.Count(); iIndex++)
            {
                CCandidateListItem* pLI = nullptr;
                pLI = pItemList->Append();
                if (pLI)
                {
                    pLI->_ItemString.Set(*pdret->_FindPhraseList.GetAt(iIndex));
                    pLI->_FindKeyCode.Set(pdret->_FindKeyCode.Get(), pdret->_FindKeyCode.GetLength());
                }
            }

            delete pdret;
            pdret = nullptr;
        }
    }
}

//+---------------------------------------------------------------------------
//
// CollectWordForWildcard
//
//----------------------------------------------------------------------------

VOID CTableDictionaryEngine::CollectWordForWildcard(_In_ CStringRange *pKeyCode, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList)
{
    CDictionaryResult* pdret = nullptr;
    CDictionarySearch dshSearch(_locale, _pDictionaryFile, pKeyCode);

    while (dshSearch.FindPhraseForWildcard(&pdret))
    {
        for (UINT iIndex = 0; iIndex < pdret->_FindPhraseList.Count(); iIndex++)
        {
            CCandidateListItem* pLI = nullptr;
            pLI = pItemList->Append();
            if (pLI)
            {
                pLI->_ItemString.Set(*pdret->_FindPhraseList.GetAt(iIndex));
                pLI->_FindKeyCode.Set(pdret->_FindKeyCode.Get(), pdret->_FindKeyCode.GetLength());
            }
        }

        delete pdret;
        pdret = nullptr;
    }
}

//+---------------------------------------------------------------------------
//
// CollectWordFromConvertedStringForWildcard
//
//----------------------------------------------------------------------------

VOID CTableDictionaryEngine::CollectWordFromConvertedStringForWildcard(_In_ CStringRange *pString, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList)
{
    CDictionaryResult* pdret = nullptr;
    CDictionarySearch dshSearch(_locale, _pDictionaryFile, pString);

    while (dshSearch.FindConvertedStringForWildcard(&pdret)) // TAIL ALL CHAR MATCH
    {
        for (UINT index = 0; index < pdret->_FindPhraseList.Count(); index++)
        {
            CCandidateListItem* pLI = nullptr;
            pLI = pItemList->Append();
            if (pLI)
            {
                pLI->_ItemString.Set(*pdret->_FindPhraseList.GetAt(index));
                pLI->_FindKeyCode.Set(pdret->_FindKeyCode.Get(), pdret->_FindKeyCode.GetLength());
            }
        }

        delete pdret;
        pdret = nullptr;
    }
}

//+---------------------------------------------------------------------------
//
// LoadDictionary
//
//----------------------------------------------------------------------------

VOID CTableDictionaryEngine::LoadDictionary()
{
    if (!_pDictionaryFile)
        return;
#if 1
    // Check if file is JSON (simple check by extension or content)
    // For simplicity, assume if file size is small and contains '{', it's JSON
    DWORD_PTR fileSize = _pDictionaryFile->GetFileSize();
/*
    if (fileSize > 1000000) // Large file, likely text
    {
        _useJson = FALSE;
        return;
    }
*/
    WCHAR firstChar = L'\0';
    DWORD bytesRead = 0;
    if (_pDictionaryFile->Read(&firstChar, sizeof(WCHAR), &bytesRead) && firstChar == L'{')
    {
        // Try to load as JSON
        _pDictionaryFile->Seek(0, FILE_BEGIN); // Reset to beginning
        CDictionaryParser parser(_locale);
        if (parser.LoadJsonDictionary(_pDictionaryFile, _jsonDictionary))
        {
            _useJson = TRUE;
        }
        else
        {
            _useJson = FALSE;
        }
    }
    else
    {
        _useJson = FALSE;
    }

    // Reset file position
    _pDictionaryFile->Seek(0, FILE_BEGIN);
#endif
}

