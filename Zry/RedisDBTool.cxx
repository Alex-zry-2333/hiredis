
#include <stdio.h> 
#include <stdlib.h> 
#include <stddef.h> 
#include <stdarg.h> 
#include <string.h> 
#include <assert.h> 
#include <string>
#include <cstdint>
#include <inttypes.h>

#include <fstream>


#include "log4ZRY.h"
#include "RedisDBTool.hxx"

using namespace std;


bool CZRYRedisDBTool::isError()
{
	if (NULL == m_reply)
	{
		DisconnectServer();
		ConnectServer();
		return true;
	}
	return false;
}

/** Free the redisReply.
* @param        none
* @result       void
*/
void CZRYRedisDBTool::freeReply() {
	if (m_reply && m_reply->type != 0) {
		freeReplyObject(m_reply);
		m_reply = NULL;
	}
}

/** Connect to the redis(Read infomation from the configure file.).
* @param        ip        Redis server IP.
* @param        port      Redis server port.
* @param        passwd    Redis ConnectServer password.
* @result       int      
*/
int CZRYRedisDBTool::ConnectServer() {
	int iReturn = REDISAPI_OK;

	if (NULL != m_redisconn) {
		return iReturn;
	}

	try {

		// Try to ConnectServer to the redis.
		//m_redisconn = redisConnect(m_ip.c_str(), m_port);
		struct timeval timeout = { 1, 500000 }; // 1.5 seconds
		m_redisconn = redisConnectWithTimeout(m_ip.c_str(), m_port, timeout);
		if (m_redisconn == NULL || m_redisconn->err)
		{
			redisFree(m_redisconn);
			if (DEBUG_REDISDB_TOOLS) {
				CLog4ZRY::ZRY_cerr_debug(true, m_strDebugPre.c_str(), "CZRYRedisDBTool::ConnectServer() Connect to redisServer failed: %s\n", m_redisconn->errstr);
			}
			iReturn = REDISAPI_NOT_RUNNING;
			return iReturn;
		}


		m_reply = (redisReply *)redisCommand(m_redisconn, "AUTH %s", m_pwd.c_str());
		if (m_reply->type == REDIS_REPLY_ERROR)
		{
			iReturn = REDISAPI_NOT_AUTH;
			if (DEBUG_REDISDB_TOOLS) {
				CLog4ZRY::ZRY_cerr_debug(true, m_strDebugPre.c_str(), "CZRYRedisDBTool::ConnectServer() Authenticate failed!\n");
			}
		}
		else
		{
			iReturn = REDISAPI_OK;
			if (DEBUG_REDISDB_TOOLS) {
				CLog4ZRY::ZRY_cerr_debug(true, m_strDebugPre.c_str(), "CZRYRedisDBTool::ConnectServer() Authenticate successful!\n");
			}
		}
		freeReplyObject(m_reply);

		/* PING server */
		m_reply = (redisReply *)redisCommand(m_redisconn, "PING");
		if (m_reply->type == REDIS_REPLY_ERROR)
		{
			iReturn = REDISAPI_NOT_RUNNING;
			if (DEBUG_REDISDB_TOOLS) {
				CLog4ZRY::ZRY_cerr_debug(true, m_strDebugPre.c_str(), "CZRYRedisDBTool::ConnectServer() Ping failed!\n");
			}
		}
		else
		{
			if (DEBUG_REDISDB_TOOLS) {
				CLog4ZRY::ZRY_cerr_debug(true, m_strDebugPre.c_str(), "CZRYRedisDBTool::ConnectServer() Ping successful!\n");
			}

			iReturn = REDISAPI_OK;
		}
		freeReplyObject(m_reply);
	}
	catch (std::exception &ex) {
		if (DEBUG_REDISDB_TOOLS) {
			CLog4ZRY::ZRY_cerr_debug(true, m_strDebugPre.c_str(), "CZRYRedisDBTool::ConnectServer() Connect to redisServer failed! Exception:%s\n", ex.what());
		}
		iReturn = REDISAPI_SYS_ERR;
	}

	return iReturn;
}


void CZRYRedisDBTool::setAccount(const string & ip, int port, const string & pwd)
{
	m_ip = ip;
	m_port = port;
	m_pwd = pwd;
}


/** Disconnect the redis.
* @param        conn      Redis connection context.
* @result       void
*/
void CZRYRedisDBTool::DisconnectServer() {
	if (m_redisconn != NULL) {
		redisFree(m_redisconn);

		m_redisconn = NULL;
	}
}

int CZRYRedisDBTool::SetRedisValueCmd(const char* key, const char* value) {
	int iReturn = REDISAPI_OK;

	iReturn = ConnectServer();

	if (REDISAPI_OK != iReturn) {
		return iReturn;
	}

	m_reply = (redisReply*)redisCommand(m_redisconn, "SET %s %s", key, value);
	if (!isError())
	{
		if (!(m_reply->type == REDIS_REPLY_STATUS && strcasecmp(m_reply->str, "OK") == 0))
		{
			//cout << "Failed to execute SET(string)" << endl;
			if (DEBUG_REDISDB_TOOLS) {
				CLog4ZRY::ZRY_cerr_debug(true, m_strDebugPre.c_str(), "CZRYRedisDBTool::SetRedisValueCmd() Failed to execute: %s\n", m_reply->str);
			}
			iReturn = REDISAPI_DATA_ERR;
		}
	}
	freeReply();

	DisconnectServer();

	return iReturn;
}

int CZRYRedisDBTool::SetRedisValueCmd(const char* key, int value) {
	int iReturn = REDISAPI_OK;

	iReturn = ConnectServer();

	if (REDISAPI_OK != iReturn) {
		return iReturn;
	}

	m_reply = (redisReply*)redisCommand(m_redisconn, "SET %s %d", key, value);
	if (!isError())
	{
		if (!(m_reply->type == REDIS_REPLY_STATUS && strcasecmp(m_reply->str, "OK") == 0))
		{
			//cout << "Failed to execute SET(int)" << endl;
			if (DEBUG_REDISDB_TOOLS) {
				CLog4ZRY::ZRY_cerr_debug(true, m_strDebugPre.c_str(), "CZRYRedisDBTool::SetRedisValueCmd() Failed to execute: %s\n", m_reply->str);
			}
			iReturn = REDISAPI_DATA_ERR;
		}
	}
	freeReply();

	DisconnectServer();

	return iReturn;
}

int CZRYRedisDBTool::SetRedisValueCmd(const char* key, float value) {
	int iReturn = REDISAPI_OK;

	iReturn = ConnectServer();

	if (REDISAPI_OK != iReturn) {
		return iReturn;
	}

	m_reply = (redisReply*)redisCommand(m_redisconn, "SET %s %f", key, value);
	if (!isError())
	{
		if (!(m_reply->type == REDIS_REPLY_STATUS && strcasecmp(m_reply->str, "OK") == 0))
		{
			//cout << "Failed to execute SET(float)" << endl;
			if (DEBUG_REDISDB_TOOLS) {
				CLog4ZRY::ZRY_cerr_debug(true, m_strDebugPre.c_str(), "CZRYRedisDBTool::SetRedisValueCmd() Failed to execute: %s\n", m_reply->str);
			}
			iReturn = REDISAPI_DATA_ERR;
		}
	}
	freeReply();
	DisconnectServer();

	return iReturn;
}

int CZRYRedisDBTool::SetRedisValueCmd(const std::string & key, const std::string & value)
{
	int iReturn = REDISAPI_OK;
#if 1
	iReturn = ConnectServer();

	if (REDISAPI_OK != iReturn) {
		return iReturn;
	}

	m_reply = (redisReply*)redisCommand(m_redisconn, "SET %s %s", key.c_str(), value.c_str());
	if (!isError())
	{
		if (!(m_reply->type == REDIS_REPLY_STATUS && strcasecmp(m_reply->str, "OK") == 0))
		{
			//cout << "Failed to execute SET(float)" << endl;
			if (DEBUG_REDISDB_TOOLS) {
				CLog4ZRY::ZRY_cerr_debug(true, m_strDebugPre.c_str(), "CZRYRedisDBTool::SetRedisValueCmd() Failed to execute: %s\n", m_reply->str);
			}
			iReturn = REDISAPI_DATA_ERR;
		}
	}
	freeReply();

	DisconnectServer();
#else
	redisContext *c;
	redisReply *reply;

	struct timeval timeout = { 1, 500000 }; // 1.5 seconds
	c = redisConnectWithTimeout("172.16.2.54", 6379, timeout);

	/* AUTH a key */
	reply = (redisReply *)redisCommand(c, "AUTH 123456");
	printf("AUTH: %s\n", reply->str);
	freeReplyObject(reply);

	/* PING server */
	reply = (redisReply *)redisCommand(c, "PING");
	printf("PING: %s\n", reply->str);
	freeReplyObject(reply);	

	reply = (redisReply*)redisCommand(c, "SET %s %s", key.c_str(), value.c_str());
	printf("SET: %s\n", reply->str);
	freeReplyObject(reply);

	redisFree(c);
#endif

	return iReturn;
}

void CZRYRedisDBTool::GetRedisValueCmd(const char* key, string& value) {

	int iReturn = ConnectServer();

	if (REDISAPI_OK != iReturn) {
		return ;
	}

	m_reply = (redisReply*)redisCommand(m_redisconn, "GET %s", key);
	if (!isError() && m_reply->type == REDIS_REPLY_STRING)
	{
		value = std::string(m_reply->str);
	}
	freeReply();
	DisconnectServer();
}

void CZRYRedisDBTool::GetRedisValueCmd(const char* key, int& value) {

	int iReturn = ConnectServer();

	if (REDISAPI_OK != iReturn) {
		return;
	}

	m_reply = (redisReply*)redisCommand(m_redisconn, "GET %s", key);
	if (!isError() && m_reply->type == REDIS_REPLY_STRING)
	{
		value = atoi(m_reply->str);
	}
	freeReply();
	DisconnectServer();
}

void CZRYRedisDBTool::GetRedisValueCmd(const char* key, float& value) {

	int iReturn = ConnectServer();

	if (REDISAPI_OK != iReturn) {
		return;
	}

	m_reply = (redisReply*)redisCommand(m_redisconn, "GET %s", key);
	if (!isError() && m_reply->type == REDIS_REPLY_STRING)
	{
		value = atof(m_reply->str);
	}
	freeReply();

	DisconnectServer();
}

void CZRYRedisDBTool::GetRedisValueCmd(const std::string & key, string & value)
{

	int iReturn = ConnectServer();

	if (REDISAPI_OK != iReturn) {
		return;
	}

	m_reply = (redisReply*)redisCommand(m_redisconn, "GET %s", key.c_str());
	if (!isError() && m_reply->type == REDIS_REPLY_STRING)
	{
		value = std::string(m_reply->str);
	}
	freeReply();

	DisconnectServer();
}

int CZRYRedisDBTool::SetHashValue(const std::string & key, const std::string & field, const std::string & value)
{
	int iReturn = REDISAPI_OK;


	iReturn = ConnectServer();

	if (REDISAPI_OK != iReturn) {
		return iReturn;
	}

	m_reply = (redisReply*)redisCommand(m_redisconn, "HSET %s %s %s", key.c_str(), field.c_str(), value.c_str());
	if (!isError())
	{
		if (!(m_reply->type == REDIS_REPLY_STATUS && strcasecmp(m_reply->str, "OK") == 0))
		{
			if (DEBUG_REDISDB_TOOLS) {
				CLog4ZRY::ZRY_cerr_debug(true, m_strDebugPre.c_str(), "CZRYRedisDBTool::SetHashValue() Failed to execute: %s\n", m_reply->str);
			}
			iReturn = REDISAPI_DATA_ERR;
		}
	}
	freeReply();

	DisconnectServer();

	return iReturn;
}

int CZRYRedisDBTool::SetHashValue(const std::string & key, const std::string & fieldsAndValues)
{
	int iReturn = REDISAPI_OK;

	iReturn = ConnectServer();

	if (REDISAPI_OK != iReturn) {
		return iReturn;
	}


	string sCmd = "HMSET ";
	sCmd.append(key);
	sCmd.append(" ");
	sCmd.append(fieldsAndValues);

	if (DEBUG_REDISDB_TOOLS) {
		CLog4ZRY::ZRY_cerr_debug(true, m_strDebugPre.c_str(), "CZRYRedisDBTool::SetHashValue() cmd:[%s]\n", sCmd.c_str());
	}
	iReturn = REDISAPI_DATA_ERR;
	m_reply = (redisReply*)redisCommand(m_redisconn, "HMSET %s %s", key.c_str(), fieldsAndValues.c_str());
	if (!isError())
	{
		if (!(m_reply->type == REDIS_REPLY_STATUS && strcasecmp(m_reply->str, "OK") == 0))
		{
			//cout << "Failed to execute SET(int)" << endl;
			if (DEBUG_REDISDB_TOOLS) {
				CLog4ZRY::ZRY_cerr_debug(true, m_strDebugPre.c_str(), "CZRYRedisDBTool::SetHashValue() Failed to execute: %s\n", m_reply->str);
			}
			iReturn = REDISAPI_DATA_ERR;
		}
	}
	freeReply();

	DisconnectServer();

	return iReturn;
}

int CZRYRedisDBTool::SetHashValue(const std::string & key, const std::map<std::string, std::string> &m)
{
	int iReturn = REDISAPI_OK;
	vector<const char *> argv;
	vector<size_t> argvlen;

	iReturn = ConnectServer();

	if (REDISAPI_OK != iReturn) {
		return iReturn;
	}


	static char cmd[] = "HMSET";
	argv.push_back(cmd);
	argvlen.push_back(sizeof(cmd) - 1);

	argv.push_back(key.c_str());
	argvlen.push_back(key.size());

	map<string, string>::const_iterator i;
	for (i = m.begin(); i != m.end(); ++i)
	{
		argv.push_back(i->first.c_str());
		argvlen.push_back(i->first.size());
		argv.push_back(i->second.c_str());
		argvlen.push_back(i->second.size());
	}

	m_reply = (redisReply*)redisCommandArgv(m_redisconn, argv.size(), &(argv[0]), &(argvlen[0]));
	if (!isError())
	{
		if (!(m_reply->type == REDIS_REPLY_STATUS && strcasecmp(m_reply->str, "OK") == 0))
		{
			//cout << "Failed to execute SET(int)" << endl;
			if (DEBUG_REDISDB_TOOLS) {
				CLog4ZRY::ZRY_cerr_debug(true, m_strDebugPre.c_str(), "CZRYRedisDBTool::SetHashValue() Failed to execute: %s\n", m_reply->str);
			}
			iReturn = REDISAPI_DATA_ERR;
		}
	}
	freeReply();

	DisconnectServer();

	return iReturn;
}

void CZRYRedisDBTool::GetHashValue(const std::string & key, const std::string & field, std::string & value)
{

	int iReturn = ConnectServer();

	if (REDISAPI_OK != iReturn) {
		return;
	}


	m_reply = (redisReply*)redisCommand(m_redisconn, "HGET %s %s", key.c_str(), field.c_str());
	if (!isError() && m_reply->type == REDIS_REPLY_STRING)
	{
		value = std::string(m_reply->str);
	}
	freeReply();

	DisconnectServer();
}

void CZRYRedisDBTool::GetHashValueAll(const std::string & key, std::vector<std::string> & values)
{

	int iReturn = ConnectServer();

	if (REDISAPI_OK != iReturn) {
		return;
	}

	m_reply = (redisReply*)redisCommand(m_redisconn, "HGETALL %s", key.c_str());
	if (!isError() && m_reply->type == REDIS_REPLY_ARRAY)
	{
		for (int i = 0; i < m_reply->elements; i++) {
			values.push_back(std::string(m_reply->element[i]->str));
		}
	}
	freeReply();

	DisconnectServer();
}

int CZRYRedisDBTool::GetHashLen(const std::string & key)
{

	int iReturn = ConnectServer();

	if (REDISAPI_OK != iReturn) {
		return 0;
	}

	int result = 0;
	m_reply = (redisReply*)redisCommand(m_redisconn, "HSET %s", key.c_str());
	if (!isError() && m_reply->type == REDIS_REPLY_STRING)
	{
		result = atoi(m_reply->str);
	}
	freeReply();

	DisconnectServer();
}


#if 0
int main(int argc, char *argv[]) {
	int iReturn = 0;

	CZRYRedisDBTool* redis = new CZRYRedisDBTool();

	// redis连接
	if (redis->ConnectServer() != REDISAPI_OK) {
		iReturn = -1;
		printf("The redis connection failed!\n");
		return iReturn;
	}

	if (argc > 2) {
		redis->SetRedisValueCmd(argv[1], argv[2]);
	}
	else {
		printf("The input parameter is error!\n");
	}

	// redis 断开连接
	redis->DisconnectServer();

	return iReturn;
} 
#endif
