
#ifndef REDIS_DB_TOOL_H
#define REDIS_DB_TOOL_H

#include "../hiredis.h"
#include <string>
#include <string.h>
#include <stdio.h>
#include <iostream>

#include <vector>
#include <map>

using namespace std;

#define REDISAPI_OK						1
#define REDISAPI_CONNENV_ERR			    -2 /* redis连接环境错误 */
#define REDISAPI_NOT_RUNNING				-3 /* redis服务未启动 */
#define REDISAPI_NOT_AUTH				    -4 /* redis授权访问失败 */
#define REDISAPI_DATA_ERR			        -5 /* redis数据存取错误 */
#define REDISAPI_SYS_ERR			        -101 /* redis系统错误 */

namespace ZRY {


class CZRYRedisDBTool {
public:

	CZRYRedisDBTool() {
		m_strDebugPre = " "; 
		m_redisconn = NULL;
		m_reply = NULL;
	}

	static const char* getClassName() { return "CZRYRedisDBTool"; }

	/** Connect to the redis(Read infomation from the configure file.).
	* @param        ip        Redis server IP.
	* @param        port      Redis server port.
	* @param        passwd    Redis connect password.
	* @result       int       
	*/
	int ConnectServer();

	int ConnectServer(const string & ip, int port, const string & pwd) {
		setAccount(ip, port, pwd);

		return 0;
	}

	void setAccount(const string & ip, int port, const string & pwd);

	/** Disconnect the redis.
	* @param        conn      Redis connection context.
	* @result       void      
	*/
	void DisconnectServer();

	int SetRedisValueCmd(const char* key, const char* value);

	int SetRedisValueCmd(const char* key, const int value);

	int SetRedisValueCmd(const char* key, const float value);
	int SetRedisValueCmd(const std::string & key, const std::string & value);


	
	void GetRedisValueCmd(const char* key, string& value);
	void GetRedisValueCmd(const char* key, int& value);
	void GetRedisValueCmd(const char* key, float& value);
	void GetRedisValueCmd(const std::string & key, string& value);

	int SetHashValue(const std::string & key, const std::string & field, const std::string & value);
	int SetHashValue(const std::string & key, const std::string & fieldsAndValues);
	int SetHashValue(const std::string & key, const std::map<std::string, std::string> &m);
	void GetHashValue(const std::string & key, const std::string & field, std::string & value);
	void GetHashValueAll(const std::string & key, std::vector<string> & values);
	int GetHashLen(const std::string & key);
private:
	string m_ip;
	string m_pwd;
	int m_port;
	// log
	string m_strDebugPre;

protected:
	redisContext* m_redisconn;
	redisReply *m_reply;
	bool m_result = true;

	bool isError();
	/** Free the redisReply.
	* @param        none
	* @result       void
	*/
	void freeReply();
};

} // namespace ZRY

#endif // REDIS_DB_TOOL_H