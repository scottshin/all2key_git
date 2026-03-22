// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "EditSession.h"
#include "SampleIME.h"
#include "CandidateListUIPresenter.h"
#include "CompositionProcessorEngine.h"

//////////////////////////////////////////////////////////////////////
//
// CSampleIME class
//
//////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
// _IsRangeCovered
//
// Returns TRUE if pRangeTest is entirely contained within pRangeCover.
//
//----------------------------------------------------------------------------

BOOL CSampleIME::_IsRangeCovered(TfEditCookie ec, _In_ ITfRange *pRangeTest, _In_ ITfRange *pRangeCover)
{
    LONG lResult = 0;;

    if (FAILED(pRangeCover->CompareStart(ec, pRangeTest, TF_ANCHOR_START, &lResult)) 
        || (lResult > 0))
    {
        return FALSE;
    }

    if (FAILED(pRangeCover->CompareEnd(ec, pRangeTest, TF_ANCHOR_END, &lResult)) 
        || (lResult < 0))
    {
        return FALSE;
    }

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// _DeleteCandidateList
//
//----------------------------------------------------------------------------

VOID CSampleIME::_DeleteCandidateList(BOOL isForce, _In_opt_ ITfContext *pContext)
{

    DebugLogFile( L"###  IME- _DeleteCandidateList() -->\n");

    isForce;pContext;

    CCompositionProcessorEngine* pCompositionProcessorEngine = nullptr;
    pCompositionProcessorEngine = _pCompositionProcessorEngine;
    pCompositionProcessorEngine->PurgeVirtualKey();

    if (_pCandidateListUIPresenter)
    {

        DebugLogFile( L"  -> CandidateList Close \n" );

        _pCandidateListUIPresenter->_EndCandidateList();

#if 1 // my  PPP

        _pCandidateListUIPresenter = 0;

#endif



        _candidateMode = CANDIDATE_NONE;
        _isCandidateWithWildcard = FALSE;
    }
}

//+---------------------------------------------------------------------------
//
// _HandleComplete
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleComplete(TfEditCookie ec, _In_ ITfContext *pContext)
{
    DebugLogFile( L"###  IME-HandleComplete() --> clear lastChar.. set 0\n\n\n");


    _DeleteCandidateList(FALSE, pContext);

    // just terminate the composition
    _TerminateComposition(ec, pContext);


#if 1 // my
    // reset
    lastChar = 0;
#endif


    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCancel
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleCancel(TfEditCookie ec, _In_ ITfContext *pContext)
{


    DebugLogFile( L"###  IME-  _HandleCancel() --> clear lastChar.. set 0\n\n\n");

    _RemoveDummyCompositionForComposing(ec, _pComposition);

    _DeleteCandidateList(FALSE, pContext);

    _TerminateComposition(ec, pContext);


#if 1 // my
    // reset
    lastChar = 0;
#endif


    return S_OK;
}


wchar_t table_alpha[] = {
    0x304B, 0x3063, 0x3060, 0x305F, 0x3089, 0x3007, 0x306F,
    0x3048, 0x3046, 0x3042, 0x3044, 0x3046, 0x3092, 0x304A, 0x3088, 0x308F,
    0x3094, 0x307E, 0x3055, 0x3071, 0x3084, 0x3070, 0x306A, 0x3056, 0x3093, 0x304C

	// L'か', L'っ', L'だ', L'た', L'ら', L'〇', L'は',					// a, b, c, d, e, f, g 
	// L'え', L'う', L'あ', L'い', L'う', L'を', L'お', L'よ', L'わ',		//h, i, j, k, l, m, n, o, p,
	// L'ゔ', L'ま', L'さ', L'ぱ', L'や', L'ば', L'な', L'ざ', L'ん', L'が'//qrstuvwxyz
};

const wchar_t* conv_table[] = {
    L"\u304B", L"\u304D", L"\u304F", L"\u3051", L"\u3053", L"\u304D\u3083", L"\u304D\u3085", L"\u304D\u3087", // か,き,く,け,こ,きゃ,きゅ,きょ

    L"\u304C", L"\u304E", L"\u3050", L"\u3052", L"\u3054", L"\u304E\u3083", L"\u304E\u3085", L"\u304E\u3087", // が,ぎ,ぐ,げ,ご,ぎゃ,ぎゅ,ぎょ

    L"\u3055", L"\u3057", L"\u3059", L"\u305B", L"\u305D", L"\u3057\u3083", L"\u3057\u3085", L"\u3057\u3087", // さ,し,す,せ,そ,しゃ,しゅ,しょ

    L"\u3056", L"\u3058", L"\u305A", L"\u305C", L"\u305E", L"\u3058\u3083", L"\u3058\u3085", L"\u3058\u3087",  // ざ,じ,ず, ぜ,  ぞ, じゃ, じゅ, じょ 



    L"\u305F",                 // た
    L"\u3061",                 // ち
    L"\u3064",                 // つ
    L"\u3066",                 // て
    L"\u3068",                 // と
    L"\u3061\u3083",           // ちゃ
    L"\u3061\u3085",           // ちゅ
    L"\u3061\u3087",           // ちょ

    L"\u3060",                 // だ
    L"\u3062",                 // ぢ
    L"\u3065",                 // づ
    L"\u3067",                 // で
    L"\u3069",                 // ど
    L"\u3062\u3083",           // ぢゃ
    L"\u3062\u3085",           // ぢゅ
    L"\u3062\u3087",           // ぢょ

    L"\u306A",                 // な
    L"\u306B",                 // に
    L"\u306C",                 // ぬ
    L"\u306D",                 // ね
    L"\u306E",                 // の
    L"\u306B\u3083",           // にゃ
    L"\u306B\u3085",           // にゅ
    L"\u306B\u3087",           // にょ

    L"\u306F",                 // は
    L"\u3072",                 // ひ
    L"\u3075",                 // ふ
    L"\u3078",                 // へ
    L"\u307B",                 // ほ
    L"\u3072\u3083",           // ひゃ
    L"\u3072\u3085",           // ひゅ
    L"\u3072\u3087",           // ひょ

    L"\u3070",                 // ば
    L"\u3073",                 // び
    L"\u3076",                 // ぶ
    L"\u3079",                 // べ
    L"\u307C",                 // ぼ
    L"\u3073\u3083",           // びゃ
    L"\u3073\u3085",           // びゅ
    L"\u3073\u3087",           // びょ

    L"\u3071",                 // ぱ
    L"\u3074",                 // ぴ
    L"\u3077",                 // ぷ
    L"\u307A",                 // ぺ
    L"\u307D",                 // ぽ
    L"\u3074\u3083",           // ぴゃ
    L"\u3074\u3085",           // ぴゅ
    L"\u3074\u3087",           // ぴょ

    L"\u307E",                 // ま
    L"\u307F",                 // み
    L"\u3080",                 // む
    L"\u3081",                 // め
    L"\u3082",                 // も
    L"\u307F\u3083",           // みゃ
    L"\u307F\u3085",           // みゅ
    L"\u307F\u3087",           // みょ

    L"\u3089",                 // ら
    L"\u308A",                 // り
    L"\u308B",                 // る
    L"\u308C",                 // れ
    L"\u308D",                 // ろ
    L"\u308A\u3083",           // りゃ
    L"\u308A\u3085",           // りゅ
    L"\u308A\u3087",           // りょ

    L"\u3094\u3041",           // ゔぁ
    L"\u3094\u3043",           // ゔぃ
    L"\u3094",                 // ゔ
    L"\u3094\u3047",           // ゔぇ
    L"\u3094\u3049",           // ゔぉ
    L"\u3094\u3083",           // ゔゃ
    L"\u3094\u3085",           // ゔゅ
    L"\u3094\u3087",           // ゔょ

    L"\u3042", L"\u3044", L"\u3046", L"\u3048", L"\u304A", L"\u3084", L"\u3086", L"\u3088"                 // あ,い,う,え,お,や,ゆ,よ
    
};



bool is_first(int ascii)
{
	return (
		'q' == ascii ||
		'w' == ascii || 'e' == ascii || 'r' == ascii || 't' == ascii ||
		'a' == ascii || 's' == ascii || 'd' == ascii || 'f' == ascii || 'g' == ascii ||
		'z' == ascii || 'x' == ascii || 'c' == ascii || 'v' == ascii);
}

bool is_second(int ascii)
{
	return (
		'j' == ascii || 
		'u' == ascii || 'i' == ascii || 'o' == ascii || 'p' == ascii ||
		'h' == ascii || 'k' == ascii || 'l' == ascii ||
		'n' == ascii || 'm' == ascii);
}

bool is_shfited( int ascii )
{
    return (
        'B' == ascii || 'N' == ascii || 'M' == ascii || 'H' == ascii || 'J' == ascii || 
        'K' == ascii || 'L' == ascii || 'Y' == ascii || 'U' == ascii || 
        'I' == ascii || 'O' == ascii || 'P' == ascii );
}

WCHAR get_shift_convert ( int shift_key )
{
    WCHAR ret = 0x0;
    switch (shift_key)
    {
        case 'B': ret = 0x3063; break; // っ
        case 'N': ret = 0x3049; break; // ぉ
        case 'M': ret = 0x3092; break; //を
        case 'H': ret = 0x3047; break; //ぇ
        case 'J': ret = 0x3041; break; //ぁ
        case 'K': ret = 0x3043; break; //ぃ
        case 'L': ret = 0x3045; break; //ぅ
        case 'Y': ret = 0x3093; break; //ん
        case 'U': ret = 0x3083; break; //ゃ
        case 'I': ret = 0x3085; break; //ゅ
        case 'O': ret = 0x3087; break; //ょ
        case 'P': ret = 0x308E; break; //ゎ
    }

	return (ret);

}

WCHAR get_convert(WCHAR lastChar, int vowel_asc_key)
{
	int inx = 0;

	WCHAR arr[] = { 'j', 'k', 'l', 'h', 'n', 'u', 'i', 'o' };       // 후타. 
	for (int i = 0; i <= 7; i++)
	{
		if (vowel_asc_key == arr[i])
		{
			inx = i;
			break;
		}
	}

    WCHAR ret = 0x0;
    switch (lastChar)
    {
        case 0x304B: /* か */ ret = conv_table[ 0 + inx][0]; break;
        case 0x304C: /* が */ ret = conv_table[ 8 + inx][0]; break;
        case 0x3055: /* さ */ ret = conv_table[16 + inx][0]; break;
        case 0x3056: /* ざ */ ret = conv_table[24 + inx][0]; break;
        case 0x305F: /* た */ ret = conv_table[32 + inx][0]; break;
        case 0x3060: /* だ */ ret = conv_table[40 + inx][0]; break;
        case 0x306A: /* な */ ret = conv_table[48 + inx][0]; break;
        case 0x306F: /* は */ ret = conv_table[56 + inx][0]; break;
        case 0x3070: /* ば */ ret = conv_table[64 + inx][0]; break;
        case 0x3071: /* ぱ */ ret = conv_table[72 + inx][0]; break;
        case 0x307E: /* ま */ ret = conv_table[80 + inx][0]; break;
        case 0x3089: /* ら */ ret = conv_table[88 + inx][0]; break;
        case 0x3094: /* ゔ */ ret = conv_table[96 + inx][0]; break;
        case 0x3007: /* 〇 */ ret = table_alpha[vowel_asc_key - 'a']; break;
    }

	return (ret);
}



WCHAR FirstToHiragana(WCHAR consonant)
{
    if (consonant == L'k')
        return 0x304B; // か
    else 
    if (consonant == L's')
        return 0x3055; // さ
    else 
    if (consonant == L't')
        return 0x305F; // た
    return 0;
}

// 모음 매핑
WCHAR VowelToHiragana(WCHAR consonant, WCHAR vowel)
{
    // 자음+모음 조합
    if (consonant == L'k')
    {
        switch (vowel)
        {
        case L'a': return 0x304B; // か
        case L'i': return 0x304D; // き
        case L'u': return 0x304F; // く
        case L'e': return 0x3051; // け
        case L'o': return 0x3053; // こ
        }
    }
    else if (consonant == L's')
    {
        switch (vowel)
        {
        case L'a': return 0x3055; // さ
        case L'i': return 0x3057; // し
        case L'u': return 0x3059; // す
        case L'e': return 0x305B; // せ
        case L'o': return 0x305D; // そ
        }
    }
    else if (consonant == L't')
    {
        switch (vowel)
        {
        case L'a': return 0x305F; // た
        case L'i': return 0x3061; // ち
        case L'u': return 0x3064; // つ
        case L'e': return 0x3066; // て
        case L'o': return 0x3068; // と
        }
    }
    // 나머지 자음+모음은 필요하면 추가
    return 0;
}

// 단독 모음 매핑
WCHAR SingleVowelToHiragana(WCHAR vowel)
{
    switch (vowel)
    {
    case L'a': return 0x3042; // あ
    case L'i': return 0x3044; // い
    case L'u': return 0x3046; // う
    case L'e': return 0x3048; // え
    case L'o': return 0x304A; // お
    default: return vowel;
    }
}

/* 화면이 수정된 상태를...표출하려면 */
void  CSampleIME :: _UpdateCompositionText( TfEditCookie ec, ITfContext* pContext, const WCHAR* text, ULONG textLen)
{
  ITfRange* pRange = nullptr;
       if (SUCCEEDED(_pComposition->GetRange(&pRange)) && pRange)
            {
                //커서를 조합 범위의 끝으로 이동 (깜빡임 유지)
                TF_SELECTION tfSel;
                tfSel.range = pRange;
                tfSel.style.ase = TF_AE_END;  // 끝으로 앵커
                tfSel.style.fInterimChar = true;
                pContext->SetSelection(ec, 1, &tfSel);


                pRange->Release();

            }
 #if 0 

/**/
    // 기존 조합 범위 가져오기
    ITfRange* pRange = nullptr;
    if (SUCCEEDED(_pComposition->GetRange(&pRange)) && pRange)
    {
        // 텍스트 길이 확인
        LONG totalLen = 0;
        pRange->GetText(ec, 0, nullptr, 0, (ULONG*)&totalLen);
        
        if (totalLen > 0)
        {
            // 최근 입력된 문자 범위 생성 (마지막 문자)
            ITfRange* pLastCharRange = nullptr;
            if (SUCCEEDED(pRange->Clone(&pLastCharRange)))
            {
                // 범위를 마지막 문자로 축소
                pLastCharRange->Collapse(ec, TF_ANCHOR_END);
                LONG shifted;
                pLastCharRange->ShiftStart(ec, -1, &shifted, NULL);  // 시작을 왼쪽으로 1 이동
                
                // DisplayAttribute 적용 (배경 색상으로 사각형 효과)
                // DisplayAttributeInfo를 사용하여 속성 설정
                // (프로젝트의 DisplayAttribute 관련 클래스를 사용)
                // 예: 배경을 회색으로 설정하여 사각형처럼 보이게 함
                // 실제 구현은 DisplayAttributeProvider를 통해 해야 함
                
                // 예시: 간단히 속성 적용 (실제로는 더 복잡)
                // _pDisplayAttributeProvider->SetDisplayAttribute(pLastCharRange, ...);
                
                pLastCharRange->Release();
            }
        }
        pRange->Release();
    }

    #endif
}
//+---------------------------------------------------------------------------
//
// _HandleCompositionInput
//
// If the keystroke happens within a composition, eat the key and return S_OK.
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleCompositionInput(TfEditCookie ec, _In_ ITfContext *pContext, WCHAR wch)
{
    ITfRange* pRangeComposition = nullptr;
    TF_SELECTION tfSelection;
    ULONG fetched = 0;
    BOOL isCovered = TRUE;


DebugLogFile( L"_HandleInput  PP  key %x\n", wch );
    CCompositionProcessorEngine* pCompositionProcessorEngine = nullptr;
    pCompositionProcessorEngine = _pCompositionProcessorEngine;

DebugLogFile( L"      pass 1\n"); 
    if ((_pCandidateListUIPresenter != nullptr) && (_candidateMode != CANDIDATE_INCREMENTAL))
    {
        DebugLogFile( L"      pass 1 --->  _HandleCompositionFinalize \n"); 
        _HandleCompositionFinalize(ec, pContext, FALSE);
    }

DebugLogFile( L"      pass 2\n"); 

    // Start the new (std::nothrow) compositon if there is no composition.
    if (!_IsComposing())
    {
        _StartComposition(pContext);
    }


DebugLogFile( L"      pass 3\n"); 
    // first, test where a keystroke would go in the document if we did an insert
    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched) != S_OK || fetched != 1)
    {
        return S_FALSE;
    }


#if 1  
  // ===============================
  // 🔽 여기부터 네 입력 처리 로직
  // ===============================

    //static ITfRange* pCompositionRange = nullptr;  // 조합 범위
    WCHAR outputChar = 0;
    bool restartComposition = false; 

    DebugLogFile( L"_HandleInput  key %x, last %x\n", wch, lastChar);

    // 첫타 입력
    if (is_first(wch))
    {
        // if ( lastChar )
        // {   // 자음 키가  두번 눌렸을때 skip
        //     outputChar = 0;
        // }
        // else
        if ( lastChar )
        {
            // 마지막 입력키가  첫타일경우 
            lastChar = wch;

            // 키를 바꾼다. 
            outputChar = get_convert(  table_alpha[lastChar-'a'], wch  );
            restartComposition = true;   // 🔴 기존 조합(이렇게하면 치환은 되나..ms워드에 고스트문자 발생)
            DebugLogFile( L"             reset first key \n");
        }
        else
        {
            // 자음 입력 → 화면에는 바로 표시하지 않고 Composition에 임시 저장
            lastChar = wch;
            //outputChar = FirstToHiragana(wch); // k → か
        
            outputChar  = table_alpha[wch - 'a'];
            restartComposition = false;
            DebugLogFile( L"             set first key \n");
        }
    }
    else    // 후타가 입력됨 
    {
        DebugLogFile( L"       SKIP key  key %x \n", lastChar );
        if (lastChar) // 첫타가 ?
        {
            // k → か 상태에서 e 입력
            //outputChar = VowelToHiragana(lastChar, wch); // → え

            outputChar = get_convert(  table_alpha[lastChar-'a'], wch  );
            //outputChar = 0x304F;
            lastChar = 0;
            restartComposition = true;  // 🔴 기존 조합 치환(이렇게하면 치환은 되나 ms워드 고스트문자 )
        }
        else        // 후타가 없는 상태
        {
            //if (Global::IsShiftKeyDownOnly)
            if ( is_shfited(wch) )
            {
                outputChar = get_shift_convert(  wch  );
                lastChar = 0;

            } else {
                // no effect..
                //outputChar = SingleVowelToHiragana(wch);
                /* first 가 없다면 아무일도 하지 않는다. */
                outputChar = 0;
            }
        }


    /*
        if (lastChar)
        {
            // 자음 + 모음 조합
            outputChar = VowelToHiragana(lastChar, wch);  // → え
            lastChar = 0;
        }
        else
        {
            outputChar = SingleVowelToHiragana(wch);
        }
            // 기존 Composition 텍스트 제거
            if (pCompositionRange)
            {
                pCompositionRange->SetText(ec, 0, L"", 0);
                pCompositionRange->Release();
                pCompositionRange = nullptr;
            }
        }
        else {
            outputChar = SingleVowelToHiragana(wch);
        }
    */
    }

    if (outputChar == 0)        // 출력문자가 없다면  
        goto Exit;
    
    // --------------------
    // 2. 기존 조합 제거 (치환일 때만)
    // --------------------
    DebugLogFile( L"             set first key %d, %x\n", restartComposition, _pComposition);
    if (restartComposition && _pComposition)
    {
/* 이것은  필요없는 항목  워드에서 고스트 문자만 생성 
            DebugLogFile( L"             restartCompition && _pOmposition   \n");
            ITfRange* pRange = nullptr;
            if (SUCCEEDED(_pComposition->GetRange(&pRange)) && pRange)
            {
                // 문서에서 기존 조합 삭제
                //1. 
                //pRange->SetText(ec, TF_ST_CORRECTION, &outputChar, 1);    // TF_ST_CORRECTIO을 쓰면 조합을 끝내버리는 앱이 많음. 
                //2.
                pRange->SetText(ec, 0, &outputChar, 1);
                //3.
                //pRange->SetText(ec, TF_TF_MOVESTART, &outputChar, 1);
                pRange->Release();
            }
*/

            // 🔴 핵심: Worker ❌ / 직접 조합 갱신 ⭕
            //_UpdateCompositionText(ec, pContext, &outputChar, 1 );
        
           
            /* 무엇을 하는 것이냐. 이것의 문제는 아닌듯하다. */
            CCompositionProcessorEngine* pEngine = _pCompositionProcessorEngine;
            DWORD_PTR len = pEngine->GetVirtualKeyLength();  // len = 2
            pEngine->RemoveVirtualKey(len - 1);  // 마지막 키 제거 → 버퍼: ['k']


            //조합은 유지
            //return S_OK;        // 클러시 
            // 여기서  리턴안하면... 글자가 두개

            // 추가. 
            // _pComposition->EndComposition(ec);
            // _pComposition->Release();
            // _pComposition = nullptr;
    }
#endif 


    // 현재 커서(입력 위치)가 조합(composition) 상태에 포함되어 있나요?
    if (SUCCEEDED(_pComposition->GetRange(&pRangeComposition)))
    {
        DebugLogFile( L"             _pComposition->GetRange(  \n");
        isCovered = _IsRangeCovered(ec, tfSelection.range, pRangeComposition);
        pRangeComposition->Release();
        if (!isCovered) { goto Exit; }
    }
    DebugLogFile( L"             _pCompositionProcessorEngine addr  %x  \n", _pCompositionProcessorEngine);
    // 가짜 키 입력
#if 1 // all2key
    _pCompositionProcessorEngine->AddVirtualKey(outputChar);  // 이문자를 사용자가 타이핑한 것처럼 처리해라. 
                                                              // 조합중이면 -> 조합문자열 처리 
                                                              // 조합중이 아니면 -> 새조합 시작 
#else  // original 
    pCompositionProcessorEngine->AddVirtualKey(wch);
#endif
    DebugLogFile( L"             _HandleCompositionInputWorker addr  %x  \n", _pCompositionProcessorEngine);
    // TSF가 조합/화면/UI 전부 처리
    // 이게 없으면 글자가 더 진행하지 않고 같은자리에서만 치환됨 
    _HandleCompositionInputWorker(_pCompositionProcessorEngine, ec, pContext);  // 조합을 계속할지 / 확정할지 / 취소할지 결정 

Exit:

            DebugLogFile( L"             last    \n");
    tfSelection.range->Release();
            DebugLogFile( L"             return    \n");
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionInputWorker
//
// If the keystroke happens within a composition, eat the key and return S_OK.
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleCompositionInputWorker(_In_ CCompositionProcessorEngine *pCompositionProcessorEngine, TfEditCookie ec, _In_ ITfContext *pContext)
{
    HRESULT hr = S_OK;
    CSampleImeArray<CStringRange> readingStrings;
    BOOL isWildcardIncluded = TRUE;


    DebugLogFile( L">> _HandleCompositionInputWorker  PP   %x \n", pCompositionProcessorEngine);

    //
    // Get reading string from composition processor engine
    //
    pCompositionProcessorEngine->GetReadingStrings(&readingStrings, &isWildcardIncluded);

    DebugLogFile( L">>                           step 1\n"); 

    for (UINT index = 0; index < readingStrings.Count(); index++)
    {
        hr = _AddComposingAndChar(ec, pContext, readingStrings.GetAt(index));
        if (FAILED(hr))
        {
            return hr;
        }
    }



    DebugLogFile( L">>                           step 2\n"); 

#if 1
    //
    // Get candidate string from composition processor engine
    //
    CSampleImeArray<CCandidateListItem> candidateList;
    pCompositionProcessorEngine->GetCandidateList(&candidateList, TRUE, FALSE);
    
    DebugLogFile( L">>                           step 2        pp 1\n"); 
    if ((candidateList.Count()))
    {
        DebugLogFile( L">>                           step 2        pp 2\n"); 
        DebugLogFile( L"  -> CandidateList found count %d\n", candidateList.Count() );

        hr = _CreateAndStartCandidate(pCompositionProcessorEngine, ec, pContext);
        if (SUCCEEDED(hr))
        {
            _pCandidateListUIPresenter->_ClearList();
            _pCandidateListUIPresenter->_SetText(&candidateList, TRUE);
        }
    }
    else if (_pCandidateListUIPresenter)
    {

            DebugLogFile( L">>                           step 2        pp 3\n"); 
        _pCandidateListUIPresenter->_ClearList();

        DebugLogFile( L">>                           step 2        pp 3-1\n"); 
    }
    else if (readingStrings.Count() && isWildcardIncluded)
    {
        DebugLogFile( L">>                           step 2        pp 4\n"); 
        hr = _CreateAndStartCandidate(pCompositionProcessorEngine, ec, pContext);
        if (SUCCEEDED(hr))
        {
            _pCandidateListUIPresenter->_ClearList();
        }
    }
#endif


    DebugLogFile( L">>                           step out \n"); 
    return hr;
}
//+---------------------------------------------------------------------------
//
// _CreateAndStartCandidate
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_CreateAndStartCandidate(_In_ CCompositionProcessorEngine *pCompositionProcessorEngine, TfEditCookie ec, _In_ ITfContext *pContext)
{
    HRESULT hr = S_OK;

    if (((_candidateMode == CANDIDATE_PHRASE) && (_pCandidateListUIPresenter))
        || ((_candidateMode == CANDIDATE_NONE) && (_pCandidateListUIPresenter)))
    {
        // Recreate candidate list
        _pCandidateListUIPresenter->_EndCandidateList();
        delete _pCandidateListUIPresenter;
        _pCandidateListUIPresenter = nullptr;

        _candidateMode = CANDIDATE_NONE;
        _isCandidateWithWildcard = FALSE;
    }

    if (_pCandidateListUIPresenter == nullptr)
    {
        _pCandidateListUIPresenter = new (std::nothrow) CCandidateListUIPresenter(this, Global::AtomCandidateWindow,
            CATEGORY_CANDIDATE,
            pCompositionProcessorEngine->GetCandidateListIndexRange(),
            FALSE);
        if (!_pCandidateListUIPresenter)
        {
            return E_OUTOFMEMORY;
        }

        _candidateMode = CANDIDATE_INCREMENTAL;
        _isCandidateWithWildcard = FALSE;

        // we don't cache the document manager object. So get it from pContext.
        ITfDocumentMgr* pDocumentMgr = nullptr;
        if (SUCCEEDED(pContext->GetDocumentMgr(&pDocumentMgr)))
        {
            // get the composition range.
            ITfRange* pRange = nullptr;
            if (SUCCEEDED(_pComposition->GetRange(&pRange)))
            {
                hr = _pCandidateListUIPresenter->_StartCandidateList(_tfClientId, pDocumentMgr, pContext, ec, pRange, pCompositionProcessorEngine->GetCandidateWindowWidth());
                pRange->Release();
            }
            pDocumentMgr->Release();
        }
    }

    return hr;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionFinalize
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleCompositionFinalize(TfEditCookie ec, _In_ ITfContext *pContext, BOOL isCandidateList)
{
    HRESULT hr = S_OK;

    if (isCandidateList && _pCandidateListUIPresenter)
    {
        // Finalize selected candidate string from CCandidateListUIPresenter
        DWORD_PTR candidateLen = 0;
        const WCHAR *pCandidateString = nullptr;

        candidateLen = _pCandidateListUIPresenter->_GetSelectedCandidateString(&pCandidateString);

        CStringRange candidateString;
        candidateString.Set(pCandidateString, candidateLen);

        if (candidateLen)
        {
            // Finalize character
            hr = _AddCharAndFinalize(ec, pContext, &candidateString);
            if (FAILED(hr))
            {
                return hr;
            }
        }
    }
    else
    {
        // Finalize current text store strings
        if (_IsComposing())
        {
            ULONG fetched = 0;
            TF_SELECTION tfSelection;

            if (FAILED(pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched)) || fetched != 1)
            {
                return S_FALSE;
            }

            ITfRange* pRangeComposition = nullptr;
            if (SUCCEEDED(_pComposition->GetRange(&pRangeComposition)))
            {
                if (_IsRangeCovered(ec, tfSelection.range, pRangeComposition))
                {
                    _EndComposition(pContext);
                }

                pRangeComposition->Release();
            }

            tfSelection.range->Release();
        }
    }

    _HandleCancel(ec, pContext);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionConvert
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleCompositionConvert(TfEditCookie ec, _In_ ITfContext *pContext, BOOL isWildcardSearch)
{
    HRESULT hr = S_OK;

    CSampleImeArray<CCandidateListItem> candidateList;

    //
    // Get candidate string from composition processor engine
    //
    CCompositionProcessorEngine* pCompositionProcessorEngine = nullptr;
    pCompositionProcessorEngine = _pCompositionProcessorEngine;
    pCompositionProcessorEngine->GetCandidateList(&candidateList, FALSE, isWildcardSearch);

    // If there is no candlidate listin the current reading string, we don't do anything. Just wait for
    // next char to be ready for the conversion with it.
    int nCount = candidateList.Count();
    if (nCount)
    {
        if (_pCandidateListUIPresenter)
        {
            _pCandidateListUIPresenter->_EndCandidateList();
            delete _pCandidateListUIPresenter;
            _pCandidateListUIPresenter = nullptr;

            _candidateMode = CANDIDATE_NONE;
            _isCandidateWithWildcard = FALSE;
        }

        // 
        // create an instance of the candidate list class.
        // 
        if (_pCandidateListUIPresenter == nullptr)
        {
            _pCandidateListUIPresenter = new (std::nothrow) CCandidateListUIPresenter(this, Global::AtomCandidateWindow,
                CATEGORY_CANDIDATE,
                pCompositionProcessorEngine->GetCandidateListIndexRange(),
                FALSE);
            if (!_pCandidateListUIPresenter)
            {
                return E_OUTOFMEMORY;
            }

            _candidateMode = CANDIDATE_ORIGINAL;
        }

        _isCandidateWithWildcard = isWildcardSearch;

        // we don't cache the document manager object. So get it from pContext.
        ITfDocumentMgr* pDocumentMgr = nullptr;
        if (SUCCEEDED(pContext->GetDocumentMgr(&pDocumentMgr)))
        {
            // get the composition range.
            ITfRange* pRange = nullptr;
            if (SUCCEEDED(_pComposition->GetRange(&pRange)))
            {
                hr = _pCandidateListUIPresenter->_StartCandidateList(_tfClientId, pDocumentMgr, pContext, ec, pRange, pCompositionProcessorEngine->GetCandidateWindowWidth());
                pRange->Release();
            }
            pDocumentMgr->Release();
        }
        if (SUCCEEDED(hr))
        {
            _pCandidateListUIPresenter->_SetText(&candidateList, FALSE);
        }
    }

    return hr;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionBackspace
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleCompositionBackspace(TfEditCookie ec, _In_ ITfContext *pContext)
{
    ITfRange* pRangeComposition = nullptr;
    TF_SELECTION tfSelection;
    ULONG fetched = 0;
    BOOL isCovered = TRUE;

    // Start the new (std::nothrow) compositon if there is no composition.
    if (!_IsComposing())
    {
        return S_OK;
    }

    // first, test where a keystroke would go in the document if we did an insert
    if (FAILED(pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched)) || fetched != 1)
    {
        return S_FALSE;
    }

    // is the insertion point covered by a composition?
    if (SUCCEEDED(_pComposition->GetRange(&pRangeComposition)))
    {
        isCovered = _IsRangeCovered(ec, tfSelection.range, pRangeComposition);

        pRangeComposition->Release();

        if (!isCovered)
        {
            goto Exit;
        }
    }

    //
    // Add virtual key to composition processor engine
    //
    CCompositionProcessorEngine* pCompositionProcessorEngine = nullptr;
    pCompositionProcessorEngine = _pCompositionProcessorEngine;

    DWORD_PTR vKeyLen = pCompositionProcessorEngine->GetVirtualKeyLength();

    if (vKeyLen)
    {
        pCompositionProcessorEngine->RemoveVirtualKey(vKeyLen - 1);

        if (pCompositionProcessorEngine->GetVirtualKeyLength())
        {
            _HandleCompositionInputWorker(pCompositionProcessorEngine, ec, pContext);
        }
        else
        {
            _HandleCancel(ec, pContext);
        }
    }

Exit:
    tfSelection.range->Release();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionArrowKey
//
// Update the selection within a composition.
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleCompositionArrowKey(TfEditCookie ec, _In_ ITfContext *pContext, KEYSTROKE_FUNCTION keyFunction)
{
    ITfRange* pRangeComposition = nullptr;
    TF_SELECTION tfSelection;
    ULONG fetched = 0;

    // get the selection
    if (FAILED(pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched))
        || fetched != 1)
    {
        // no selection, eat the keystroke
        return S_OK;
    }

    // get the composition range
    if (FAILED(_pComposition->GetRange(&pRangeComposition)))
    {
        goto Exit;
    }

    // For incremental candidate list
    if (_pCandidateListUIPresenter)
    {
        _pCandidateListUIPresenter->AdviseUIChangedByArrowKey(keyFunction);
    }

    pContext->SetSelection(ec, 1, &tfSelection);

    pRangeComposition->Release();

Exit:
    tfSelection.range->Release();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionPunctuation
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleCompositionPunctuation(TfEditCookie ec, _In_ ITfContext *pContext, WCHAR wch)
{
    HRESULT hr = S_OK;

    if (_candidateMode != CANDIDATE_NONE && _pCandidateListUIPresenter)
    {
        DWORD_PTR candidateLen = 0;
        const WCHAR* pCandidateString = nullptr;

        candidateLen = _pCandidateListUIPresenter->_GetSelectedCandidateString(&pCandidateString);

        CStringRange candidateString;
        candidateString.Set(pCandidateString, candidateLen);

        if (candidateLen)
        {
            _AddComposingAndChar(ec, pContext, &candidateString);
        }
    }
    //
    // Get punctuation char from composition processor engine
    //
    CCompositionProcessorEngine* pCompositionProcessorEngine = nullptr;
    pCompositionProcessorEngine = _pCompositionProcessorEngine;

    WCHAR punctuation = pCompositionProcessorEngine->GetPunctuation(wch);

    CStringRange punctuationString;
    punctuationString.Set(&punctuation, 1);

    // Finalize character
    hr = _AddCharAndFinalize(ec, pContext, &punctuationString);
    if (FAILED(hr))
    {
        return hr;
    }

    _HandleCancel(ec, pContext);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionDoubleSingleByte
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleCompositionDoubleSingleByte(TfEditCookie ec, _In_ ITfContext *pContext, WCHAR wch)
{
    HRESULT hr = S_OK;

    WCHAR fullWidth = Global::FullWidthCharTable[wch - 0x20];

    CStringRange fullWidthString;
    fullWidthString.Set(&fullWidth, 1);

    // Finalize character
    hr = _AddCharAndFinalize(ec, pContext, &fullWidthString);
    if (FAILED(hr))
    {
        return hr;
    }

    _HandleCancel(ec, pContext);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _InvokeKeyHandler
//
// This text service is interested in handling keystrokes to demonstrate the
// use the compositions. Some apps will cancel compositions if they receive
// keystrokes while a compositions is ongoing.
//
// param
//    [in] uCode - virtual key code of WM_KEYDOWN wParam
//    [in] dwFlags - WM_KEYDOWN lParam
//    [in] dwKeyFunction - Function regarding virtual key
//----------------------------------------------------------------------------

HRESULT CSampleIME::_InvokeKeyHandler(_In_ ITfContext *pContext, UINT code, WCHAR wch, DWORD flags, _KEYSTROKE_STATE keyState)
{
    flags;

    CKeyHandlerEditSession* pEditSession = nullptr;
    HRESULT hr = E_FAIL;

    // we'll insert a char ourselves in place of this keystroke
    pEditSession = new (std::nothrow) CKeyHandlerEditSession(this, pContext, code, wch, keyState);
    if (pEditSession == nullptr)
    {
        goto Exit;
    }

    //
    // Call CKeyHandlerEditSession::DoEditSession().
    //
    // Do not specify TF_ES_SYNC so edit session is not invoked on WinWord
    //
    hr = pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);

    pEditSession->Release();

Exit:
    return hr;
}
