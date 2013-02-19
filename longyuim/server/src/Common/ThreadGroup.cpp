

#include "ThreadGroup.h"

// $_FUNCTION_BEGIN *****************************
// �������ƣ�CThreadGroup
// ������������ 
// �� �� ֵ����
// ����˵��������
// $_FUNCTION_END *******************************
CThreadGroup::CThreadGroup(void)
{
	mbStop = true;
	
}

// $_FUNCTION_BEGIN *****************************
// �������ƣ�~CThreadGroup
// ������������ 
// �� �� ֵ����
// ����˵��������
// $_FUNCTION_END *******************************
CThreadGroup::~CThreadGroup(void)
{
	if(false == mbStop)
	{
		StopAll();
	}
}

// $_FUNCTION_BEGIN *****************************
// �������ƣ�Sleep
// ����������
//			unsigned int ldwWaitTime				ʱ��
// �� �� ֵ��
//			false							�ǵ�ʱ
//			true							��ʱ
// ����˵�����߳����ṩ�ĵȴ�����
// $_FUNCTION_END *******************************
//��ʱʱ����ʱ�����޹أ��κ�һ���������㼴����	zyw
bool CThreadGroup::Sleep(unsigned int ldwWaitTime)
{
	if(false == mbStop)
	{
		mbStop = true;
		int miResult = moStopEvent.WaitForEvent(ldwWaitTime);
		if(0 == miResult || -1 == miResult)
		{
			return true;
		}
	}
	return false;
}


//���������߳�
void CThreadGroup::WakeAll()
{
	if(mbStop)
	{
		moStopEvent.SetEvent();
		mbStop = false;
	}
}

// $_FUNCTION_BEGIN *****************************
// �������ƣ�Start
// ����������
//			ThreadFunc* apThreadFunc		�̺߳���
//			void* apWorkContext				�����ռ�(this)
//			unsigned int auiThreadNum		����������
// �� �� ֵ��
//			unsigned int					�ɹ��������߳�����
// ����˵����
//			��ʼ�̵߳�������������ָ���̺߳���
// $_FUNCTION_END *******************************
unsigned int CThreadGroup::Start(ThreadFunc* apThreadFunc,\
								 void* apWorkContext,\
								 unsigned int auiThreadNum,\
								 char* apThreadName,\
								 int aiPriority)
{
	ASSERT(apThreadFunc);
	STRU_THREAD_CONTEXT* lpThreadContext = NULL;
	unsigned int luiCounter = 0;

	CAutoLock loLock(moAccessCS);
	//�����ֹͣ״̬,��������ֹͣ���
	if(mbStop)
	{
		mbStop = false;
		if (!moStopEvent.Create(false, false))
		{
			TRACE(1,"CThreadGroup::Initialize �����¼�ʧ�ܡ�");
			return luiCounter;
		}
	}
	try
	{
		for(luiCounter = 0; luiCounter < auiThreadNum; luiCounter++)
		{
			lpThreadContext = NULL;
			lpThreadContext = new STRU_THREAD_CONTEXT();
			lpThreadContext->mpGroupManager = this;
			lpThreadContext->mpWorkContext = apWorkContext;
			lpThreadContext->moThreadStat.SetThreadName(apThreadName, luiCounter);
			lpThreadContext->miPriority = aiPriority;

			if(StartThread(lpThreadContext, apThreadFunc))
			{
				moThreadList.push_back(lpThreadContext);
				lpThreadContext = NULL;
			}
			else
			{
				delete lpThreadContext;
				lpThreadContext = NULL;
			}
		}
	}
	catch(...)
	{
		//ɾ�������Ļ�������
		TRACE(1, "CThreadGroup::Start �����߳�ʧ�ܡ�");
		if (lpThreadContext)
		{
			delete lpThreadContext;
			lpThreadContext = NULL;
		}
	}
	return luiCounter;
}

//�����߳�
#ifdef WIN32 
bool CThreadGroup::StartThread(STRU_THREAD_CONTEXT* apContext,ThreadFunc* apThreadFunc)
{
	apContext->mhHandle = (HANDLE)_beginthreadex(	NULL, NULL,\
        (StdThreadFunc*)apThreadFunc, apContext, NULL,\
        &(apContext->muiThreadID));
	
	//��ϵͳ�����̴߳ﵽ����ʱ������ֵΪ0������ʧ�ܷ���-1
	if (apContext->mhHandle > 0)
	{
		//�����߳����ȼ�
		SetThreadPriority((HANDLE)apContext->mhHandle, apContext->miPriority);
		return true;
	}
	TRACE(1, "CThreadGroup::StartThread �����߳�ʧ�� ERROR_NO:" << GetLastError());
	return false;
}
#else
bool CThreadGroup::StartThread(STRU_THREAD_CONTEXT* apContext, ThreadFunc* apThreadFunc)
{
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	// ���÷����߳��̣߳��ŵ㣺1���ٶȺܿ�;2���Զ��ͷ���Դ
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setstacksize(&attr, 2000000);//�����̷߳����ջ�Ĵ�С����λ���ֽ�

	int liResult = pthread_create(&(apContext->muiThreadID), &attr,\
		(StdThreadFunc*)apThreadFunc, apContext);
	pthread_attr_destroy(&attr);

	if(0 == liResult)
	{
		//�����߳����ȼ�
		//pthread_setschedparam(apContext->muiThreadID, apContext->miPriority, NULL);
		//ȡ���̴߳���ʱ��
		memset(&apContext->moStartTime, 0, sizeof(apContext->moStartTime));
		ftime(&apContext->moStartTime);
		liResult = pthread_detach(apContext->muiThreadID);
		//TRACE(1, "CThreadGroup::StartThread pthread_detach ����ֵ: "<<liResult);

		return true;
	}
	TRACE(1,"CThreadGroup::StartThread �����߳�ʧ�ܡ�ԭ��:" << liResult);
	return false;
}
#endif
// $_FUNCTION_BEGIN *****************************
// �������ƣ�StopAll
// ������������ 
// �� �� ֵ����
// ����˵����ֹͣ���߳�
// $_FUNCTION_END *******************************
bool CThreadGroup::StopAll()
{
	int liThread = (int)moThreadList.size();
	list<STRU_THREAD_CONTEXT*>::iterator loIterator;
	int liTermainte = 0;
	
	if (!IsStop())
	{
		CAutoLock loLock(moAccessCS);
		mbStop = true;	//��������ֹͣ���
		//�����˳��¼�,ʹ���е�Sleep����ʧ��
		moStopEvent.SetEvent();

		//�ȴ����е��߳��˳�
		for(loIterator = moThreadList.begin(); loIterator != moThreadList.end(); loIterator++)
		{
			if (!EndThread(*loIterator))
			{
				liTermainte++;
			}
			delete (*loIterator);
		}
		//������е��߳���Ϣ
		moThreadList.clear();

		//�˳��¼�
		moStopEvent.Close();
	}

	if(liTermainte)
	{
		TRACE(1,"CThreadGroup::StopAll ǿ�н����߳�����" << liTermainte
			<< " ������������:" << liThread-liTermainte);
	}
	else
	{
		TRACE(1,"CThreadGroup::StopAll �����߳�����������");
	}
	return true;
}

//ֹͣ�߳�
#ifdef WIN32 
bool CThreadGroup::EndThread(STRU_THREAD_CONTEXT* apContext)
{
	bool lbResult = true;
	if(WaitForSingleObject(apContext->mhHandle, 2000) != WAIT_OBJECT_0)
	{
		TRACE(1,"CThreadGroup::EndThread ǿ�н����߳�" << apContext->moThreadStat.GetThreadName());
		TerminateThread(apContext->mhHandle, 0xFFFFFFFF);
		lbResult = false;
	}
	CloseHandle(apContext->mhHandle);
	return lbResult;
}

#else
bool CThreadGroup::EndThread(STRU_THREAD_CONTEXT* apContext)
{
	bool lbResult = true;
	int liWaitCount = 0;
	while ((liWaitCount++ < 12))// && (0 == pthread_kill(apContext->muiThreadID, 0)))
	{	
		usleep(99000);//΢�뼶��Լ100ms															
	}
	//if (0 == pthread_kill(apContext->muiThreadID, 0))
	//{
	//	TRACE(1,"CThreadGroup::EndThread ǿ�н����߳�:" << apContext->moThreadStat.GetThreadName() 
	//		<< " ID:" << apContext->muiThreadID);
	//	pthread_cancel(apContext->muiThreadID);
	//	lbResult = false;
	//}
	return lbResult;
}
#endif

// $_FUNCTION_BEGIN *****************************
// �������ƣ�Dump
// ������������ 
// �� �� ֵ����
// ����˵�������������Ϣ
// $_FUNCTION_END *******************************
void CThreadGroup::Dump()
{
	list<STRU_THREAD_CONTEXT*>::iterator loIterator;

	CAutoLock loLock(moAccessCS);
	TRACE(1,"CThreadGroup::Dump �߳�����:" << moThreadList.size());
	for(loIterator = moThreadList.begin(); loIterator != moThreadList.end(); loIterator++)
	{
		PrintThreadInfo(**loIterator);
	}
}

// $_FUNCTION_BEGIN *****************************
// �������ƣ�PrintThreadInfo
// ����������
//			STRU_THREAD_CONTEXT&			�̻߳���
// �� �� ֵ����
// ����˵����
//			���ϵͳ��Ϣ��WIN32ƽ̨
// $_FUNCTION_END *******************************
#ifdef WIN32 
void CThreadGroup::PrintThreadInfo(STRU_THREAD_CONTEXT& aoThreadContext)
{
	ASSERT(&aoThreadContext != NULL);

	//��ӡ�߳���Ϣ
	TRACE(1,"CThreadGroup::Dump �߳�ID:" << aoThreadContext.muiThreadID
				<< "���:" << aoThreadContext.mhHandle
				<< "�߳�����:" << aoThreadContext.moThreadStat.GetThreadName()
				<< "����:" << aoThreadContext.moThreadStat.GetPerformance()
				<< "���ȼ�:" << aoThreadContext.miPriority);

    FILETIME lstruCreationTime;
    FILETIME lstruExitTime;
    FILETIME lstruKernelTime;
    FILETIME lstruUserTime;

	if(GetThreadTimes(aoThreadContext.mhHandle, &lstruCreationTime, &lstruExitTime, &lstruKernelTime, &lstruUserTime))
	{
		PrintThreadTime("����ʱ��", lstruCreationTime);
	
		unsigned int	ldwExitCode = NULL;
		//����˳�ʱ��
		if(GetExitCodeThread(aoThreadContext.mhHandle, &ldwExitCode))
		{
			if(ldwExitCode != STILL_ACTIVE)
			{
				char lszExit[32];
				sprintf(lszExit, "�˳�����%u ʱ��", ldwExitCode);
				PrintThreadTime(lszExit, lstruExitTime);
			}
			else
			{
				TRACE(1, "CThreadGroup::Dump �߳�û���˳���������:"
					<< aoThreadContext.moThreadStat.GetAliveCounter());
			}
		}	

		PrintThreadTime("�ں�ʱ��", lstruKernelTime);
		PrintThreadTime("�û�ʱ��", lstruUserTime);
	}
}
//������������߳�ʱ��
void CThreadGroup::PrintThreadTime(char* apTime, FILETIME& aoFileTime)
{
	FILETIME lstruLocalTime;
	if (false == FileTimeToLocalFileTime(&aoFileTime, &lstruLocalTime))
	{
		return;
	}

	SYSTEMTIME lstruSystemTime;
	if(false == FileTimeToSystemTime(&lstruLocalTime, &lstruSystemTime))
	{
		return;
	}

	TRACE(1,"CThreadGroup::Dump " << apTime << " " 
		<< lstruSystemTime.wYear << "-" << lstruSystemTime.wMonth << "-" << lstruSystemTime.wDay << "-" 
		<< lstruSystemTime.wHour << "-" << lstruSystemTime.wMinute << "-" << lstruSystemTime.wSecond << "-" 
		<< lstruSystemTime.wMilliseconds);
}

// $_FUNCTION_BEGIN *****************************
// �������ƣ�PrintThreadInfo
// ����������
//			STRU_THREAD_CONTEXT&			�̻߳���
// �� �� ֵ����
// ����˵����
//			���ϵͳ��Ϣ��Linuxƽ̨
// $_FUNCTION_END *******************************
#else
void CThreadGroup::PrintThreadInfo(STRU_THREAD_CONTEXT& aoThreadContext)
{
	ASSERT(&aoThreadContext != NULL);

	//��ӡ�߳���Ϣ
	TRACE(1,"CThreadGroup::Dump " << "�߳�ID:" << aoThreadContext.muiThreadID
				<< " �߳�����:" << aoThreadContext.moThreadStat.GetThreadName()
				<< " ����:" << aoThreadContext.moThreadStat.GetPerformance()
				<< " ���ȼ�:" << aoThreadContext.miPriority);
	
	struct tm* lptm = localtime(&aoThreadContext.moStartTime.time);
	char lszTemp[256];
	sprintf(lszTemp, "%04d-%02d-%02d %02d:%02d:%02d:%03d",
		1900+lptm->tm_year, 1+lptm->tm_mon, lptm->tm_mday,
		lptm->tm_hour, lptm->tm_min, lptm->tm_sec, aoThreadContext.moStartTime.millitm);
	TRACE(1,"CThreadGroup::Dump ����ʱ�� " << lszTemp);

	//��sig����Ϊ0ʱ�����������д����飬�������źţ��������tid�ĺϷ��ԡ�
	if (0 == pthread_kill(aoThreadContext.muiThreadID, 0))
	{
		TRACE(1,"CThreadGroup::Dump �߳�û���˳���������:"
			<< aoThreadContext.moThreadStat.GetAliveCounter());
	}
	else
	{
		TRACE(1,"CThreadGroup::Dump �߳��Ѿ��˳���������:"
			<< aoThreadContext.moThreadStat.GetAliveCounter());
	}
}
#endif

// $_FUNCTION_BEGIN *****************************
// �������ƣ�Detach
// ������������ 
// �� �� ֵ����
// ����˵����
//			�͹����߷���
//			���������߳��е��ã����ú��̵߳Ļ��������ٷ�����
// $_FUNCTION_END *******************************
bool CThreadGroup::Detach(STRU_THREAD_CONTEXT& apThread)
{
	bool lbRet = false;

	list<STRU_THREAD_CONTEXT*>::iterator loIterator;

	//�����ֹͣ״̬,��������ֹͣ���
	if(mbStop)
	{
		TRACE(1,"CThreadGroup::Detach ���ڽ���״̬��");
		return true;
	}

	{
		CAutoLock loLock(moAccessCS);
		for(loIterator = moThreadList.begin(); loIterator != moThreadList.end(); loIterator++)
		{
			if((*loIterator) == &apThread)
			{
				lbRet = true;
				moThreadList.erase(loIterator);
				break;
			}
		}

	}

	if(lbRet)
	{
		TRACE(1,"CThreadGroup::Detach �̷߳��롣");
		PrintThreadInfo(apThread);

		delete (&apThread);
	}
	return lbRet;
}