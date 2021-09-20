#include "eyre_json.h"
#include "general.h"

/*
 * 作者: Eyre Turing (Eyre-Turing)
 * 最后编辑于: 2021/09/20 16:41
 */

Json JsonNone;
Json::Iterator JsonIteratorNone;
JsonArray JsonArrayNone;

Json::Json()
{
	m_parent = NULL;
	m_type = JSON_NONE;
}

void Json::asObject(const Json &json, Json *parent)
{
	m_parent = parent;
	m_type = json.m_type;
	switch (m_type)
	{
	case JSON_NONE:
		break;
	case JSON_BOOLEAN:
		m_bolval = json.m_bolval;
		break;
	case JSON_NUMBER:
		m_numval = json.m_numval;
		break;
	case JSON_STRING:
		m_strval = json.m_strval;
		break;
	case JSON_ARRAY:
	{
		size_t count = json.m_arrval.size();
		for (size_t i = 0; i < count; ++i)
		{
			m_arrval.push_back(new Json(*(json.m_arrval[i]), this));
		}
		break;
	}
	case JSON_OBJECT:
	{
		for (std::map<String, Json *>::const_iterator it = json.m_objval.begin();
			it != json.m_objval.end();
			++it)
		{
			m_objval[it->first] = new Json(*(it->second), this);
		}
		break;
	}
	default:
#ifdef EYRE_DEBUG
		fprintf(stderr, "Json(%p) unknow data type \'%d\'!\n", this, m_type);
#endif
	}
}

Json::Json(const Json &json, Json *parent)
{
	asObject(json, parent);
}

Json::Json(const Json &json)
{
	asObject(json, NULL);
}

Json::~Json()
{
	cleanOldTypeData();
}

Json::Json(bool val) : Json()
{
	asBoolean(val);
}

Json::Json(double val) : Json()
{
	asNumber(val);
}

Json::Json(const String &val) : Json()
{
	asString(val);
}

Json::Json(const JsonArray &val) : Json()
{
	asArray(val);
}

void Json::cleanOldTypeData()
{
	if (m_type == JSON_ARRAY)
	{
		size_t count = m_arrval.size();
		for (size_t i = 0; i < count; ++i)
		{
			delete m_arrval[i];
		}
		m_arrval.clear();
	}
	else if (m_type == JSON_OBJECT)
	{
		for (std::map<String, Json *>::const_iterator it = m_objval.begin();
			it != m_objval.end();
			++it)
		{
			delete it->second;
		}
		m_objval.clear();
	}
}

void Json::asBoolean(bool val)
{
	cleanOldTypeData();
	m_type = JSON_BOOLEAN;
	m_bolval = val;
}

void Json::asNumber(double val)
{
	cleanOldTypeData();
	m_type = JSON_NUMBER;
	m_numval = val;
}

void Json::asString(const String &val)
{
	cleanOldTypeData();
	m_type = JSON_STRING;
	m_strval = val;
}

void Json::asArray()
{
	cleanOldTypeData();
	m_type = JSON_ARRAY;
}

void Json::asArray(const JsonArray &val)
{
	asArray();
	size_t count = val.m_json->m_arrval.size();
	for (size_t i = 0; i < count; ++i)
	{
		m_arrval.push_back(new Json(*(val.m_json->m_arrval[i]), this));
	}
}

void Json::asObject()
{
	cleanOldTypeData();
	m_type = JSON_NONE;		// 因为没有数据了，所以先转换为JSON_NONE状态
}

void Json::asObject(const Json &val)
{
	asObject();
	for (std::map<String, Json *>::const_iterator it = val.m_objval.begin();
		it != val.m_objval.end();
		++it)
	{
		m_type = JSON_OBJECT;	// 添加了数据，变为JSON_OBJECT状态
		m_objval[it->first] = new Json(*(it->second), this);
	}
}

Json::Iterator Json::operator[](const String &key)
{
	if (m_type != JSON_OBJECT && m_type != JSON_NONE)
	{
		return JsonIteratorNone;
	}
	std::map<String, Json *>::iterator it = m_objval.find(key);
	return Json::Iterator(this, it, key);
}

const Json &Json::operator[](const String &key) const
{
	if (m_type != JSON_OBJECT && m_type != JSON_NONE)
	{
#ifdef EYRE_DEBUG
		fprintf(stderr, "Json(%p) isn\'t a object!\n", this);
#endif
		return JsonNone;
	}
	std::map<String, Json *>::const_iterator it = m_objval.find(key);
	if (it == m_objval.end())
	{
		return JsonNone;
	}
	return *(it->second);
}

Json &Json::parent()
{
	if (!m_parent)
	{
		return JsonNone;
	}
	return *m_parent;
}

const Json &Json::parent() const
{
	if (!m_parent)
	{
		return JsonNone;
	}
	return *m_parent;
}

int Json::type() const
{
	return m_type;
}

bool Json::boolean(bool def) const
{
	if (m_type != JSON_BOOLEAN)
	{
		return def;
	}
	return m_bolval;
}

double Json::number(double def) const
{
	if (m_type != JSON_NUMBER)
	{
		return def;
	}
	return m_numval;
}

String Json::string(const String &def) const
{
	if (m_type != JSON_STRING)
	{
		return def;
	}
	return m_strval;
}

JsonArray Json::toArray()
{
	if (m_type != JSON_ARRAY)
	{
		return JsonArrayNone;
	}
	return JsonArray(this);
}

bool Json::keyExist(const String &key) const
{
	if (m_type != JSON_OBJECT)
	{
		return false;
	}
	return (m_objval.find(key) != m_objval.end());
}

int Json::set(const String &key, const Json &val)
{
	if (m_type != JSON_OBJECT && m_type != JSON_NONE)
	{
		return JSON_FAIL;
	}
	int method;
	if (keyExist(key))	// 更新
	{
		delete m_objval[key];	// 把旧的删除
		method = JSON_UPDATE;
	}
	else
	{
		method = JSON_APPEND;
	}
	m_objval[key] = new Json(val);
	if (m_type == JSON_NONE)
	{
		m_type = JSON_OBJECT;
	}
	return method;
}

bool Json::remove(const String &key)
{
	if (m_type != JSON_OBJECT)
	{
		return false;
	}
	std::map<String, Json *>::iterator it = m_objval.find(key);
	if (it == m_objval.end())
	{
		return false;
	}
	m_objval.erase(it);
	if (m_objval.empty())
	{
		m_type = JSON_NONE;		// 没有数据了，状态变更为JSON_NONE
	}
	return true;
}

Json Json::parseFromText(const String &text)
{
	Json json;

	return json;
}

String Json::toString(bool fold) const
{
	String str;

	return str;
}

std::vector<String> Json::keys() const
{
	std::vector<String> result;
	if (m_type != JSON_OBJECT)
	{
		return result;
	}
	for (std::map<String, Json *>::const_iterator it = m_objval.begin();
		it != m_objval.end();
		++it)
	{
		result.push_back(it->first);
	}
	return result;
}

Json &Json::operator=(const Json &json)
{
	asObject(json);
}

Json &Json::operator=(bool val)
{
	asBoolean(val);
}

Json &Json::operator=(double val)
{
	asNumber(val);
}

Json &Json::operator=(const String &val)
{
	asString(val);
}

Json &Json::operator=(const JsonArray &val)
{
	asArray(val);
}

Json::Iterator::Iterator()
{
	m_j = NULL;
}

Json::Iterator::Iterator(Json *json, std::map<String, Json *>::iterator &it, const String &key)
{
	m_j = json;
	m_it = it;
	m_key = key;
}

Json::Iterator::operator Json &()
{
	if (m_it == m_j->m_objval.end())
	{
		return JsonNone;
	}
	return *(m_it->second);
}

Json::Iterator &Json::Iterator::operator=(const Json &json)
{
	m_j->set(m_key, json);
	return *this;
}

Json::Iterator &Json::Iterator::operator=(bool val)
{
	m_j->set(m_key, Json(val));
	return *this;
}

Json::Iterator &Json::Iterator::operator=(double val)
{
	m_j->set(m_key, Json(val));
	return *this;
}

Json::Iterator &Json::Iterator::operator=(const String &val)
{
	m_j->set(m_key, Json(val));
	return *this;
}

Json::Iterator &Json::Iterator::operator=(const JsonArray &val)
{
	m_j->set(m_key, Json(val));
	return *this;
}

static std::map<char, String> genEscapeMethod()
{
	std::map<char, String> result;
	result['\n'] = "\\n";
	result['\t'] = "\\t";
	result['\"'] = "\\\"";
	return result;
}

String Json::escape(const String &str)
{
	static std::map<char, String> escapeMethod = genEscapeMethod();

	String result = "";
	size_t len = str.size();
	for (size_t i = 0; i < len; ++i)
	{
		std::map<char, String>::const_iterator it = escapeMethod.find(str.at(i));
		if (it == escapeMethod.end())
		{
			result += str.at(i);
		}
		else
		{
			result += it->second;
		}
	}
	return result;
}

JsonArray::JsonArray(Json *json)
{
	m_json = json;
	needDeleteJson = false;
}

JsonArray::JsonArray()
{
	m_json = new Json();
	m_json->asArray();
	needDeleteJson = true;
}

JsonArray::JsonArray(const JsonArray &jsonArray)
{
	m_json = new Json(jsonArray.m_json);
	needDeleteJson = true;
}

JsonArray::~JsonArray()
{
	if (needDeleteJson)
	{
		delete m_json;
	}
}

size_t JsonArray::size() const
{
	return m_json->m_arrval.size();
}

void JsonArray::append(const Json &json)
{
	m_json->m_arrval.push_back(new Json(json));
}

bool JsonArray::remove(size_t index)
{
	if (index >= size())
	{
		return false;
	}
	m_json->m_arrval.erase(m_json->m_arrval.begin()+index);
	return true;
}

Json &JsonArray::toJson()
{
	return *m_json;
}

Json &JsonArray::operator[](size_t index)
{
	if (index >= size())
	{
		return JsonNone;
	}
	return *(m_json->m_arrval[index]);
}

const Json &JsonArray::operator[](size_t index) const
{
	if (index >= size())
	{
		return JsonNone;
	}
	return *(m_json->m_arrval[index]);
}

JsonArray &JsonArray::operator=(const JsonArray &jsonArray)
{
	m_json->asArray(jsonArray);
	return *this;
}

static size_t currentOutputJsonDepth = 0;	// 当前输出json到哪一层了
static size_t depthSpaces = 2;				// 每升一层往右偏移多少个空格

std::ostream &operator<<(std::ostream &out, const Json &json)
{
	switch (json.m_type)
	{
	case JSON_NONE:
		out << "none" << "\n";
		break;
	case JSON_BOOLEAN:
		if (json.m_bolval)
		{
			out << "true" << "\n";
		}
		else
		{
			out << "false" << "\n";
		}
		break;
	case JSON_NUMBER:
		out << json.m_numval << "\n";
		break;
	case JSON_STRING:
		out << "\"" << Json::escape(json.m_strval) << "\"" << "\n";
		break;
	case JSON_ARRAY:
	{
		out << Json(json).toArray();
		break;
	}
	case JSON_OBJECT:
	{
		if (currentOutputJsonDepth > 0)
		{
			out << "\n";
		}
		std::vector<String> keys = json.keys();
		size_t keycount = keys.size();
		for (size_t i = 0; i < keycount; ++i)
		{
			
			for (size_t i = 0; i < currentOutputJsonDepth*depthSpaces; ++i)
			{
				out << " ";
			}
			out << keys[i] << ": ";
			++currentOutputJsonDepth;
			out << json[keys[i]];
			--currentOutputJsonDepth;
		}
		break;
	}
	default:
		out << "Error: Unknow type!" << "\n";
	}
	return out;
}

std::ostream &operator<<(std::ostream &out, const JsonArray &jsonArray)
{
	if (currentOutputJsonDepth > 0)
	{
		out << "\n";
	}
	size_t count = jsonArray.size();
	for (size_t i = 0; i < count; ++i)
	{
		for (size_t i = 0; i < currentOutputJsonDepth*depthSpaces; ++i)
		{
			out << " ";
		}
		++currentOutputJsonDepth;
		out << "- " << jsonArray[i];
		--currentOutputJsonDepth;
	}

	return out;
}
