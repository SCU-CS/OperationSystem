#include<windows.h>
#include<fstream>
#include<iostream>
#include<stdio.h>
#include<string>
#include<conio.h>
using namespace std;
//����һЩ������

//����������������ٽ�������
#define MAX_BUFFER_NUM	10

//�뵽΢��ĳ˷����ӣ�
#define INTE_PER_SEC 1000

//�����������������������̵߳�������
#define MAX_THREAD_NUM 64

//����һ���ṹ����¼�ڲ����ļ���ָ����ÿһ���̵߳Ĳ���
struct ThreadInfo
{
	int	serial;								//�߳����к�
	char	entity;							//��P����C
	double	delay;							//�߳��ӳ�
	int	thread_request[MAX_THREAD_NUM]; 	//�߳��������
	int	n_request;							//�������
};

//ȫ�ֱ����Ķ���

//�ٽ������������,���ڹ����������Ļ�����ʣ�
int	        Buffer_Critical[MAX_BUFFER_NUM]; //���������������ڴ�Ų�Ʒ��
ThreadInfo	Thread_Info[MAX_THREAD_NUM];	 //�߳���Ϣ���飻
HANDLE		h_Thread[MAX_THREAD_NUM];		 //���ڴ洢ÿ���߳̾�������飻

HANDLE	empty_semaphore;					 //һ���ź�����
HANDLE	h_mutex;							 //һ����������
HANDLE	h_Semaphore[MAX_THREAD_NUM];		 //���������������߿�ʼ���ѵ��ź�����
CRITICAL_SECTION	PC_Critical[MAX_BUFFER_NUM];	

DWORD		n_Thread = 0;					 //ʵ�ʵ��̵߳���Ŀ��
DWORD		n_Buffer_or_Critical;			 //ʵ�ʵĻ����������ٽ�������Ŀ��

//�������Ѽ���������������
void  Produce(void *p);
void  Consume(void *p);	
bool  IfInOtherRequest(int);
int	  FindProducePositon();
int	  FindBufferPosition(int);

int main(void)
{
	//�������������
	DWORD		wait_for_all;
	ifstream	inFile;
	
	/*if (argc!=2)	{
		printf("Usage:%s <File>\n",argv[0]);
		return 1;
	}*/

	//��ʼ����������
	for(int i=0;i< MAX_BUFFER_NUM;i++)
		Buffer_Critical[i] = -1;		
	
	//��ʼ��ÿ���̵߳�������У�
	for(int j=0;j<MAX_THREAD_NUM;j++) {
		for(int k=0;k<MAX_THREAD_NUM;k++)
			Thread_Info[j].thread_request[k] = -1;

		Thread_Info[j].n_request = 0;
	}
	
	//��ʼ���ٽ�����
	for(i =0;i< MAX_BUFFER_NUM;i++) 
		InitializeCriticalSection(&PC_Critical[i]);

	//�������ļ������չ涨�ĸ�ʽ��ȡ�̵߳���Ϣ��
	inFile.open("test.txt");

	//���ļ��л��ʵ�ʵĻ���������Ŀ��
	inFile >> n_Buffer_or_Critical;
	inFile.get();
	printf("�����ļ���:\n");

	//���Ի�õĻ���������Ŀ��Ϣ��
	printf("%d \n",(int) n_Buffer_or_Critical);

	//��ȡÿ���̵߳���Ϣ����Ӧ���ݽṹ�У�
	while(inFile){
		inFile >> Thread_Info[n_Thread].serial;
		inFile >> Thread_Info[n_Thread].entity;
		inFile >> Thread_Info[n_Thread].delay;

		char c;
		inFile.get(c);

		while(c!='\n'&& !inFile.eof()) {
		 inFile>> Thread_Info[n_Thread].thread_request[Thread_Info[n_Thread].n_request++];
		 inFile.get(c);	
		}
		
		n_Thread++;
	}  

	//���Ի�õ��߳���Ϣ������ȷ����ȷ�ԣ�	
	for(j=0;j<(int) n_Thread;j++) {
		int    Temp_serial  = Thread_Info[j].serial;
		char   Temp_entity  = Thread_Info[j].entity;
		double Temp_delay   = Thread_Info[j].delay;
		
		printf(" \nthread%2d    %c    %f   ",Temp_serial,Temp_entity,Temp_delay);

		int Temp_request = Thread_Info[j].n_request;

		for(int k=0;k<Temp_request;k++)
			printf(" %d    ", Thread_Info[j].thread_request[k]);

		cout<<endl;
	}
	
	printf("\n\n");

 	//������ģ������м�����Ҫ���ź���
	empty_semaphore = CreateSemaphore(NULL,n_Buffer_or_Critical,n_Buffer_or_Critical,
								  "semaphore_for_empty");
	h_mutex	= CreateMutex(NULL,FALSE,"mutex_for_update");

	//�������ѭ�����̵߳�ID����Ϊ��Ӧ�����̵߳Ĳ�Ʒ��дʱ��
	//ʹ�õ�ͬ���ź���������
	for(j=0;j<(int)n_Thread;j++) {  			
		std::string lp ="semaphore_for_produce_";
		int temp =j;
		while(temp){
		char c = (char)(temp%10);
		lp+=c;
		temp/=10;
		}

		h_Semaphore[j+1]=CreateSemaphore(NULL,0,n_Thread,lp.c_str());
	}

	//���������ߺ��������̣߳�
	for(i =0;i< (int) n_Thread;i++) {
		if(Thread_Info[i].entity =='P')
			h_Thread[i]= CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)(Produce),
									&(Thread_Info[i]),0,NULL);
		else
		   h_Thread[i]=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)(Consume),
								&(Thread_Info[i]),0,NULL);
	}

	//������ȴ������̵߳Ķ���������
	wait_for_all = WaitForMultipleObjects(n_Thread,h_Thread,TRUE,-1);

	printf(" \n \nALL Producer and consumer have finished their work. \n");
	printf("Press any key to quit!\n");
	_getch();
	return 0;
}

//ȷ���Ƿ��ж�ͬһ��Ʒ����������δִ�У�
bool IfInOtherRequest(int req)
{
	for(int i=0;i<n_Thread;i++)
		for(int j=0;j<Thread_Info[i].n_request;j++)
			if(Thread_Info[i].thread_request[j] == req)
				return TRUE;

	return FALSE;
}

//�ҳ���ǰ���Խ��в�Ʒ�����Ŀջ�����λ�ã�
int	FindProducePosition()
{
	int EmptyPosition;
	for (int i =0;i<n_Buffer_or_Critical;i++)
		if(Buffer_Critical[i] == -1) {
			EmptyPosition = i;

			//�������������ֵ��ʾ�������������ڱ�д״̬��
			Buffer_Critical[i] = -2;
			break;
		}
	return  EmptyPosition;
}

//�ҳ���ǰ���������������Ĳ�Ʒ��λ�ã�
int FindBufferPosition(int ProPos)
{
	int TempPos;
	for (int i =0 ;i<n_Buffer_or_Critical;i++)
		if(Buffer_Critical[i]==ProPos){
			TempPos = i;
			break;
		}
	return TempPos;
}

//�����߽���
void Produce(void *p)
{
	//�ֲ�����������
	DWORD	wait_for_semaphore,wait_for_mutex,m_delay;
	int		m_serial;

	//��ñ��̵߳���Ϣ��
	m_serial = ((ThreadInfo*)(p))->serial;
	m_delay  = (DWORD)(((ThreadInfo*)(p))->delay *INTE_PER_SEC);

	Sleep(m_delay);
	//��ʼ��������
	printf("Producer %2d sends the produce require.\n",m_serial);

	//ȷ���пջ������ɹ�������ͬʱ����λ����empty��1�����������ߺ������ߵ�ͬ����
	wait_for_semaphore	=  WaitForSingleObject(empty_semaphore,-1);
						
	//���������һ�������������Ŀ��ٽ�����ʵ��дд���⣻
	wait_for_mutex  = WaitForSingleObject(h_mutex,-1);
	int  ProducePos = FindProducePosition();
    ReleaseMutex(h_mutex);

	//�������ڻ���Լ��Ŀ�λ�ò����ϱ�Ǻ����µ�д������������֮����Բ�����
	//�������������У����������ߵ�ID��Ϊ��Ʒ��ŷ��룬����������ʶ��;
	printf("Producer %2d begin  to produce at position %2d.\n",m_serial,ProducePos);
	Buffer_Critical[ProducePos] = m_serial;
	printf("Producer %2d finish producing :\n ",m_serial);
	printf("	 position[ %2d ]:%3d \n\n" ,ProducePos,Buffer_Critical[ProducePos]);

	//ʹ������д�Ļ��������Ա����������ʹ�ã�ʵ�ֶ�дͬ����
	ReleaseSemaphore(h_Semaphore[m_serial],n_Thread,NULL);
}

//�����߽���
void Consume(void * p)
{
	//�ֲ�����������
	DWORD	wait_for_semaphore,m_delay;
	int	m_serial,m_requestNum;				//�����ߵ����кź��������Ŀ��
	int	m_thread_request[MAX_THREAD_NUM];	//�������̵߳�������У�
    
	//��ȡ���̵߳���Ϣ�����أ�
	m_serial = ((ThreadInfo*)(p))->serial;
	m_delay  = (DWORD)(((ThreadInfo*)(p))->delay *INTE_PER_SEC);
	m_requestNum = ((ThreadInfo *)(p))->n_request;
	
	for (int i = 0;i<m_requestNum;i++)
		m_thread_request[i] = ((ThreadInfo*)(p))->thread_request[i];

	Sleep(m_delay);	

	//ѭ�����������Ʒ������
	for(i =0;i<m_requestNum;i++){   
		
	  //����������һ����Ʒ
	  printf("Consumer %2d request to consume %2d product\n",m_serial,m_thread_request[i]);

	  //�����Ӧ������û����������ȴ������������,��������������Ŀ-1��ʵ���˶�дͬ����
	  wait_for_semaphore=WaitForSingleObject(h_Semaphore[m_thread_request[i]],-1); 

	  //��ѯ�����Ʒ�ŵ��������ĺ�		
	  int BufferPos=FindBufferPosition(m_thread_request[i]);	  
			
	  //��ʼ���о��建���������Ѵ��������Ͷ��ڸû���������Ȼ�ǻ���ģ�
	  //�����ٽ�����ִ�����Ѷ�����������ɴ˴������֪ͨ����������߱���������
	  //�����㣻ͬʱ�����Ӧ�Ĳ�Ʒʹ����ϣ�������Ӧ��������������Ӧ�����Ľ�����
	  //ʾ������Ӧ����ָ����Ӧ��������գ������Ӵ����ջ��������ź�����
	  EnterCriticalSection(&PC_Critical[BufferPos]);
	  printf("Consumer %2d begin to consume %2d product \n",m_serial,m_thread_request[i]);
 		  ((ThreadInfo*)(p))->thread_request[i] =-1;

	  if(!IfInOtherRequest(m_thread_request[i])) {
		Buffer_Critical[BufferPos] = -1;		//-1��ǻ�����Ϊ�գ�
		printf("Consumer %2d finish consuming %2d:\n ",m_serial,m_thread_request[i]);
		printf("	 position[ %2d ]:%3d \n\n" ,BufferPos,Buffer_Critical[BufferPos]);
		ReleaseSemaphore(empty_semaphore,1,NULL);
	  }		
	  else {
		printf("Consumer %2d finish consuming product %2d\n\n ",m_serial,m_thread_request[i]);
	  }

	  //�뿪�ٽ���
	  LeaveCriticalSection(&PC_Critical[BufferPos]);
	}
}