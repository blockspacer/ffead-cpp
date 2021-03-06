/*
 * Connection.h
 *
 *  Created on: 08-May-2014
 *      Author: sumeetc
 */

#ifndef CONNECTION_H_
#define CONNECTION_H_
#include "string"
#include "map"
#include "vector"


class ConnectionNode {
public:
	float getConnectionTimeout() const;
	void setConnectionTimeout(const float& connectionTimeout);
	const std::string& getHost() const;
	void setHost(const std::string& host);
	const std::string& getPassword() const;
	void setPassword(const std::string& password);
	int getPort() const;
	void setPort(const int& port);
	float getReadTimeout() const;
	void setReadTimeout(const float& readTimeout);
	const std::string& getUsername() const;
	void setUsername(const std::string& username);
	ConnectionNode();
	virtual ~ConnectionNode();
	const std::string& getDatabaseName() const;
	void setDatabaseName(const std::string& databaseName);
	const std::string& getDsn() const;
	void setBaseUrl(const std::string& url);
	const std::string& getBaseUrl() const;

private:
	std::string url;
	std::string dsn;
	std::string host;
	std::string username;
	std::string password;
	std::string databaseName;
	int port;
	float readTimeout;
	float connectionTimeout;
	friend class ConnectionPooler;
	friend class ConfigurationHandler;
};

class Connection
{
	bool busy;
	bool type;
	void* _conn;
	ConnectionNode node;
public:
	Connection();
	virtual ~Connection();
	void* getConn();
	void setConn(void* conn);
	bool isBusy() const;
	void setBusy(const bool& busy);
	bool isType() const;
	void setType(const bool& type);
	const ConnectionNode& getNode() const;
	void setNode(const ConnectionNode& node);
};

class ConnectionProperties {
	static std::string BLANK;
public:
	bool isNewConnectionStrategy() const;
	void setNewConnectionStrategy(const bool& newConnectionStrategy);
	const std::vector<ConnectionNode>& getNodes() const;
	void setNodes(const std::vector<ConnectionNode>& nodes);
	void addNode(const ConnectionNode& node);
	int getPoolReadSize() const;
	void setPoolReadSize(const int& poolReadSize);
	int getPoolWriteSize() const;
	void setPoolWriteSize(const int& poolWriteSize);
	const std::map<std::string, std::string>& getProperties() const;
	const std::string& getProperty(const std::string& name) const;
	void setProperties(const std::map<std::string, std::string>& properties);
	void addProperty(const std::string& name, const std::string& value);
	const std::string& getType() const;
	void setType(const std::string& type);
	ConnectionProperties();
	virtual ~ConnectionProperties();
	const std::string& getName() const;
	void setName(const std::string& name);

private:
	std::string name;
	std::string type;
	int poolReadSize;
	int poolWriteSize;
	bool newConnectionStrategy;
	std::vector<ConnectionNode> nodes;
	std::map<std::string, std::string> properties;
	friend class ConfigurationHandler;
};

#endif /* CONNECTION_H_ */
