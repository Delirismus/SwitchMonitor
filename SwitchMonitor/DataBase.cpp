#include "stdafx.h"
#include "DataBase.h"


sqlite3 *OpenDataBase()
{
	BOOL bRet = FALSE;
	sqlite3 *db = NULL;
	char* errmsg;
	char **dbResult;
	int nRow, nColumn;
	int nRet = sqlite3_open("BeQuick.db",&db);
	if (nRet == SQLITE_OK)
	{
		OutputDebugString(TEXT("DataBasse open successfully!\n"));
		nRet = sqlite3_exec(db,"CREATE TABLE QuickKey(Id integer primary key autoincrement,UserDefine integer,KeyName varchar(32),KeyValue integer,SubKey varchar(32));",NULL,NULL,&errmsg); 
		if (nRet == SQLITE_OK)
		{
			OutputDebugString(TEXT("Create table successfully!\n"));
		}
		else
		{
			OutputDebugString(TEXT("Create table failed!\n"));
			sqlite3_get_table(db, "SELECT * FROM sqlite_master where type='table' and name='QuickKey'",&dbResult,&nRow,&nColumn,NULL);
			if(nRow > 0 )
			{
				OutputDebugString(TEXT("�˱��Ѿ�����!\n"));
				// ��ѯ���ݱ�
			}
			sqlite3_free_table(dbResult);
		}
		return db;
	}
	OutputDebugString(TEXT("DataBasse open failed!\n"));
	return NULL;
}
void CloseDataBase(sqlite3 *db)
{
	sqlite3_close(db);
}
int callback(void* ,int nCount,char** pValue,char** pName)
{
	for (int i = 1;i <= nCount;i++)
	{
		OutputDebugStringA(pName[i]);
		OutputDebugStringA(" : ");
		OutputDebugStringA(pValue[i]);
		OutputDebugStringA("    ");
	}
	OutputDebugStringA("\n");
	return 0;
}
BOOL GetMaxCountDataBase(sqlite3 *db,int *nMaxCount)
{	
	int nRet = 0;
	char **dbResult;
	int nRow, nColumn;
	sqlite3_get_table(db,"select * from QuickKey",&dbResult,&nRow,&nColumn,NULL);
	if(nRow > 0 )
	{
		*nMaxCount = nRow;
		sqlite3_free_table(dbResult);
		return TRUE;
	}
	sqlite3_free_table(dbResult);
	nMaxCount = 0;
	return FALSE;
}
BOOL GetKeyToDataBase(sqlite3 *db,PQUICK_KEY_LIST pQuickList)
{
	char* errmsg;
	char **dbResult;
	int nRow, nColumn;
	int nRet = 0;
	int nNum = 1;
	nRet = sqlite3_get_table(db,"select * from QuickKey",&dbResult,&nRow,&nColumn,&errmsg);
	if(nRet == SQLITE_OK)
	{
		//dbResult1 ǰ���һ���������ֶ����ƣ��� nColumn ������ʼ��������������
		//dbResult1 ���ֶ�ֵ�������ģ��ӵ�0�������� nColumn - 1���������ֶ����ƣ��ӵ� nColumn ������ʼ��
		//���涼���ֶ�ֵ������һ����ά�ı���ͳ�����б�ʾ������һ����ƽ����ʽ����ʾ
		int index = nColumn;
		for(int i = 1; i <= nRow ; i++)
		{
			for(int j = 0 ; j < nColumn; j++)
			{
				OutputDebugStringA(dbResult[j]);
				OutputDebugStringA("    ");
			}
			OutputDebugStringA("\n");
		}
		//
		for(int i = 1; i <= nRow ; i++)
		{
			for(int j = nColumn; j < (nRow * nColumn + nColumn); j++)
			{
				OutputDebugStringA(dbResult[j]);
				OutputDebugStringA("    ");
			}
			OutputDebugStringA("\n");
		}
		for(int i = 0; i < nRow ; i++)
		{
			pQuickList->QuickKey[i].nId = atoi(dbResult[nNum * nColumn + 0]);

			pQuickList->QuickKey[i].nUserDefine = atoi(dbResult[nNum * nColumn + 1]);

			StringCchCopyA(pQuickList->QuickKey[i].KeyName,32,dbResult[nNum * nColumn + 2]);

			pQuickList->QuickKey[i].nKeyValue = atoi(dbResult[nNum * nColumn + 3]);

			pQuickList->QuickKey[i].SubKeyValue = *dbResult[nNum * nColumn + 4];

			pQuickList->nCurCount++;

			nNum++;
		}
		return TRUE;
	}
	sqlite3_free_table(dbResult);
	//�������ݿ��ѯ�Ƿ�ɹ������ͷ� char** ��ѯ�����ʹ�� sqlite �ṩ�Ĺ������ͷ�
	return FALSE;
}
BOOL DeleteKeyToDataBase(sqlite3 *db,int nID)
{
	char* errmsg;
	int nRet = 0;
	char *szFormatSql = NULL;
	szFormatSql = (char*)malloc(MAX_PATH);
	if (szFormatSql == NULL)
	{
		OutputDebugString(TEXT("DeleteKeyToDataBase --> malloc failed!\n"));
		return FALSE;
	}
	RtlZeroMemory(szFormatSql,MAX_PATH);
	//"CREATE TABLE QuickKey(Id integer primary key autoincrement,UserDefine integer,KeyName varchar(32),KeyValue integer,SubKey varchar(32));"
	StringCchPrintfA(szFormatSql,MAX_PATH,"DELTE from QuickKey WHERE Id = %d;",nID);
	nRet = sqlite3_exec(db,szFormatSql,NULL,NULL,&errmsg);
	if (nRet == SQLITE_OK)
	{
		if (szFormatSql)
		{
			free(szFormatSql);
			szFormatSql = NULL;
		}
		sqlite3_free(errmsg);
		OutputDebugString(TEXT("Delete key successfully!\n"));
		return TRUE;
	}
	sqlite3_free(errmsg);
	if (szFormatSql)
	{
		free(szFormatSql);
		szFormatSql = NULL;
	}
	return FALSE;
}
BOOL UpdateKeyToDataBase(sqlite3 *db,int nID,int nUserDefine,char* szKeyName,int nKeyValue,char szSubKey)
{
	char* errmsg;
	int nRet = 0;
	char *szFormatSql = NULL;
	szFormatSql = (char*)malloc(MAX_PATH);
	if (szFormatSql == NULL)
	{
		OutputDebugString(TEXT("UpdateKeyToDataBase --> malloc failed!\n"));
		return FALSE;
	}
	RtlZeroMemory(szFormatSql,MAX_PATH);
	//"CREATE TABLE QuickKey(Id integer primary key autoincrement,UserDefine integer,KeyName varchar(32),KeyValue integer,SubKey varchar(32));"
	StringCchPrintfA(szFormatSql,MAX_PATH,"UPDATE QuickKey SET VALUES(%d,\'%s\',%d,\'%c\') WHERE Id = %d;",nUserDefine,szKeyName,nKeyValue,szSubKey,nID);
	nRet = sqlite3_exec(db,szFormatSql,NULL,NULL,&errmsg);
	if (nRet == SQLITE_OK)
	{
		if (szFormatSql)
		{
			free(szFormatSql);
			szFormatSql = NULL;
		}
		sqlite3_free(errmsg);
		OutputDebugString(TEXT("Update key successfully!\n"));
		return TRUE;
	}
	sqlite3_free(errmsg);
	if (szFormatSql)
	{
		free(szFormatSql);
		szFormatSql = NULL;
	}
	return FALSE;
}
BOOL SetKeyToDataBase(sqlite3 *db,int nUserDefine,char* szKeyName,int nKeyValue,char szSubKey)
{
	char* errmsg;
	int nRet = 0;
	char *szFormatSql = NULL;
	szFormatSql = (char*)malloc(MAX_PATH);
	if (szFormatSql == NULL)
	{
		OutputDebugString(TEXT("SetKeyToDataBase --> malloc failed!\n"));
		return FALSE;
	}
	RtlZeroMemory(szFormatSql,MAX_PATH);
	StringCchPrintfA(szFormatSql,MAX_PATH,"INSERT INTO QuickKey VALUES(1,%d,\'%s\',%d,\'%c\');",nUserDefine,szKeyName,nKeyValue,szSubKey);
	nRet = sqlite3_exec(db,szFormatSql,NULL,NULL,&errmsg);
	if (nRet == SQLITE_OK)
	{
		if (szFormatSql)
		{
			free(szFormatSql);
			szFormatSql = NULL;
		}
		sqlite3_free(errmsg);
		OutputDebugString(TEXT("Insert Key successfully!\n"));
		return TRUE;
	}
	sqlite3_free(errmsg);
	if (szFormatSql)
	{
		free(szFormatSql);
		szFormatSql = NULL;
	}
	return FALSE;
}