/*
 * DataSourceManager.cpp
 *
 *  Created on: 14-May-2014
 *      Author: sumeetc
 */

#include "DataSourceManager.h"

std::map<std::string, DataSourceManager*> DataSourceManager::dsns;
std::map<std::string, std::string> DataSourceManager::defDsnNames;

void DataSourceManager::initDSN(const ConnectionProperties& props, const Mapping& mapping)
{
	Logger logger = LoggerFactory::getLogger("DataSourceManager");
	std::string name = StringUtil::trimCopy(props.getName());
	if(name=="")
	{
		throw std::runtime_error("Data Source Name cannot be blank");
	}
	std::string appName = mapping.getAppName();
	StringUtil::replaceAll(appName, "-", "_");
	RegexUtil::replace(appName, "[^a-zA-Z0-9_]+", "");
	name = appName + name;
	if(dsns.find(name)!=dsns.end())
	{
		throw std::runtime_error("Data Source Already exists");
	}
	if(props.getProperty("_isdefault_")=="true") {
		defDsnNames[appName] = StringUtil::trimCopy(props.getName());
	}

	try {
		DataSourceManager* dsnMgr = new DataSourceManager(props, mapping);
		dsns[name] = dsnMgr;
	} catch (const std::exception& e) {
		logger.info("Error initializing Datasource " + mapping.getAppName() + "@" + props.getName() + " " + std::string(e.what()));
		return;
	}

	Reflector* ref = GenericObject::getReflector();
	if(props.getProperty("init")!="") {
		std::string meth = props.getProperty("init");
		std::vector<std::string> v;
		StringUtil::split(v, meth, ".");
		if(v.size()==2) {
			CommonUtils::setAppName(appName);
			ClassInfo* clas = ref->getClassInfo(v.at(0), appName);
			if(clas->getClassName()!="") {
				args argus;
				vals valus;
				const Constructor& ctor = clas->getConstructor(argus);
				void* _temp = ref->newInstanceGVP(ctor);
				try {
					if(_temp!=NULL) {
						const Method& meth = clas->getMethod(v.at(1), argus);
						if(meth.getMethodName()!="")
						{
							ref->invokeMethodGVP(_temp, meth, valus);
						}
					}
				} catch(const std::exception& e) {
					logger.info("Error during init call for Datasource " + mapping.getAppName() + "@" + props.getName() + " " + std::string(e.what()));
				}
				ref->destroy(_temp, v.at(0), appName);
			}
		}
	}
}

void DataSourceManager::destroy()
{
	std::map<std::string, DataSourceManager*>::iterator it;
	for(it=dsns.begin();it!=dsns.end();++it)
	{
		if(it->second!=NULL)
		{
			delete it->second;
		}
	}
	dsns.clear();
}

DataSourceManager::DataSourceManager(const ConnectionProperties& props, const Mapping& mapping) {
	logger = LoggerFactory::getLogger("DataSourceManager");
	this->pool = NULL;
	this->props = props;
	this->mapping = mapping;
	if(StringUtil::toLowerCopy(props.getType()) == "sql") {
#ifdef INC_SDORM_SQL
		this->pool = new SQLConnectionPool(props);
#endif
	} else if(StringUtil::toLowerCopy(props.getType()) == "mongo") {
#ifdef INC_SDORM_MONGO
		this->pool = new MongoDBConnectionPool(props);
#endif
	}
}

DataSourceManager::~DataSourceManager() {
	if(this->pool!=NULL) {
		delete this->pool;
	}
}

DataSourceInterface* DataSourceManager::getImpl(std::string name) {
	std::string appName = CommonUtils::getAppName();
	StringUtil::trim(name);
	if(name=="") {
		name = defDsnNames[appName];
	}
	name = appName + name;
	if(dsns.find(name)==dsns.end())
	{
		throw std::runtime_error("Data Source Not found...");
	}
	DataSourceManager* dsnMgr = dsns[name];
	DataSourceInterface* t = NULL;
	if(StringUtil::toLowerCopy(dsnMgr->props.getType())=="sql")
	{
#ifdef INC_SDORM_SQL
		t = new SQLDataSourceImpl(dsnMgr->pool, &dsnMgr->mapping);
#endif
	}
	else if(StringUtil::toLowerCopy(dsnMgr->props.getType())=="mongo")
	{
#ifdef INC_SDORM_MONGO
		t = new MongoDBDataSourceImpl(dsnMgr->pool, &dsnMgr->mapping);
#endif
	}
	if(t == NULL)
	{
		return NULL;
	}
	t->appName = dsnMgr->mapping.getAppName();
	t->reflector = GenericObject::getReflector();
	std::map<std::string, DataSourceEntityMapping>::iterator it;
	for(it=dsnMgr->mapping.getDseMap().begin();it!=dsnMgr->mapping.getDseMap().end();++it)
	{
		DataSourceEntityMapping dsemp = it->second;
		if(dsemp.isIdGenerate())
		{
			t->init(dsemp, true);
		}
	}
	return t;
}
