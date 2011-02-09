/************************************************************************ */
/* 以下代码在 PHP 5.2.11 / PHP 5.3.2                                      */
/*   在Visual Studio 2008(Windows XP SP2) PHP5.2.11/Apache2.2测试运行正常 */
/*   在Ubuntu 10.04 PHP5.3.2(FastCGI) Nginx 下测试运行正常                */
/*                                                                        */
/* 性能测试见 BenchResult.txt											  */
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

#define strcasecmp	_stricmp			//Windows上没有 strcasecmp
#define snprintf	_snprintf			//Windows上没有 snprintf
#else
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#endif

/************************************************************************/
/* 将数字类型的IP转化为可读的字符串形式                                 */
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
/* 构造函数，在这个函数中通过文件内存映射将纯真数据库加载到内存中       */
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
			CloseHandle(stTempFile);
			return;
		}
		m_stHandle	= CreateFileMapping(stTempFile, NULL, PAGE_READONLY, 0, 
										m_nDBSize, "CHUNZHEN_IP_DB");
		if(m_stHandle == INVALID_HANDLE_VALUE || m_stHandle == NULL)
		{
			m_nDBSize	= 0;
			m_stHandle	= NULL;
			CloseHandle(stTempFile);
			return;
		}
		m_szDBPtr	= (unsigned char*) MapViewOfFile(m_stHandle, 
											FILE_MAP_READ, 0, 0, m_nDBSize);
		if (m_szDBPtr == NULL)
		{
			m_nDBSize	= 0;
			CloseHandle(m_stHandle);
			CloseHandle(stTempFile);
			m_stHandle	= NULL;
		}
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

	if (m_ucCursor != NULL)
	{
		m_nFirstIP		= ReadLong();
		m_nLastIP		= ReadLong();
		m_nTotalCount	= (m_nLastIP - m_nFirstIP) / 7;
	}
}

/************************************************************************/
/* 析构函数，在这里会关闭掉文件句柄，但不会清空共享内存（多进程）       */
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
/* 从共享内存中读取出区域字段的信息                                     */
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
/* 从共享内存中读取出一个字节                                           */
/************************************************************************/
unsigned char ChunZhenDB::ReadByte()
{
	unsigned char ucRetVal;
	ucRetVal	= * m_ucCursor;
	m_ucCursor ++;
	return ucRetVal;
}

/************************************************************************/
/* 移动指针到要读取的位置                                               */
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
/* 重置指针的位置到最头部                                               */
/************************************************************************/
void ChunZhenDB::Rewind()
{
	m_ucCursor	= m_szDBPtr;
}

/************************************************************************/
/* 从内存中读取出 4 个字节（小头在前）                                  */
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
/* 从内存中读取出 3 个字节的字符串偏移量(小头在前)                      */
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
/* 将字符串形式的IP表示为数字形式                                       */
/************************************************************************/
unsigned int ChunZhenDB::IPtoLong(const char* szIPStr, unsigned int nIPLen)
{
	IPTokenizer stToken(szIPStr, nIPLen);
	return stToken.GetLongVal();
}

/************************************************************************/
/* 解析IP的地理信息                                                     */
/************************************************************************/
bool	ChunZhenDB::GetLocation(const char* szIPStr, unsigned int nIPLen, 
								IPEntry& stIPEntry)
{
	return GetLocation(IPtoLong(szIPStr, nIPLen), stIPEntry);
}

/************************************************************************/
/* 解析IP的地理信息                                                     */
/************************************************************************/
bool	ChunZhenDB::GetLocation(unsigned int nIPNum, IPEntry& stIPEntry)
{
	if(m_szDBPtr == NULL)	return false;

	Rewind();
	unsigned int	nLow		= 0;
	unsigned int	nHigh		= m_nTotalCount;

	unsigned int	nFindIP	= m_nLastIP;
	unsigned int	nBegin	= 0;
	unsigned int	nEnd	= 0;
	while (nLow <= nHigh)
	{
		unsigned int nMiddle	= (nLow + nHigh) / 2;
		Seek(m_nFirstIP + nMiddle * 7);
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
				nFindIP	= m_nFirstIP + nMiddle * 7;
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
	case 0x01://国家和区域信息全部重定向
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
