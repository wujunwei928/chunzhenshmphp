/************************************************************************ */
/* ���´����� PHP 5.2.11 / PHP 5.3.2                                      */
/*   ��Visual Studio 2008(Windows XP SP2) PHP5.2.11/Apache2.2������������ */
/*   ��Ubuntu 10.04 PHP5.3.2(FastCGI) Nginx �²�����������                */
/*                                                                        */
/* ���ܲ��Լ� BenchResult.txt											  */
/**************************************************************************/
/* @author wps2000                                                        */
/* @date 2010-09-21                                                       */
/* @email	zhangsilly@gmail.com                                          */
/**************************************************************************/
/* Usage:                                                                 */
/* print_r(chunzhen_getIPLocation('192.168.16.4'));                       */
/*                                                                        */
/* php.ini Configuration                                                  */
/* [Chunzhen]                                                             */
/* chunzhen_shm.chunzhen_db="D:\QQWry.Dat"                                */
/**************************************************************************/

#include "chunzhen.h"

#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef WIN32
#include <io.h>

#define strcasecmp	_stricmp			//Windows��û�� strcasecmp
#define snprintf	_snprintf			//Windows��û�� snprintf
#else
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#endif

/************************************************************************/
/* ���������͵�IPת��Ϊ�ɶ����ַ�����ʽ                                 */
/************************************************************************/
bool	IPNumtoStr(char* szBuffer, unsigned int nBufferLen, unsigned int nIPNum)
{
	snprintf(szBuffer, nBufferLen, "%d.%d.%d.%d",
						(nIPNum >> 24) & 0xFF,
						(nIPNum >> 16) & 0xFF,
						(nIPNum >> 8) & 0xFF,
						nIPNum & 0xFF);
	return true;
}

/************************************************************************/
/* ���캯���������������ͨ���ļ��ڴ�ӳ�佫�������ݿ���ص��ڴ���       */
/************************************************************************/
ChunZhenDB::ChunZhenDB(char* szFileName)
{
	m_nDBSize	= 0;
	m_szDBPtr	= NULL;
#ifdef WIN32
	m_stHandle      = NULL;
	HANDLE	stTempFile	= CreateFile((LPCTSTR) szFileName,
										GENERIC_READ,
										FILE_SHARE_READ,
										NULL,
										OPEN_EXISTING,
										FILE_ATTRIBUTE_NORMAL,
										NULL);

	if (stTempFile != INVALID_HANDLE_VALUE)
	{
		m_nDBSize	= GetFileSize(stTempFile, NULL);
		if(m_nDBSize == 0xFFFFFFFF)
		{
			m_nDBSize	= 0;
			return;
		}
		m_stHandle	= CreateFileMapping(stTempFile, NULL, PAGE_READONLY, 0, 
										m_nDBSize, "CHUNZHEN_IP_DB");
		if(m_stHandle == INVALID_HANDLE_VALUE)
		{
			m_nDBSize	= 0;
			m_stHandle	= NULL;
			return;
		}
		m_szDBPtr	= (unsigned char*) MapViewOfFile(m_stHandle, 
											FILE_MAP_READ, 0, 0, m_nDBSize);
	}
	CloseHandle(stTempFile);
#else
    m_fd    = open(szFileName, O_RDONLY);
    if (m_fd != -1)
    {
        struct stat stFileInfo;
        fstat(m_fd, &stFileInfo);
        m_nDBSize   = stFileInfo.st_size;
        m_szDBPtr   = (unsigned char*) mmap(NULL, m_nDBSize, PROT_READ,
											MAP_SHARED, m_fd, 0);
    }
#endif
	m_ucCursor	= m_szDBPtr;
}

/************************************************************************/
/* �����������������رյ��ļ��������������չ����ڴ棨����̣�       */
/************************************************************************/
ChunZhenDB::~ChunZhenDB()
{
#ifdef WIN32
	CloseHandle(m_stHandle);
#else
    close(m_fd);
#endif
}

/************************************************************************/
/* �ӹ����ڴ��ж�ȡ�������ֶε���Ϣ                                     */
/************************************************************************/
void ChunZhenDB::GetArea(const char** ppStrPtr)
{
	char	cTemp = 0;
	cTemp	= ReadByte();
	switch (cTemp)
	{
	case 0x00:
		return;
	case 0x01:
	case 0x02:
		Seek(ReadStrOffSet());
		*ppStrPtr	= (const char*) m_ucCursor;
		return;
	default:
		*ppStrPtr	= (const char*) (m_ucCursor - 1);
	}
}

/************************************************************************/
/* �ӹ����ڴ��ж�ȡ��һ���ֽ�                                           */
/************************************************************************/
unsigned char ChunZhenDB::ReadByte()
{
	unsigned char ucRetVal;
	ucRetVal	= * m_ucCursor;
	m_ucCursor ++;
	return ucRetVal;
}

/************************************************************************/
/* �ƶ�ָ�뵽Ҫ��ȡ��λ��                                               */
/************************************************************************/
void ChunZhenDB::Seek(unsigned int nOffset, int nSeekType)
{
	if (nSeekType == CHUNZHEN_SEEK_SET)
	{
		m_ucCursor	= m_szDBPtr + nOffset;
	} else if (nSeekType == CHUNZHEN_SEEK_CUR)
	{
		m_ucCursor	+= nOffset;
	}
}

/************************************************************************/
/* ����ָ���λ�õ���ͷ��                                               */
/************************************************************************/
void ChunZhenDB::Rewind()
{
	m_ucCursor	= m_szDBPtr;
}

/************************************************************************/
/* ���ڴ��ж�ȡ�� 4 ���ֽڣ�Сͷ��ǰ��                                  */
/************************************************************************/
unsigned int ChunZhenDB::ReadLong()
{
	unsigned int ulRetVal = 0;
	ulRetVal	|=	ReadByte();
	ulRetVal	|=	ReadByte() << 8;
	ulRetVal	|=	ReadByte() << 16;
	ulRetVal	|=	ReadByte() << 24;
	return ulRetVal;
}

/************************************************************************/
/* ���ڴ��ж�ȡ�� 3 ���ֽڵ��ַ���ƫ����(Сͷ��ǰ)                      */
/************************************************************************/
unsigned int ChunZhenDB::ReadStrOffSet()
{
	unsigned int ulRetVal	= 0;
	ulRetVal	|=	ReadByte();
	ulRetVal	|=	ReadByte() << 8;
	ulRetVal	|=	ReadByte() << 16;
	return ulRetVal;
}

/************************************************************************/
/* ���ַ�����ʽ��IP��ʾΪ������ʽ                                       */
/************************************************************************/
unsigned int ChunZhenDB::IPtoLong(const char* szIPStr, unsigned int nIPLen)
{
	IPTokenizer stToken(szIPStr, nIPLen);
	return stToken.GetLongVal();
}

/************************************************************************/
/* ����IP�ĵ�����Ϣ                                                     */
/************************************************************************/
bool	ChunZhenDB::GetLocation(const char* szIPStr, unsigned int nIPLen, 
								IPEntry& stIPEntry)
{
	return GetLocation(IPtoLong(szIPStr, nIPLen), stIPEntry);
}

/************************************************************************/
/* ����IP�ĵ�����Ϣ                                                     */
/************************************************************************/
bool	ChunZhenDB::GetLocation(unsigned int nIPNum, IPEntry& stIPEntry)
{
	if(m_szDBPtr == NULL)	return false;

	Rewind();
	unsigned int	nLow		= 0;
	unsigned int	nFirstIP	= ReadLong();
	unsigned int	nLastIP		= ReadLong();
	unsigned int	nTotalCount	= (nLastIP - nFirstIP) / 7;
	unsigned int	nHigh		= nTotalCount;

	unsigned int	nFindIP	= nLastIP;
	unsigned int	nBegin	= 0;
	unsigned int	nEnd	= 0;
	while (nLow <= nHigh)
	{
		unsigned int nMiddle	= (nLow + nHigh) / 2;
		Seek(nFirstIP + nMiddle * 7);
		nBegin	= ReadLong();
		if (nIPNum < nBegin)
		{
			nHigh	= nMiddle - 1;
		} else {
			Seek(ReadStrOffSet());
			nEnd	= ReadLong();
			if (nIPNum > nEnd)
			{
				nLow	= nMiddle + 1;
			} else {
				nFindIP	= nFirstIP + nMiddle * 7;
				break;
			}
		}
	}
	Seek(nFindIP);
	stIPEntry.nBeginIP	= ReadLong();
	unsigned int nOffset	= ReadStrOffSet();
	Seek(nOffset);
	stIPEntry.nEndIP		= ReadLong();

	char  cTemp = 0;
	cTemp	= ReadByte();
	switch(cTemp)
	{
	case 0x01://���Һ�������Ϣȫ���ض���
		nOffset	= ReadStrOffSet();
		Seek(nOffset);
		cTemp	= ReadByte();
		switch(cTemp)
		{
		case 0x02:
			Seek(ReadStrOffSet());
			stIPEntry.szCountry	= (const char*) m_ucCursor;
			Seek(nOffset + 4);
			GetArea(&stIPEntry.szArea);
			break;
		default:
			stIPEntry.szCountry	= (const char*) (m_ucCursor - 1);
			Seek(strlen((const char*) m_ucCursor) + 1, CHUNZHEN_SEEK_CUR);
			GetArea(&stIPEntry.szArea);
		}
		break;
	case 0x02:
		Seek(ReadStrOffSet());
		stIPEntry.szCountry	= (const char*) m_ucCursor;
		Seek(nOffset + 8);
		GetArea(&stIPEntry.szArea);
		break;
	default:
		stIPEntry.szCountry	= (const char*) (m_ucCursor - 1);
		Seek(strlen((const char*) m_ucCursor) + 1, CHUNZHEN_SEEK_CUR);
		GetArea(&stIPEntry.szArea);
	}

	if(strcasecmp(stIPEntry.szCountry, " CZ88.NET") == 0)
	{
		stIPEntry.szCountry	= "unkown";
	}
	if(strcasecmp(stIPEntry.szArea, " CZ88.NET") == 0)
	{
		stIPEntry.szArea	= "unkown";
	}

	return true;
}
