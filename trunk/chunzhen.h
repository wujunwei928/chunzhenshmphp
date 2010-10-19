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

#ifndef _HAVE_CHUNZHEN_H__
#define _HAVE_CHUNZHEN_H__

#include <stdio.h>

#ifdef WIN32
#include <Windows.h>
#endif

#define CHUNZHEN_SEEK_CUR	0x01
#define CHUNZHEN_SEEK_SET	0x02

bool	IPNumtoStr(char* szBuffer, unsigned int nBufferLen, unsigned int nIPNum);

/************************************************************************/
/* IP解析后的信息格式                                                   */
/************************************************************************/
typedef struct _ipEntry {
	unsigned int	nBeginIP;
	unsigned int	nEndIP;
	const char*		szCountry;
	const char*		szArea;
} IPEntry;

/************************************************************************/
/* 纯真IP数据库的包装类                                                 */
/************************************************************************/
class ChunZhenDB
{
public:
	ChunZhenDB(char* szFileName);
	~ChunZhenDB();

	void	Rewind();
	void	Seek(unsigned int nOffset, int nSeekType = CHUNZHEN_SEEK_SET);
	unsigned int ReadLong();
	unsigned int ReadStrOffSet();
	void	GetArea(const char** szBuffer);
	bool	GetLocation(const char* szIPStr, unsigned int nIPLen, 
							IPEntry& stIPEntry);
	bool	GetLocation(unsigned int nIPNum, IPEntry& stIPEntry);
	unsigned int GetDBSize()
	{
		return m_nDBSize;
	}

	static unsigned int	IPtoLong(const char* szIPStr, unsigned int nIPLen);

private:
	unsigned char ReadByte();

	unsigned char	* m_szDBPtr;
	unsigned char	* m_ucCursor;
	unsigned int	m_nDBSize;
#ifdef WIN32
	HANDLE			m_stHandle;
#else
    int             m_fd;
#endif
};

/************************************************************************/
/* 将字符串形式的IP转化成数字类型的工具类                               */
/************************************************************************/
class IPTokenizer
{
public:
	IPTokenizer(const char* szIPStr, unsigned short usIPLen)
	{
		m_szIPStr	= szIPStr;
		m_ucCursor	= m_szIPStr;
		m_usIPLen	= usIPLen;
		if (usIPLen < 7 || usIPLen > 15)
		{
			m_bIsFailed = true;
		}
		m_bIsFailed	= false;
		m_bIsEof	= false;
		m_bySegNum	= 0;
	}
	char	NextChar()
	{
		char cRetVal = 0;
		if(m_bIsFailed) return 0;
		if (m_ucCursor < (m_szIPStr + m_usIPLen))
		{
			cRetVal	= * m_ucCursor;
			m_ucCursor ++;
		} else {
			m_bIsEof	= true;
		}
		return cRetVal;
	}
	unsigned char GetNextSeg()
	{
		unsigned char byRetVal	= 0;
		char cNextChar;
		if(m_bIsFailed)	return 0;
		if (m_bySegNum < 3)
		{
			for(int i = 0; i < 4; ++ i)
			{
				cNextChar = NextChar();
				if(m_bIsEof)
				{
					return byRetVal;
				} else if(i > 0 && cNextChar == '.')
				{
					return byRetVal;
				} else if (i == 3 && cNextChar != '.') {
					m_bIsFailed	= true;
					return 0;
				} else if (cNextChar >= '0' && cNextChar <= '9') {
					byRetVal = byRetVal * 10 + (cNextChar - '0');
				} else {
					m_bIsFailed = true;
					return 0;
				}
			}
			return byRetVal;
		}
		m_bIsFailed	= true;
		return 0;
	}
	unsigned int GetLongVal(){
		unsigned int  nRetVal	= 0;
		if(!m_bIsFailed)
		{
			nRetVal	|=	GetNextSeg() << 24;
			nRetVal	|=  GetNextSeg() << 16;
			nRetVal |=  GetNextSeg() << 8;
			nRetVal	|=	GetNextSeg();
			if(!m_bIsFailed)
			{
				return nRetVal;
			}
		}
		return 0;
	}
private:
	const char		* m_szIPStr;
	const char		* m_ucCursor;
	unsigned short	m_usIPLen;
	unsigned char	m_bySegNum;
	bool			m_bIsFailed;
	bool			m_bIsEof;
};
#endif
