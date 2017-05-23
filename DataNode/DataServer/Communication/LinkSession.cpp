#include "LinkSession.h"
#include "../DataEcho.h"
#include "../NodeServer.h"


LinkIDSet::LinkIDSet()
{}

LinkIDSet& LinkIDSet::GetSetObject()
{
	static	LinkIDSet	obj;
	return obj;
}

int LinkIDSet::NewLinkID( unsigned int nNewLinkID )
{
	CriticalLock	guard( m_oLock );

	///< ID未添加，可以添加
	if( m_setLinkID.find( nNewLinkID ) == m_setLinkID.end() )
	{
		m_setLinkID.insert( nNewLinkID );
		return 1;
	}

	return 0;
}

void LinkIDSet::RemoveLinkID( unsigned int nRemoveLinkID )
{
	CriticalLock	guard( m_oLock );

	///< 存在这个ID，可以移除
	if( m_setLinkID.find( nRemoveLinkID ) != m_setLinkID.end() )
	{
		m_setLinkID.erase( nRemoveLinkID );
	}
}

unsigned int LinkIDSet::FetchLinkIDList( unsigned int * lpLinkNoArray, unsigned int uiArraySize )
{
	unsigned int	nLinkNum = 0;				///< 有效链路数量
	static	int		s_nLastLinkNoNum = 0;		///< 上一次的链路数量
	CriticalLock	guard( m_oLock );			///< 锁

	if( m_setLinkID.size() != s_nLastLinkNoNum )
	{
		DataNodeService::GetSerivceObj().WriteInfo( "LinkIDSet::FetchLinkIDList() : TCP connection number of QServer fluctuated! new no. = %d, old no. = %d", m_setLinkID.size(), s_nLastLinkNoNum );
		s_nLastLinkNoNum = m_setLinkID.size();
	}

	for( std::set<unsigned int>::iterator it = m_setLinkID.begin(); it != m_setLinkID.end() && nLinkNum < uiArraySize; it++ )
	{
		lpLinkNoArray[nLinkNum++] = *it;
	}

	return nLinkNum;
}


LinkSessions::LinkSessions()
 : m_pDatabase( NULL )
{
}

int LinkSessions::Instance()
{
	DataNodeService::GetSerivceObj().WriteInfo( "LinkSessions::Instance() : initializing ......" );

	int		nErrCode = m_oQuotationBuffer.Initialize();

	if( 0 == nErrCode )
	{
		DataNodeService::GetSerivceObj().WriteInfo( "LinkSessions::Instance() : initialized ......" );
	}
	else
	{
		DataNodeService::GetSerivceObj().WriteError( "LinkSessions::Instance() : failed 2 initialize ..." );
		return nErrCode;
	}

	if( 0 != (nErrCode = ImageRebuilder::GetRebuilder().Initialize()) )	///< 分配10M的快照数据缓存(用于对下初始化)
	{
		DataNodeService::GetSerivceObj().WriteError( "LinkSessions::Instance() : failed 2 initialize Image buffer ..." );
		return -100;
	}

	return 0;
}

void LinkSessions::PushData( unsigned short usMessageNo, unsigned short usFunctionID, const char* lpInBuf, unsigned int uiInSize, bool bPushFlag, unsigned __int64 nSerialNo )
{
	m_oQuotationBuffer.PutMessage( usMessageNo, lpInBuf, uiInSize, nSerialNo );
}

int LinkSessions::CloseLink( unsigned int uiLinkNo )
{
	LinkIDSet::GetSetObject().RemoveLinkID( uiLinkNo );

	return 0;
}

void LinkSessions::OnReportStatus( char* szStatusInfo, unsigned int uiSize )
{
	::sprintf( szStatusInfo, ":working = %s,链路数 = 0 \n", DataNodeService::GetSerivceObj().OnInquireStatus()==true?"true":"false" );
}

bool LinkSessions::OnCommand( const char* szSrvUnitName, const char* szCommand, char* szResult, unsigned int uiSize )
{
	int							nArgc = 32;
	char*						pArgv[32] = { 0 };

	///< 拆解出关键字和参数字符
	if( false == SplitString( pArgv, nArgc, szCommand ) )
	{
		::sprintf( szResult, "LinkSessions::OnCommand : [ERR] parse command string failed" );
		return true;
	}

	CTP_DL_Echo					objEcho4CTPDL;

	///< 执行回显命令串
	return objEcho4CTPDL( pArgv, nArgc, szResult, uiSize );
}

bool LinkSessions::OnNewLink( unsigned int uiLinkNo, unsigned int uiIpAddr, unsigned int uiPort )
{
	return ImageRebuilder::GetRebuilder().AddNewReqSession( uiLinkNo );
}

void LinkSessions::OnCloseLink( unsigned int uiLinkNo, int iCloseType )
{
	LinkIDSet::GetSetObject().RemoveLinkID( uiLinkNo );
}

bool LinkSessions::OnRecvData( unsigned int uiLinkNo, unsigned short usMessageNo, unsigned short usFunctionID, bool bErrorFlag, const char* lpData, unsigned int uiSize, unsigned int& uiAddtionData )
{

	return false;
}





