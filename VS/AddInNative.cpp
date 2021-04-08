
#include "stdafx.h"

#include <stdio.h>
#include <wchar.h>
#include "AddInNative.h"
#include <string>

#define TIME_LEN 34

#define BASE_ERRNO     7

#ifdef __linux__

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h> 
#include <termios.h>
#include <wchar.h>
#include <stdarg.h>
#include <iconv.h>

typedef char TCHAR;

#ifdef __GNUC__
#define  swscanf_s swscanf
#endif

int ReadFile(int file, void *buffer, size_t count, DWORD* bytes_read,
        void * overlapped_ignored){
    int status = 1;
    *bytes_read = read(file, buffer, count);
    if(*bytes_read < 0){
        *bytes_read = 0;
        status = 0;
    }
    return status;
}

int WriteFile(int file, const void *buffer, size_t count, DWORD* bytes_written, 
        void * overlapped_ignored){
    int status = 1;
    *bytes_written = write(file, buffer, count);
    if(*bytes_written < 0){
        *bytes_written = 0;
        status = 0;
    }
    return status;
}

void CloseHandle(int file){
    close(file);
}

void Sleep(long milliseconds){
    usleep(milliseconds * 1000);
}

int wsprintf(wchar_t *buf, const wchar_t* format, ...){
    va_list ap;
    va_start(ap, format);
    int ret = vswprintf(buf, sizeof(buf) / sizeof(*buf), format, ap);
    va_end(ap);
    return ret;
}
#endif


static const wchar_t *curver = L"2.3.0";

static const wchar_t *g_PropNames[] = {L"Port", L"Baud", L"IsOpen", 
		L"LastCmd", L"LastAnswer", L"LastError", L"LastTextError", 
		L"Version", L"ErrorsNumber"};
static const wchar_t *g_MethodNames[] = {L"OpenPort", L"ClosePort", L"Test", 
		L"Loging", L"CMD", L"SetCnt", L"ErrorByIndex", L"ErrorTextByIndex"};

static const wchar_t *g_PropNamesRu[] = {L"����", L"��������", L"����������", 
		L"�������", L"�����", L"������",  L"�����������", 
		L"������", L"����������������"};
static const wchar_t *g_MethodNamesRu[] = {L"�����������", L"�����������", L"����",  
		L"�����������", L"�������", L"�����������������", L"��������������", L"�������������������"};

static const wchar_t *maria_Errors[] = {
		L"HARDPAPER",  L"HARDSENSOR", L"HARDPOINT",  L"HARDTXD",    L"HARDTIMER", 
		L"HARDMEMORY", L"HARDLCD",    L"HARDUCCLOW", L"HARDCUTTER", L"SHUTDOWN",
		L"SOFTBLOCK",  L"SOFTNREP",   L"SOFTSYSLOC", L"SOFTCOMMAN", L"SOFTPROTOC",
		L"SOFTZREPOR", L"SOFTFMFULL", L"SOFTPARAM",  L"SOFTUPAS",   L"SOFTCHECK",
		L"SOFTSLWORK", L"SOFTSLPROG", L"SOFTSLZREP", L"SOFTSLNREP", L"SOFTREPL", 
		L"SOFTREGIST", L"SOFTOVER",   L"SOFTNEED",   L"SOFT24HOUR", L"SOFTDIFART", 
		L"SOFTBADART", L"SOFTCOPY",   L"SOFTOVART",  L"SOFTBADDISC", L"SOFTBADCS", 
		L"SOFTARTMODE", 
		L"MEM_ERROR_CODE_01", L"MEM_ERROR_CODE_02", L"MEM_ERROR_CODE_03", 
		L"MEM_ERROR_CODE_04", L"MEM_ERROR_CODE_05", L"MEM_ERROR_CODE_06", 
		L"MEM_ERROR_CODE_07", L"MEM_ERROR_CODE_08", L"MEM_ERROR_CODE_09", 
		L"MEM_ERROR_CODE_10", L"MEM_ERROR_CODE_11", L"MEM_ERROR_CODE_12", 
		L"MEM_ERROR_CODE_13", L"MEM_ERROR_CODE_14", L"MEM_ERROR_CODE_15", 
		L"MEM_ERROR_CODE_16", L"MEM_ERROR_CODE_17", L"MEM_ERROR_CODE_18", 
		L"MEM_ERROR_CODE_19", L"MEM_ERROR_CODE_20", L"MEM_ERROR_CODE_21", 
		L"MEM_ERROR_CODE_22", L"MEM_ERROR_CODE_23", L"MEM_ERROR_CODE_24", 
		L"MEM_ERROR_CODE_25", L"MEM_ERROR_CODE_26", 
		L"RTC_ERROR_CODE_01", L"RTC_ERROR_CODE_02", L"RTC_ERROR_CODE_03", 
		L"RTC_ERROR_CODE_04", L"RTC_ERROR_CODE_05"};

static const wchar_t *maria_Text_Errors[] = {L"��� ������",
		L"���������� ������� ��� ����",
		L"���������� �������� ��������� ��� �����",
		L"���������� ���������� ��������� ��� �����",
		L"���������� ���������� �������� ��� �����",
		L"������ ������ � ��� ����",
		L"������ ������ ��� �����",
		L"��� ������ �� ����������",
		L"������ ������������� ������",
		L"������� ������� ��������",
		L"�������� �������� �������� ������",
		L"����������� ������� ���/� ����������� �����", //HARDPAPER 
		L"������������ ������������� ����� ���������� �������", 
		L"����������� ���������� ������� �������������� ��������� ���������� �������",  
		L"������ ������ �����: �������� �� ��������",    
		L"������ ��������� ������ ��������� ����� ��������� ������� ", 
		L"������ �������� ������ � ���������� ������ ", 
		L"������������� ������� ����������",    
		L"������ ���������� �������", 
		L"������������� ��������� ������� �����", 
		L"���� ���������� �� ����������� ��������", //SOFTBLOCK
		L"�������� ������� � ��������",  
		L"���������� ��������� Z �����",   
		L"��������� ����� � �����������", 
		L"��� ����� ��������", 
		L"������ ���������",
		L"�� ����������� Z �����", 
		L"���������� ������� ����������", 
		L"������ ���������� ��������",  
		L"��������� ��������� ���� � ����������� �������",   
		L"�� ��������� ����������� ����� ����������� ������� ",
		L"���������� ���� � �", 
		L"���������� ���� � �", 
		L"���������� ���� � �", 
		L"���������� ���� � Z", 
		L"��������������� �������� ��� ���� � ����", 
		L"���� �� ��������������", 
		L"�������� ������� � �����",   
		L"������ ���������� ���������� ����",   
		L"������ ������������ ����� 24-� �����", 
		L"�������������� ���������", 
		L"����� �������� ����� ��������", 
		L"������������ ������ �����������",   
		L"��������� ������������ ���������� ������ � ���� ",  
		L"���� ������ ������ ����� �������", 
		L"������ �������� ����������� �����", 
		L"������ ������ � ����������� �������", 
		L"����-��� �������� ������ � ����", //MEM_ERROR_CODE_01
		L"������ ������ � ����",
		L"�������� ����� �������� ����",
		L"�������� ����� ����",
		L"����������� ��� ������� ��������� �����, ���������� � ����",
		L"����������� ������ � ������ �����",
		L"����� ���������� Z-������, ����������� � ����, ������ ������ �������� Z-������",
		L"����� �������� Z-������ ����� ��� �� ������� ���������� �� ������ ���������� Z-������, ����������� � ����",
		L"����� �������� Z-������ �� ������ �� ������� ������ ���������� Z-������, ����������� � ����",
		L"�������� ���������� ���������� ������ � Z-������", //MEM_ERROR_CODE_10
		L"�������� ���������� ���������� ������ � ������",
		L"�������� ���������� ���������� ������ � �����������",
		L"�������� ���������� ���������� ������ � ������ �����",
		L"�������� ������������������ ������� Z-������� ��� ������������ ������ �� ������",
		L"����-��� �������� ������ � ����",
		L"������ ������ � ����",
		L"�������� ������������������ ������� ������� ������� ��� ������������ ������ �� ������",
		L"������",
		L"��������� ���������� ���������� ��������� ��������� ����������� ������",
		L"�������� ���������� ���������� ������ �� ��������� ��������� ���-�������� ������", //MEM_ERROR_CODE_20
		L"��������� ������ ���������� ������ � ������� ������� � �����������",
		L"��������� ������ ���������� ������ � ������� ������� � �������",
		L"��������� ������ ���������� ������ � ������� ������� � ������ �����",
		L"������",
		L"��������� ������ ���������� ������ � ������� ������� � ������� ���������� �������",
		L"��������� ������ ����",
		L"��������� ���� ��������� ������� �����������", //RTC_ERROR_CODE_01
		L"���� ���������� Z-������, ����������� � ����, ������ ������� ���� � ��������� ����� ��������� �������",
		L"�������� ����� � ��������� ����� ��������� �������",
		L"�������� ���� � ��������� ����� ��������� �������",
		L"������������� ���������� ����� ��������� ������� ��� ������ ����� ���������-����"};

static const wchar_t g_kClassNames[] = L"CAddInNative"; //"|OtherClass1|OtherClass2";
static IAddInDefBase *pAsyncEvent = NULL;

#ifndef __linux__
HANDLE hComm;
HANDLE hTempFile;
DCB		dcb;
#else
int hComm;
int hTempFile;
struct termios dcb;
#endif

uint8_t err_arr[80];

uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, size_t len = 0);
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len = 0);
uint32_t getLenShortWcharStr(const WCHAR_T* Source);
void string_normalize(std::wstring& inStr, int newLen, uint8_t ls, char ch);
bool param_check(std::wstring& inStr, tVariant* paParam, int newLen, int mult);
std::wstring strtowstr(const std::string &str);
std::string wstrtostr(const std::wstring &wstr);
void IpHexToStr(std::wstring& in, std::wstring& out);
void PortHexToStr(std::wstring& in, std::wstring& out);
void DelayHexToStr(std::wstring& in, std::wstring& out);
int subst( char * str, int ln, const char * substr, int subln);

//---------------------------------------------------------------------------//
long GetClassObject(const WCHAR_T* wsName, IComponentBase** pInterface)
{
    if(!*pInterface)
    {
        *pInterface= new CAddInNative;
        return (long)*pInterface;
    }
    return 0;
}
//---------------------------------------------------------------------------//
long DestroyObject(IComponentBase** pIntf)
{
   if(!*pIntf)
      return -1;

   delete *pIntf;
   *pIntf = 0;
   return 0;
}
//---------------------------------------------------------------------------//
const WCHAR_T* GetClassNames()
{
    static WCHAR_T* names = 0;
    if (!names)
        ::convToShortWchar(&names, g_kClassNames);
    return names;
}
//---------------------------------------------------------------------------//
#ifndef __linux__
VOID CALLBACK MyTimerProc(
        HWND hwnd, // handle of window for timer messages
        UINT uMsg, // WM_TIMER message
        UINT idEvent, // timer identifier
        DWORD dwTime // current system time
);
#else
static void MyTimerProc(int sig);
#endif //__linux__

// CAddInNative
//---------------------------------------------------------------------------//
CAddInNative::CAddInNative()
{
    m_iMemory = 0;
    m_iConnect = 0;
}
//---------------------------------------------------------------------------//
CAddInNative::~CAddInNative()
{
}
//---------------------------------------------------------------------------//
bool CAddInNative::Init(void* pConnection)
{ 
    m_iConnect = (IAddInDefBase*)pConnection;
	m_isOpen = false;
	m_loging = false;
	//m_loging = true;
	m_cnt = 300;
	m_err = 0;
	m_err_cnt = 0;
#ifndef __linux__
	m_port = 1;
#else
    m_port = 0;
#endif
	m_baud = 9600;
    m_log_file = fopen("/tmp/maria_printer.log", "a");
    fprintf(m_log_file, "MARIA: Initialaized!\n"); fflush(m_log_file);
    return m_iConnect != NULL;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetInfo()
{ 
    // Component should put supported component technology version 
    // This component supports 2.0 version
    return 2000; 
}
//---------------------------------------------------------------------------//
void CAddInNative::Done()
{
}
/////////////////////////////////////////////////////////////////////////////
// ILanguageExtenderBase
//---------------------------------------------------------------------------//
bool CAddInNative::RegisterExtensionAs(WCHAR_T** wsExtensionName)
{ 
    const wchar_t *wsExtension = L"Maria2";
    int iActualSize = ::wcslen(wsExtension) + 1;
    WCHAR_T* dest = 0;


    if (m_iMemory)
    {
        if(m_iMemory->AllocMemory((void**)wsExtensionName, iActualSize * sizeof(WCHAR_T)))
            ::convToShortWchar(wsExtensionName, wsExtension, iActualSize);
            fprintf(m_log_file, "Registered! Name: '%s'\n", wstrtostr(wsExtension).c_str()); fflush(m_log_file);
        return true;
    }
    
    fprintf(m_log_file, "NOT Registered!\n"); fflush(m_log_file);

    return false; 
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNProps()
{ 
    // You may delete next lines and add your own implementation code here
    return ePropLast;
}
//---------------------------------------------------------------------------//
long CAddInNative::FindProp(const WCHAR_T* wsPropName)
{ 
    long plPropNum = -1;
    wchar_t* propName = 0;

    ::convFromShortWchar(&propName, wsPropName);
    plPropNum = findName(g_PropNames, propName, ePropLast);

    if (plPropNum == -1)
        plPropNum = findName(g_PropNamesRu, propName, ePropLast);

    delete[] propName;

    return plPropNum;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInNative::GetPropName(long lPropNum, long lPropAlias)
{ 
    if (lPropNum >= ePropLast)
        return NULL;

    const wchar_t *wsCurrentName = NULL;
    WCHAR_T *wsPropName = NULL;
    int iActualSize = 0;

    //fprintf(m_log_file, "GetPropName(%ld, %ld)\n", lPropNum, lPropAlias); fflush(m_log_file);

    switch(lPropAlias)
    {
    case 0: // First language
        wsCurrentName = g_PropNames[lPropNum];
        break;
    case 1: // Second language
        wsCurrentName = g_PropNamesRu[lPropNum];
        break;
    default:
        //fprintf(m_log_file, "GetPropName: No name matched!\n"); fflush(m_log_file);
        return 0;
    }
    
    //fprintf(m_log_file, "GetPropName: wsCurrentName: '%s'\n", wstrtostr(wsCurrentName).c_str()); fflush(m_log_file);
    
    iActualSize = wcslen(wsCurrentName)+1;

    if (m_iMemory && wsCurrentName)
    {
        if (m_iMemory->AllocMemory((void**)&wsPropName, iActualSize * sizeof(WCHAR_T)))
            ::convToShortWchar(&wsPropName, wsCurrentName, iActualSize);
    }

    //fprintf(m_log_file, "GetPropName: wsPropName: '%s'\n", wstrtostr(wsCurrentName).c_str()); fflush(m_log_file);
    return wsPropName;
}
//---------------------------------------------------------------------------//
bool CAddInNative::GetPropVal(const long lPropNum, tVariant* pvarPropVal)
{ 
	std::wstring S;
	//const wchar_t* ss;
	switch(lPropNum)
    {
    case ePropPort:
		TV_VT(pvarPropVal) = VTYPE_UI2;
		TV_UI1(pvarPropVal) = m_port;
        break;
    case ePropBaud:
        TV_VT(pvarPropVal) = VTYPE_UI4;
		TV_UI4(pvarPropVal) = m_baud;
        break;
	case ePropIsOpen:
        TV_VT(pvarPropVal) = VTYPE_BOOL;
		TV_BOOL(pvarPropVal) = m_isOpen;
		break;
	case ePropLastCmd:
		wstring_to_p(m_cmd, pvarPropVal);
		break;
	case ePropLastAns:
		wstring_to_p(m_ans, pvarPropVal);
		break;
	case ePropLastError:
		TV_VT(pvarPropVal) = VTYPE_UI2;
		TV_UI1(pvarPropVal) = m_err;
        break;
	case ePropLastErrorText:
		S.assign(maria_Text_Errors[m_err]);
		wstring_to_p(S, pvarPropVal);
		break;
	case ePropVersion:
		wstring_to_p(curver, pvarPropVal);
		break;
    case ePropErrorNumber:
		TV_VT(pvarPropVal) = VTYPE_UI2;
		TV_UI1(pvarPropVal) = m_err_cnt;
		break;
	default:
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------//
bool CAddInNative::SetPropVal(const long lPropNum, tVariant *varPropVal)
{ 
	//last_type = TV_VT(varPropVal);
	//const wchar_t* s;
    switch(lPropNum)
    { 
    case ePropPort:
		//if (TV_VT(varPropVal) != VTYPE_UI1)
        //    return false;
        //m_port = TV_UI1(varPropVal);
        //break;
    case ePropBaud:
		//if (TV_VT(varPropVal) != VTYPE_UI4)
        //    return false;
        //m_baud = TV_UI4(varPropVal);
        //break;
    case ePropIsOpen:
        //if (TV_VT(varPropVal) != VTYPE_BOOL)
        //    return false;
		//m_isOpen = TV_BOOL(varPropVal);
		//break;
	case ePropLastCmd:
	case ePropVersion:
    case ePropErrorNumber:
    default:
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------//
bool CAddInNative::IsPropReadable(const long lPropNum)
{ 
    switch(lPropNum)
    { 
	case ePropPort:
    case ePropBaud:
	case ePropIsOpen:
	case ePropLastCmd:
	case ePropLastAns:
	case ePropLastError:
	case ePropLastErrorText:
	case ePropVersion:
    case ePropErrorNumber:
		return true;
    default:
        return false;
    }

    return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::IsPropWritable(const long lPropNum)
{
    switch(lPropNum)
    { 
    case ePropPort:
		//return true;
	case ePropBaud:
		//return true;
	case ePropIsOpen:
	case ePropLastCmd:
	case ePropLastAns:
    case ePropVersion:
    case ePropErrorNumber:
        return false;
    default:
        return false;
    }

    return false;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNMethods()
{ 
    return eMethLast;
}
//---------------------------------------------------------------------------//
long CAddInNative::FindMethod(const WCHAR_T* wsMethodName)
{ 
    long plMethodNum = -1;
    wchar_t* name = 0;

    ::convFromShortWchar(&name, wsMethodName);

    plMethodNum = findName(g_MethodNames, name, eMethLast);

    if (plMethodNum == -1)
        plMethodNum = findName(g_MethodNamesRu, name, eMethLast);

    return plMethodNum;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInNative::GetMethodName(const long lMethodNum, const long lMethodAlias)
{ 
    if (lMethodNum >= eMethLast)
        return NULL;

    const wchar_t *wsCurrentName = NULL;
    WCHAR_T *wsMethodName = NULL;
    int iActualSize = 0;

    switch(lMethodAlias)
    {
    case 0: // First language
        wsCurrentName = g_MethodNames[lMethodNum];
        break;
    case 1: // Second language
        wsCurrentName = g_MethodNamesRu[lMethodNum];
        break;
    default: 
        return 0;
    }

    iActualSize = wcslen(wsCurrentName)+1;

    if (m_iMemory && wsCurrentName)
    {
        if(m_iMemory->AllocMemory((void**)&wsMethodName, iActualSize * sizeof(WCHAR_T)))
            ::convToShortWchar(&wsMethodName, wsCurrentName, iActualSize);
    }

    return wsMethodName;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNParams(const long lMethodNum)
{ 
    switch(lMethodNum)
    { 
	case eMethOpenPort:
        return 2;
	case eMethCMD:
	case eMethSetCnt:
	case eMethGetError:
	case eMethGetErrorText:
        return 1;
	default:
        return 0;
    }
    
    return 0;
}
//---------------------------------------------------------------------------//
bool CAddInNative::GetParamDefValue(const long lMethodNum, const long lParamNum,
                          tVariant *pvarParamDefValue)
{ 
	
	TV_VT(pvarParamDefValue)= VTYPE_EMPTY;

    switch(lMethodNum)
    { 
    case eMethOpenPort:
		switch(lParamNum)
		{
		case 0:
			//m_port = 1;
			TV_VT(pvarParamDefValue) = VTYPE_UI1;
			TV_UI1(pvarParamDefValue) = 1;
			return true;
		case 1:
			TV_VT(pvarParamDefValue) = VTYPE_UI4;
			TV_UI4(pvarParamDefValue) = 115200;
			return true;
		}
	case eMethCMD:
		switch(lParamNum)
		{
		case 1:
     		wstring_to_p((L"SYNC"), pvarParamDefValue);
			return true;
		default:
			return false;
		}
	case eMethTest:
	case eMethSetCnt:
	case eMethGetError:
	case eMethGetErrorText:
       // There are no parameter values by default 
        break;
    default:
        return false;
    }

    return false;
} 
//---------------------------------------------------------------------------//
bool CAddInNative::HasRetVal(const long lMethodNum)
{ 
    switch(lMethodNum)
    { 
	case eMethOpenPort:
 	case eMethTest:
	case eMethCMD:
	case eMethGetError:
	case eMethGetErrorText:
		return true;
    default:
		return false;
    }

    return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::CallAsProc(const long lMethodNum,
                    tVariant* paParams, const long lSizeArray)
{ 
	
    switch(lMethodNum)
    { 
    case eMethClosePort:
		CAddInNative::ClosePort();
        break;
	case eMethLoging:
		if (!m_isOpen) m_loging = !m_loging;
        break;
	case eMethSetCnt:
		m_cnt = TV_UI4(paParams);
        break;
    default:
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------//
bool CAddInNative::CallAsFunc(const long lMethodNum,
                tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{ 
    bool ret = false;
	uint8_t res;
	uint8_t errnum;
	std::wstring S;

    switch(lMethodNum)
    {
	case eMethOpenPort:
        //if (!lSizeArray || !paParams)
        //    return false;

		res = 0;
		if (!m_isOpen)
		{ 

			m_port = TV_UI1(paParams);
			m_baud = TV_UI4(paParams+1);

			res = CAddInNative::OpenPort();
		} 
		else res = 8; //allready opened

        TV_VT(pvarRetValue) = VTYPE_UI1;
		TV_UI1(pvarRetValue) = res;

		ret = true;
		break;

	case eMethTest:
		ret = CAddInNative::Test(pvarRetValue, paParams, lSizeArray);
		break;	
	
	case eMethCMD:
		ret = CAddInNative::CMD(pvarRetValue, paParams, lSizeArray);
		break;

	case eMethGetError:

		errnum = TV_UI1(paParams);

		if (m_err_cnt == 0){
			res = 0;
		} else if (errnum > m_err_cnt) {
			res = 0;
		} else if (errnum < 1) {
			res = 0;
		} else {
			m_err = err_arr[errnum-1];
			res = m_err;
		}

        TV_VT(pvarRetValue) = VTYPE_UI1;
		TV_UI1(pvarRetValue) = res;

		ret = true;
		break;

	case eMethGetErrorText:

		errnum = TV_UI1(paParams);

		if (m_err_cnt == 0){
			res = 0;
		} else if (errnum > m_err_cnt) {
			res = 0;
		} else if (errnum < 1) {
			res = 0;
		} else {
			res = err_arr[errnum-1];
			S.assign(maria_Text_Errors[res]);
			wstring_to_p(S, pvarRetValue);
		}


		ret = true;
		break;

	}

	//if (!ret) ClosePort();
    return ret; 
}
//---------------------------------------------------------------------------//
// This code will work only on the client!
#ifndef __linux__
VOID CALLBACK MyTimerProc(
  HWND hwnd,    // handle of window for timer messages
  UINT uMsg,    // WM_TIMER message
  UINT idEvent, // timer identifier
  DWORD dwTime  // current system time
)
{
    if (!pAsyncEvent)
        return;

    wchar_t *who = L"Maria", *what = L"Timer";

    wchar_t *wstime = new wchar_t[TIME_LEN];
    if (wstime)
    {
        wmemset(wstime, 0, TIME_LEN);
        ::_ultow(dwTime, wstime, 10);
        pAsyncEvent->ExternalEvent(who, what, wstime);
        delete[] wstime;
    }
}
#else
void MyTimerProc(int sig)
{
    if (pAsyncEvent)
        return;

    WCHAR_T *who = 0, *what = 0, *wdata = 0;
    wchar_t *data = 0;
    time_t dwTime = time(NULL);

    data = new wchar_t[TIME_LEN];
    
    if (data)
    {
        wmemset(data, 0, TIME_LEN);
        swprintf(data, TIME_LEN, L"%ul", dwTime);
        ::convToShortWchar(&who, L"Maria");
        ::convToShortWchar(&what, L"Timer");
        ::convToShortWchar(&wdata, data);

        pAsyncEvent->ExternalEvent(who, what, wdata);

        delete[] who;
        delete[] what;
        delete[] wdata;
        delete[] data;
    }
}
#endif
//---------------------------------------------------------------------------//
void CAddInNative::SetLocale(const WCHAR_T* loc)
{
#ifndef __linux__
    _wsetlocale(LC_ALL, loc);
#else
    fprintf(m_log_file, "SetLocale called\n"); fflush(m_log_file);
    //We convert in char* char_locale
    //also we estabilish locale
    //setlocale(LC_ALL, wstrtostr(loc).c_str());
#endif
}
/////////////////////////////////////////////////////////////////////////////
// LocaleBase
//---------------------------------------------------------------------------//
bool CAddInNative::setMemManager(void* mem)
{
    m_iMemory = (IMemoryManager*)mem;
    return m_iMemory != 0;
}
//---------------------------------------------------------------------------//
void CAddInNative::addError(uint32_t wcode, const wchar_t* source, 
                        const wchar_t* descriptor, long code)
{
    if (m_iConnect)
    {
        WCHAR_T *err = 0;
        WCHAR_T *descr = 0;
        
        ::convToShortWchar(&err, source);
        ::convToShortWchar(&descr, descriptor);

        m_iConnect->AddError(wcode, err, descr, code);
        delete[] err;
        delete[] descr;
    }
}
//---------------------------------------------------------------------------//
long CAddInNative::findName(const wchar_t* names[], const wchar_t* name, 
                         const uint32_t size) const
{
    long ret = -1;
    for (uint32_t i = 0; i < size; i++)
    {
        if (!wcscmp(names[i], name))
        {
            ret = i;
            break;
        }
    }
    return ret;
}

//---------------------------------------------------------------------------//

bool CAddInNative::wstring_to_p(std::wstring str, tVariant* val) {
	char* t1;
	TV_VT(val) = VTYPE_PWSTR;
	m_iMemory->AllocMemory((void**)&t1, (str.length()+1) * sizeof(WCHAR_T));
	memcpy(t1, str.c_str(), (str.length()+1) * sizeof(WCHAR_T));
	val -> pstrVal = t1;
	val -> strLen = str.length();
	return true;
}
//---------------------------------------------------------------------------//
uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, size_t len)
{
    if (!len)
        len = ::wcslen(Source) + 1;

    if (!*Dest)
        *Dest = new WCHAR_T[len];

    WCHAR_T* tmpShort = *Dest;
    wchar_t* tmpWChar = (wchar_t*) Source;
    uint32_t res = 0;

    ::memset(*Dest, 0, len * sizeof(WCHAR_T));
#ifdef __linux__
    size_t succeed = (size_t)-1;
    size_t f = len * sizeof(wchar_t), t = len * sizeof(WCHAR_T);
    const char* fromCode = sizeof(wchar_t) == 2 ? "UTF-16" : "UTF-32";
    iconv_t cd = iconv_open("UTF-16LE", fromCode);
    if (cd != (iconv_t)-1)
    {
        succeed = iconv(cd, (char**)&tmpWChar, &f, (char**)&tmpShort, &t);
        iconv_close(cd);
        if(succeed != (size_t)-1)
            return (uint32_t)succeed;
    }
#endif //__linux__
    for (; len; --len, ++res, ++tmpWChar, ++tmpShort)
    {
        *tmpShort = (WCHAR_T)*tmpWChar;
    }

    return res;
}
//---------------------------------------------------------------------------//
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len)
{
    if (!len)
        len = getLenShortWcharStr(Source) + 1;

    if (!*Dest)
        *Dest = new wchar_t[len];

    wchar_t* tmpWChar = *Dest;
    WCHAR_T* tmpShort = (WCHAR_T*)Source;
    uint32_t res = 0;

    ::memset(*Dest, 0, len * sizeof(wchar_t));
#ifdef __linux__
    size_t succeed = (size_t)-1;
    const char* fromCode = sizeof(wchar_t) == 2 ? "UTF-16" : "UTF-32";
    size_t f = len * sizeof(WCHAR_T), t = len * sizeof(wchar_t);
    iconv_t cd = iconv_open("UTF-32LE", fromCode);
    if (cd != (iconv_t)-1)
    {
        succeed = iconv(cd, (char**)&tmpShort, &f, (char**)&tmpWChar, &t);
        iconv_close(cd);
        if(succeed != (size_t)-1)
            return (uint32_t)succeed;
    }
#endif //__linux__
    for (; len; --len, ++res, ++tmpWChar, ++tmpShort)
    {
        *tmpWChar = (wchar_t)*tmpShort;
    }

    return res;
}


/*
uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, uint32_t len)
{
    if (!len)
        len = ::wcslen(Source)+1;

    if (!*Dest)
        *Dest = new WCHAR_T[len];

    WCHAR_T* tmpShort = *Dest;
    wchar_t* tmpWChar = (wchar_t*) Source;
    uint32_t res = 0;

    ::memset(*Dest, 0, len*sizeof(WCHAR_T));
    do
    {
        *tmpShort++ = (WCHAR_T)*tmpWChar++;
        ++res;
    }
    while (len-- && *tmpWChar);

    return res;
}
//---------------------------------------------------------------------------//
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len)
{
    if (!len)
        len = getLenShortWcharStr(Source)+1;

    if (!*Dest)
        *Dest = new wchar_t[len];

    wchar_t* tmpWChar = *Dest;
    WCHAR_T* tmpShort = (WCHAR_T*)Source;
    uint32_t res = 0;

    ::memset(*Dest, 0, len*sizeof(wchar_t));
    do
    {
        *tmpWChar++ = (wchar_t)*tmpShort++;
        ++res;
    }
    while (len-- && *tmpShort);

    return res;
}
*/
//---------------------------------------------------------------------------//
uint32_t getLenShortWcharStr(const WCHAR_T* Source)
{
    uint32_t res = 0;
    WCHAR_T *tmpShort = (WCHAR_T*)Source;

    while (*tmpShort++)
        ++res;

    return res;
}
//---------------------------------------------------------------------------//

void CAddInNative::write_log(const char* OUTBUFFER, int l, char log_type)
{
	char	TMPBUFFER[50];
    DWORD   bytes_written = 0;
	int		bStatus;

	if (m_loging)
	{
		time_t t = time(0);   // get time now
		struct tm * now = localtime( & t );

		sprintf(TMPBUFFER,"[%04d%02d%02d%02d%02d%02d]", now->tm_year+1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
		TMPBUFFER[16] = log_type;
		TMPBUFFER[17] = ':';
		bStatus = WriteFile(hTempFile, &TMPBUFFER, 18, &bytes_written, NULL);

		bStatus = WriteFile(hTempFile, OUTBUFFER, l, &bytes_written, NULL);

		TMPBUFFER[0] = '\r';
		TMPBUFFER[1] = '\n';
		bStatus = WriteFile(hTempFile, &TMPBUFFER, 2, &bytes_written, NULL);

	}
}

uint8_t CAddInNative::return_error(uint8_t err_no)
{
	char TMPBUFFER[50];

	if (m_loging)
	{
		sprintf(TMPBUFFER,"%d", err_no);
		write_log(TMPBUFFER, 2, 'e');
	}
	m_err = err_no;
	return err_no;
}

std::string recode(const char* from, const char* to, std::string &src) {
    size_t f, t;
    f = src.length();
    t = src.length() * 2;
    std::string dst = "";
    char *c_dst = (char *) malloc(sizeof(char) * (t + 1));
    memset(c_dst, 0, t+1);
    char *c_dst_p = c_dst;
    char *c_src = (char *)src.c_str();

    FILE* log_file = fopen("/tmp/maria_printer.log", "a");
    fprintf(log_file, "recode: %d: %s\n", f, c_src);
    iconv_t cd = iconv_open(to, from);
    if (cd != (iconv_t)-1){
	errno = 0;
        int succeed = iconv(cd, &c_src, &f, &c_dst_p, &t);
        iconv_close(cd);
    	fprintf(log_file, "recode succeed: %d %d\n", succeed, errno);
//        if(succeed != (size_t)-1)
//            return (uint32_t)succeed;
    }
    fprintf(log_file, "recode res: %d: %d: %s\n", t, strlen(c_dst), c_dst);
    fclose(log_file);
    dst = c_dst;
    free(c_dst);
    return dst;
}

uint8_t CAddInNative::send_data(void)
{
    char	SMBUFFER[50];
    char	INBUFFER[500];
    char	OUTBUFFER[255];
	char	TMPBUFFER[50];

    DWORD   bytes_read		 = 0;
    DWORD   total_bytes_read = 0;
    DWORD   bytes_written	 = 0;
	
	int		bStatus;
	uint16_t i,l,j;
    //char *  pch = 0;
    int pch = 0;
    int pcw = 0;
    int tmppcw = -1;
	int cnt = m_cnt;

	std::string s;
	//std::size_t fnd;

	m_ans = (L"");
	m_err = 0;

	l = m_cmd.length();
	if (l>255) return return_error(9); //to long command

	for(i=0;i<l;i++) //ukrainian i problem
	{
		if (m_cmd[i]==L'�') m_cmd[i]=L'i';
		if (m_cmd[i]==L'�') m_cmd[i]=L'I';
	}

	std::string s_tmp = wstrtostr(m_cmd);
	s = recode("UTF-8", "cp866", s_tmp);
	l = s.length();

	if (l > 255) return return_error(9); //to long command
	if (!m_isOpen) return return_error(1);

	OUTBUFFER[0] = (char)253;
	memcpy(OUTBUFFER + 1, s.c_str(), l);
	OUTBUFFER[l+1] = l + 1;
	OUTBUFFER[l+2] = (char)254;

	if (m_loging) write_log(OUTBUFFER, l+3, 'c');

    fprintf(m_log_file, "Writing to port: %d chars: '%*s'\n", l+3, l+3, OUTBUFFER);
    fprintf(m_log_file, "Dump: %d chars:", l+3);
    for(i=0; i<l+3; i++)
        fprintf(m_log_file, " 0x%.2x", OUTBUFFER[i]);
    fprintf(m_log_file, "\n");
    fflush(m_log_file);

	bStatus = WriteFile(hComm, &OUTBUFFER, l+3, &bytes_written, NULL);
    if (!bStatus ) return return_error(5); //error while data send
	
	total_bytes_read = 0;
	for (i=0; i<cnt && !pch; i++){
		//Sleep(2);
		
		//GetCommModemStatus(hComm, &ModemStat);
		//if ((ModemStat && MS_CTS_ON)==0)
		//{
		//	return return_error(6);
		//}

		bytes_read = 0;
		bStatus = ReadFile(hComm, &SMBUFFER, 50, &bytes_read, NULL);
		fprintf(m_log_file, "port answer: %ld: %s\n", bytes_read, SMBUFFER); fflush(m_log_file);
		
		if (!bStatus) return return_error(6); //error while data recieve
		
		if (bytes_read>0)
		{
			memcpy(INBUFFER+total_bytes_read, SMBUFFER, bytes_read);
			total_bytes_read += bytes_read;
			INBUFFER[total_bytes_read] = 0;
			//pch = strstr(INBUFFER, "READY");
			pch = subst(INBUFFER, total_bytes_read, "READY", 5);

			pcw = subst(INBUFFER, total_bytes_read, "WRK", 3);
			if (pcw>0 && pcw!=tmppcw) 
			{
				cnt = cnt + m_cnt;
				tmppcw = pcw;
			}

			//s = INBUFFER;
			//s.resize(total_bytes_read);
			//fnd = s.find("READY");

		}
		//if (m_loging) 
		//{
		//	sprintf(TMPBUFFER,"%4d %4d %4d %d ZZ", i, bytes_read, total_bytes_read, pch);
		//	write_log(TMPBUFFER, 35, 'w');
		//	write_log(SMBUFFER, bytes_read, 'z');
		//}
	}

	//Sleep(10);

	s_tmp = INBUFFER;
    s = recode("cp866", "UTF-8", s_tmp);
	s.resize(total_bytes_read);
	m_ans = strtowstr(s);
	fprintf(m_log_file, "m_ans: before: %d: %s\n", s.length(), s.c_str()); fflush(m_log_file);
	fprintf(m_log_file, "m_ans: after: %d: %ls\n", m_ans.length(), m_ans.c_str()); fflush(m_log_file);

	if (m_loging) 
	{
		sprintf(TMPBUFFER,"%4d %4ld %4d", i, total_bytes_read, cnt);
		write_log(TMPBUFFER, 14, 't');
		if (total_bytes_read > 0) 
		{
			write_log(INBUFFER, total_bytes_read, 'a');
		}
	}

	if (pch == 0) 
	{
		return return_error(10); //no ready from maria
	}

	if (total_bytes_read == 0) 
	{
		return return_error(7); //no answer from maria
	}

	//fnd = std::string::npos;
	//for (i=58;i>=0;i--) //��� ������ - �����������
	//for (i=0;i<60;i++) //��� ��������

	m_err_cnt = 0;
	//for (i=59;i>0;i--)
	for (j = 0; j < 67; j++) 
	{
		l = wcslen(maria_Errors[j]);
#ifndef __linux__
		sprintf(TMPBUFFER,"%ws", maria_Errors[j]);
#else
		sprintf(TMPBUFFER,"%ls", maria_Errors[j]);
#endif
		pch = subst(INBUFFER, total_bytes_read, TMPBUFFER, l);
		if (pch > 0)
		{
			err_arr[m_err_cnt] = j+11;
			m_err_cnt++;
		}
	}

	if (m_err_cnt>0) return return_error(err_arr[0]);
	
	return 0;
}

uint8_t CAddInNative::OpenPort(void)
{
	char	pcCommPort[32];

    char	INBUFFER[500];
    //char	ZZBUFFER[500];
    char	OUTBUFFER[255];
	char	TMPBUFFER[50];
    char	SMBUFFER[500];
    DWORD        bytes_read    = 0;    // Number of bytes read from port
    DWORD        total_bytes_read    = 0;    
    DWORD        bytes_written = 0;    // Number of bytes written to the port
	int		bStatus;
	DWORD	dwStatus;
	TCHAR	cmdU = 'U';
    //char *  pch = 0;
	int		pch = 0;
	int		i,l;
    int pcw = 0;
    int tmppcw = -1;
	int cnt = m_cnt;

	std::string s;

	TCHAR lpTempPathBuffer[MAX_PATH];

	m_cmd = (L"");
	m_ans = (L"");
	m_err = 0;

	if (m_loging) 
	{
#ifndef __linux__
		dwStatus = GetTempPath(MAX_PATH, lpTempPathBuffer);
		memcpy(lpTempPathBuffer + dwStatus, TEXT("maria.log"),18);
		//uRetVal = GetTempFileName(lpTempPathBuffer, TEXT("maria"), 0, szTempFileName);
	    hTempFile = CreateFile((LPTSTR) lpTempPathBuffer,  GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);                // no template 
		dwStatus = SetFilePointer(hTempFile, 0, NULL, FILE_END);
#else
	// hTempFile = mkstemp((char *) "/tmp/maria.log");
        hTempFile = open("/tmp/maria.log", O_APPEND|O_CREAT);
        if(hTempFile < 0)
            return return_error(1);
#endif
	}

#ifndef __linux__
	sprintf(pcCommPort,"\\\\.\\COM%d", m_port);
	hComm = CreateFileA( pcCommPort,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
	if(hComm == INVALID_HANDLE_VALUE) return return_error(1); //error opening com port

	if(!GetCommState(hComm, &dcb))
	{
		CAddInNative::ClosePort();
		return return_error(2);  //error geting com port settings
	}
	dcb.BaudRate	= m_baud;
	dcb.ByteSize	= 8;
	dcb.Parity		= EVENPARITY;
	dcb.StopBits	= TWOSTOPBITS;
    dcb.fParity		= TRUE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE; 
	if(!SetCommState(hComm, &dcb))
	{
		CAddInNative::ClosePort();
		return return_error(3); //error write com port settings 
	}

	COMMTIMEOUTS timeouts;
	memset(&timeouts,0,sizeof(timeouts));
    timeouts.ReadTotalTimeoutConstant    = 10;
    timeouts.WriteTotalTimeoutConstant   = 100;
    timeouts.ReadIntervalTimeout         = 0; 
    timeouts.ReadTotalTimeoutMultiplier  = 0; 
    timeouts.WriteTotalTimeoutMultiplier = 0;
	//timeouts.ReadTotalTimeoutConstant=500;
	if(!SetCommTimeouts(hComm,&timeouts))
	{
		CAddInNative::ClosePort();
		return return_error(4); //error seting com port timeouts
	}
#else
	if (m_port < 100)
	{
	    sprintf(pcCommPort,"/dev/ttyS%d", m_port);
	    fprintf(m_log_file, "ComPort: /dev/ttyS%d\n", m_port); fflush(m_log_file);

	}
	else
	{
	    sprintf(pcCommPort,"/dev/ttyUSB%d", m_port-100);
	    fprintf(m_log_file, "ComPort: /dev/ttyUSB%d\n", m_port-100); fflush(m_log_file);
	}

	hComm = open (pcCommPort, O_RDWR | O_NOCTTY | O_SYNC);
	if(hComm < 0) 
	{
	    fprintf(m_log_file, "Error %i from open: %s\n", errno, strerror(errno)); fflush(m_log_file);
	    return return_error(1); //error opening com port
	}
	else
	{
//	    fprintf(m_log_file, "hComm: OK\n"); fflush(m_log_file);	    
	}
    
	if(tcgetattr(hComm, &dcb) != 0)
	{
		CAddInNative::ClosePort();
		return return_error(2);  //error geting com port settings
	}

    cfsetospeed (&dcb, m_baud);
    cfsetispeed (&dcb, m_baud);

    dcb.c_cflag = (dcb.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    dcb.c_iflag &= ~IGNBRK;         // disable break processing
    dcb.c_lflag = 0;                // no signaling chars, no echo,
                                    // no canonical processing
    dcb.c_oflag = 0;                // no remapping, no delays
    dcb.c_cc[VMIN]  = 0;            // read doesn't block
    dcb.c_cc[VTIME] = 1;            // deciseconds

/*
    dcb.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    dcb.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                    // enable reading
*/

    dcb.c_cflag |= INPCK;
    dcb.c_cflag |= PARENB;
    dcb.c_cflag &= ~PARODD;
    dcb.c_cflag |= CSTOPB;
    dcb.c_cflag |= CRTSCTS;

    if (tcsetattr (hComm, TCSANOW, &dcb) != 0)
    {
        CAddInNative::ClosePort();
        return return_error(3);  //error write com port settings
    }
#endif


	if (m_loging) 
	{
		sprintf(TMPBUFFER,"====== %s ======", wstrtostr(curver).c_str());
		write_log(TMPBUFFER, 19, 'u');
	}

	//cmdU = (char)253;
	Sleep(200);

	bStatus = WriteFile(hComm, &cmdU, 1, &bytes_written, NULL);
    if (!bStatus )
    {
		CAddInNative::ClosePort();
		return return_error(5); //error while data send
    }
	Sleep(50);
	
	//cmdU = 'U';
	bStatus = WriteFile(hComm, &cmdU, 1, &bytes_written, NULL);
    if (!bStatus )
    {
		CAddInNative::ClosePort();
		return return_error(5); //error while data send
    }
	Sleep(50);
	
	total_bytes_read = 0;
	for (i=0; i<cnt && !pch; i++){
		//Sleep(2);
		bytes_read = 0;
		bStatus = ReadFile(hComm, &SMBUFFER, 10, &bytes_read, NULL);
		if (!bStatus)
		{
			CAddInNative::ClosePort();
			return return_error(6); //error while data recieve
		}
		if (bytes_read > 0){
			memcpy(INBUFFER + total_bytes_read, SMBUFFER, bytes_read);
			total_bytes_read = total_bytes_read+bytes_read;
			INBUFFER[total_bytes_read] = 0;
			//pch = strstr(INBUFFER, "READY");
			pch = subst(INBUFFER, total_bytes_read, "READY", 5);

			pcw = subst(INBUFFER, total_bytes_read, "WRK", 3);
			if (pcw>0 && pcw!=tmppcw) 
			{
				cnt = cnt + m_cnt;
				tmppcw = pcw;
			}
		}
	}
	Sleep(20);
	
    std::string s_tmp = INBUFFER;
	s_tmp.resize(total_bytes_read);
    s = recode("cp866", "UTF-8", s_tmp);
	m_ans = strtowstr(s);

	if (m_loging) 
	{
		sprintf(TMPBUFFER,"%4d %4ld", i, total_bytes_read);
		write_log(TMPBUFFER, 9, 't');
		if (total_bytes_read > 0)
		{
			write_log(INBUFFER, total_bytes_read, 'u');
		}
	}

	if (pch == 0) 
	{
	 	CAddInNative::ClosePort();
		return return_error(10); //no ready from maria
	}

	if (total_bytes_read == 0) 
	{
	 	CAddInNative::ClosePort();
		return return_error(7); //no answer from maria
	}

	m_cmd = (L"SYNC");
	s = wstrtostr(m_cmd);
	l = s.length();

	OUTBUFFER[0] = (char)253;
	memcpy(OUTBUFFER + 1, s.c_str(), l);
	OUTBUFFER[l+1] = l + 1;
	OUTBUFFER[l+2] = (char)254;

	if (m_loging) write_log(OUTBUFFER, l+3, 'c');

	bStatus = WriteFile(hComm, &OUTBUFFER, l+3, &bytes_written, NULL);
    if (!bStatus ) return return_error(5); //error while data send
	
	pch = 0;
	total_bytes_read = 0;
	for (i=0; i<cnt && !pch; i++){
		//Sleep(2);
		bytes_read = 0;
		bStatus = ReadFile(hComm, &SMBUFFER, 10, &bytes_read, NULL);
		
		if (!bStatus) 
		{
			CAddInNative::ClosePort();
			return return_error(6); //error while data recieve
		}

		if (bytes_read>0)
		{
			memcpy(INBUFFER+total_bytes_read, SMBUFFER, bytes_read);
			total_bytes_read += bytes_read;
			INBUFFER[total_bytes_read] = 0;
			//pch = strstr(INBUFFER, "READY");
			pch = subst(INBUFFER, total_bytes_read, "READY", 5);

			pcw = subst(INBUFFER, total_bytes_read, "WRK", 3);
			if (pcw>0 && pcw>tmppcw) 
			{
				cnt = cnt + m_cnt;
				tmppcw = pcw;
			}

		}
	}

	s_tmp = INBUFFER;
	s_tmp.resize(total_bytes_read);
    s = recode("cp866", "UTF-8", s_tmp);
	m_ans = strtowstr(s);

	if (m_loging) 
	{
		sprintf(TMPBUFFER,"%4d %4ld %4d", i, total_bytes_read, cnt);
		write_log(TMPBUFFER, 14, 't');
		if (total_bytes_read > 0)
		{
			write_log(INBUFFER, total_bytes_read, 'a');
		}
	}

	if (pch == 0) 
	{
	 	CAddInNative::ClosePort();
		return return_error(10); //no ready from maria
	}

	if (total_bytes_read == 0) 
	{
	 	CAddInNative::ClosePort();
		return return_error(7); //no answer from maria
	}

	//Sleep(10);

	//std::size_t fnd;
	//for (i=0;i<59;i++)
	//{
	//	fnd = m_ans.find(maria_Errors[i]);
	//	if (fnd != std::string::npos) 
	//	{
	//		//CAddInNative::ClosePort();
	//		return return_error(i+10);
	//	}
	//}

	m_isOpen = true;

	return 0;

}

void CAddInNative::ClosePort(void)
{
	//if (m_isCheckOpen) CAddInNative::CancelCheck(NULL,NULL,0);
	if (m_isOpen) {
#ifndef __linux__
        dcb.fDtrControl = DTR_CONTROL_DISABLE;
		if(!SetCommState(hComm, &dcb))
#else
        dcb.c_cflag &= ~CRTSCTS;
        if(tcgetattr(hComm, &dcb) != 0)
#endif
        {
			//return return_error(3); //error write com port settings 
			write_log("", 3, 'e');
		}
		CloseHandle(hComm);
		Sleep(m_cnt);
	}
	m_isOpen = false;
	if (m_loging) CloseHandle(hTempFile);
	return;
}

bool CAddInNative::CMD(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
	int res = 1;
	std::wstring p0;
    wchar_t *p0_wc = NULL;

    ::convFromShortWchar(&p0_wc, (paParams) -> pwstrVal);
    p0 = p0_wc;
    
	m_cmd = L"" + p0;
	res = send_data();

    TV_VT(pvarRetValue) = VTYPE_UI1;
	TV_UI1(pvarRetValue) = res;

	return true; 
}



bool CAddInNative::Test(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
	int res = 0;

    //if (!lSizeArray || !paParams)
    //    return false;
	m_ans = L"Driver test";

	TV_VT(pvarRetValue) = VTYPE_UI1;
	TV_UI1(pvarRetValue) = res;

	return true; 
}


void string_normalize(std::wstring& inStr, int newLen, uint8_t ls, char ch)
{
		
	int i = inStr.length();
	if (i>newLen) inStr.resize(newLen);

	if (ls==1 && i<newLen) inStr.append((newLen-i), ch);
	if (ls==2 && i<newLen) inStr.insert(0,(newLen-i), ch);
}

bool param_check(std::wstring& inStr, tVariant* paParam, int newLen, int mult)
{
	uint32_t i;
	double d;
	bool ism = false;
	wchar_t wc[256];
	wchar_t pwc[10];
    wchar_t *p0_wc = NULL;

    switch(TV_VT(paParam))
    {
    //case VTYPE_PSTR:
    //    break;
    case VTYPE_PWSTR:
        ::convFromShortWchar(&p0_wc, (paParam) -> pwstrVal);
        inStr = p0_wc;
		if (inStr[0]==L'-'){
			ism = true;
			inStr[0]=L'0';
		}
		string_normalize(inStr, newLen, 2, '0');
		if (ism) inStr[0]=L'1';
        break;
    case VTYPE_I4:
		i = TV_I4(paParam);
		wsprintf(pwc,L"%%0%dd",newLen);
		wsprintf(wc,pwc,i);
		inStr.assign(wc);
		if (inStr[0]==L'-') inStr[0]=L'1';
        break;
    case VTYPE_R8:
		d = TV_R8(paParam);
		i = (int)(d * mult);
		wsprintf(pwc,L"%%0%dd",newLen);
		wsprintf(wc,pwc,i);
 		inStr.assign(wc);
		if (inStr[0]==L'-') inStr[0]=L'1';
       break;
	case VTYPE_BOOL:
		ism = TV_BOOL(paParam);
		if (ism) inStr = L"1"; else inStr = L"0";
		string_normalize(inStr, newLen, 2, '0');
		break;
    default:
        return false;
    }
	return true;
}

std::string wstrtostr(const std::wstring &wstr)
{
	// Convert a Unicode string to an ASCII string
	std::string strTo;

	int result_length = (int)wstr.length()*2;
	char *szTo = new char[result_length + 1];
	memset(szTo, 0, (result_length + 1));
#ifndef __linux__
	WideCharToMultiByte(CP_OEMCP, 0, wstr.c_str(), -1, szTo, (int)wstr.length(), NULL, NULL);
#else
	const wchar_t * wstr_c = wstr.c_str();
	wcsrtombs(szTo, &wstr_c, result_length, NULL);
#endif
	strTo = szTo;
	delete[] szTo;
	return strTo;
}

std::wstring strtowstr(const std::string &str)
{
	// Convert an ASCII string to a Unicode String
	std::wstring wstrTo;
	wchar_t *wszTo = new wchar_t[str.length() + 1];
	wszTo[str.size()] = L'\0';
#ifndef __linux__
	MultiByteToWideChar(CP_OEMCP, 0, str.c_str(), -1, wszTo, (int)str.length());
#else
    const char *str_c = str.c_str();
    mbsrtowcs(wszTo, &str_c, (int)str.length(), NULL);
#endif
	wstrTo = wszTo;
	delete[] wszTo;
	return wstrTo;
}

void IpHexToStr(std::wstring& in, std::wstring& out)
{
	WCHAR wc[100];
	int tmp[4];
	int i;

	for (i=0;i<4;i++) tmp[i]=0;

	swscanf_s(in.c_str(), L"%02x%02x%02x%02x", &tmp[0], &tmp[1], &tmp[2], &tmp[3]);
	wsprintf(wc, L"%d.%d.%d.%d",tmp[0],tmp[1],tmp[2],tmp[3]);
	out = wc;
}

void DelayHexToStr(std::wstring& in, std::wstring& out)
{
	WCHAR wc[100];
	int tmp[4];
	int i;

	for (i=0;i<4;i++) tmp[i]=0;

	swscanf_s(in.c_str(), L"%02x%02x%02x%02x", &tmp[0], &tmp[1], &tmp[2], &tmp[3]);
	wsprintf(wc, L"%d",tmp[0]+tmp[1]*256+tmp[2]*256*256+tmp[3]*256*256*256);
	out = wc;
}

void PortHexToStr(std::wstring& in, std::wstring& out)
{
	WCHAR wc[20];
	int tmp[2];
	int i;

	for (i=0;i<2;i++) tmp[i]=0;

	swscanf_s(in.c_str(), L"%02x%02x", &tmp[0], &tmp[1]);
	wsprintf(wc, L"%d",tmp[0]+tmp[1]*256);
	out = wc;
}

int subst( char * str, int ln, const char * substr, int subln)
{
	int i,j;
	int ret = 0;
	for(i=0; i<(ln-subln); i++)
	{
		if ( *(str+i) == *substr )
		{
			ret = i;
			for(j=1; j<subln && ret==i; j++)
			{
				if ( *(str+i+j) != *(substr+j) ) ret = 0;
			}
			if (ret == i) return ret;
		}
	}
	return ret;
}


