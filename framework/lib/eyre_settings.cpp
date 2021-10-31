/*
 * 任意格式配置读写
 *
 * 作者: Eyre Turing (Eyre-Turing)
 * 最后编辑于: 2021-10-30 10:16
 */

#include "eyre_settings.h"

SettingsParse::SettingsParse(char sep)
{
	m_s = NULL;
	m_sep = sep;
	m_data = JsonNone;
}

SettingsParse::SettingsParse(const SettingsParse &sp)
{
	m_s = NULL;
	m_data = sp.m_data;
	m_sep = sp.m_sep;
}

SettingsParse::~SettingsParse()
{

}

char SettingsParse::getSep() const
{
	return m_sep;
}

void SettingsParse::setSep(char sep)
{
	m_sep = sep;
}

static Json *enterKey(Json *ret, const String &currentKey, bool touch)
{
	if (ret == NULL)
	{
		return NULL;
	}
	Json *json = ret;
	ret = &((*ret)[currentKey].obj());
	if (ret->isNull())
	{
		if (touch)
		{
			if (json->type() != JSON_OBJECT && json->type() != JSON_NONE)
			{
				json->asObject();
			}
			(*json)[currentKey] = JsonNone;
			ret = &((*json)[currentKey].obj());
		}
		else
		{
			return NULL;
		}
	}
	return ret;
}

static Json *enterKey(Json *ret, unsigned int idx, bool touch)
{
	if (ret == NULL)
	{
		return NULL;
	}
	Json *json = ret;
	JsonArray array = ret->toArray();
	if (array.isNull())
	{
		if (touch)
		{
			if (json->type() != JSON_ARRAY && json->type() != JSON_NONE)
			{
				json->asArray();
			}
			array = json->toArray();
		}
		else
		{
			return NULL;
		}
	}
	ret = &(array[idx]);
	if (ret->isNull())
	{
		if (touch)
		{
			for (size_t ai = array.size(); ai <= idx; ++ai)	// ai: append id
			{
				array.append(JsonNone);
			}
			ret = &(array[idx]);
		}
		else
		{
			return NULL;
		}
	}
	return ret;
}

Json *SettingsParse::parseKey(const String &key, String &lastKey)
{
	return parseKey(&m_data, key, false, lastKey);
}

/*
 * 设计逻辑：出现反斜杠，后面无论出现什么内容都照搬，即反斜杠后面表示原生内容
 * lastKey 返回最后一级键
 * 默认任何一级键都不能为空字符串（不能为""，去掉双引号）
 * key不存在返回NULL，lastKey为最后一级键（不为空）
 * key格式错误返回NULL，同时lastKey为空字符串
 */
Json *SettingsParse::parseKey(Json *json, const String &key, bool touch, String &lastKey)
{
	Json *ret = json;
	bool escapeStatus = false;
	String currentKey;
	size_t len = key.size();
	for (size_t i = 0; i < len; ++i)
	{
		if (escapeStatus)
		{
			currentKey += key.at(i);	// 直接加，即原生
			escapeStatus = false;
		}
		else
		{
			if (key.at(i) == '\\')
			{
				escapeStatus = true;
			}
			else
			{
				if (key.at(i) == '/')	// 键值对
				{
					if (currentKey != "")
					{
						lastKey = currentKey;
						if (enterKey(ret, currentKey, touch) == NULL)	// 进入键后返回为空，说明无法进入该键
						{
							ret = NULL;		// 不直接 return 是为了用这个函数继续检验 key 的格式是否正确
						}
					}
					currentKey = "";
				}
				else if (key.at(i) == '[')	// 数组开端，寻找]
				{
					if (currentKey != "")
					{
						lastKey = currentKey;
						if (enterKey(ret, currentKey, touch) == NULL)
						{
							ret = NULL;
						}
					}
					currentKey = "";
					for (; i < len && key.at(i) != ']'; ++i)
					{
						currentKey += key.at(i);
					}
					if (i >= len)
					{
						lastKey = "";
						return NULL;
					}
					if (currentKey == "")
					{
						lastKey = "";
						return NULL;
					}
					lastKey = currentKey;
					if (enterKey(ret, currentKey.toUInt(), touch) == NULL)
					{
						ret = NULL;
					}
					currentKey = "";
					++i;
				}
				else if (key.at(i) == ']')	//异常，出现 [ 之前不能出现 ]
				{
					lastKey = "";
					return NULL;
				}
				else
				{
					currentKey += key.at(i);
				}
			}
		}
	}
	if (currentKey != "")	// 最后一级了
	{
		lastKey = currentKey;
		if (enterKey(ret, currentKey, touch) == NULL)	// 进入键后返回为空，说明无法进入该键
		{
			ret = NULL;
		}
		currentKey = "";
	}
	return ret;
}

SettingsParse::Iterator::Iterator(SettingsParse *sp ,Json *json, Json *parse, const String &key, const String &lastKey)
{
	m_sp = sp;
	m_json = json;
	m_parse = parse;	// parse 为 NULL 则表示当前键不存在，当出现赋值操作时需要按照 key 来创建键
	m_key = key;
	m_lastKey = lastKey;
}

bool SettingsParse::Iterator::removeKey()
{
	if (!m_parse)
	{
		return true;
	}
	if (m_lastKey != "")
	{
		if (m_parse->parent().isNull())
		{
			return false;
		}
		return m_parse->parent().remove(m_lastKey);
	}
	else
	{
		return false;
	}
}

bool SettingsParse::Iterator::remove()
{
	if (!m_parse)
	{
		return true;
	}
	*m_parse = JsonNone;
	if (m_sp && m_sp->m_s)
	{
		return m_sp->m_s->save();
	}
	return true;
}

bool SettingsParse::Iterator::setValue(const Json &v)
{
	if (m_parse)	// 数据已经存在，修改
	{
		*m_parse = v;
	}
	else if (m_lastKey != "")	// 数据不存在，且key的格式是正确的，则强行创建键并赋值
	{
		String lastKey;
		m_parse = SettingsParse::parseKey(m_json, m_key, true, lastKey);
		*m_parse = v;
	}
	else
	{
		return false;
	}
	if (m_sp && m_sp->m_s)
	{
		return m_sp->m_s->save();
	}
	return true;
}

SettingsParse::Iterator &SettingsParse::Iterator::operator=(const Json &v)
{
	setValue(v);
	return *this;
}

Json SettingsParse::Iterator::value() const
{
	if (m_parse)
	{
		return *m_parse;
	}
	return JsonNone;
}

SettingsParse::Iterator::operator Json() const
{
	return value();
}

bool SettingsParse::keyExist(const String &key)
{
	String lastKey;
	return parseKey(key, lastKey) != NULL;
}

SettingsParse::Iterator SettingsParse::operator[](const String &key)
{
	String lastKey;
	Json *parse = parseKey(key, lastKey);
	return Iterator(this ,&m_data, parse, key, lastKey);
}

bool SettingsParse::remove()
{
	return setValue(JsonNone);
}

bool SettingsParse::setValue(const Json &v)
{
	m_data = v;
	if (m_s)
	{
		return m_s->save();
	}
	return true;
}

SettingsParse &SettingsParse::operator=(const Json &v)
{
	setValue(v);
	return *this;
}

Json SettingsParse::value() const
{
	return m_data;
}

SettingsParse::operator Json() const
{
	return value();
}

Settings::Settings(const String &filename, SettingsParse *parse, char sep, StringCodec codec)
{
	m_parse = parse;
	m_codec = codec;

	if (m_parse == NULL)
	{
		m_parse = new SettingsParseJson();
	}

	m_parse->setSep(sep);
	m_parse->m_s = this;

	setFilename(filename);
}

Settings::Settings(const Settings &settings)
{
	m_file = settings.m_file;
	m_parse = settings.m_parse;
	m_codec = settings.m_codec;
}

Settings::~Settings()
{
	delete m_parse;
}

bool Settings::setFilename(const String &filename)
{
	if (!m_file.setFilename(filename))
	{
		return false;
	}
	if (m_file.exist())
	{
		if (!m_file.open(FILE_OPEN_MODE_Read))
		{
			return false;
		}
		m_parse->loadFromString(m_file.readAll().toString(m_codec));
		m_file.close();
	}
	return true;
}

char Settings::getSep() const
{
	return m_parse->getSep();
}

void Settings::setSep(char sep)
{
	m_parse->setSep(sep);
}

SettingsParse::Iterator Settings::operator[](const String &key)
{
	return (*m_parse)[key];
}

bool Settings::save(const String &filename)
{
	if (filename != "")	// 另存为
	{
		if (!m_file.setFilename(filename))
		{
			return false;
		}
	}
	if (!m_file.open(FILE_OPEN_MODE_Write))
	{
		return false;
	}
	bool status = m_file.write(ByteArray::fromString(m_parse->saveToString(), m_codec));
	m_file.close();
	return status;
}

SettingsParseJson::SettingsParseJson(char sep) : SettingsParse(sep)
{

}

SettingsParseJson::SettingsParseJson(const SettingsParseJson &spj) : SettingsParse(spj)
{

}

SettingsParseJson::~SettingsParseJson()
{

}

bool SettingsParseJson::loadFromString(const String &text)
{
	m_data = Json::parseFromText(text);
}

String SettingsParseJson::saveToString() const
{
	return m_data.toString(true);
}
