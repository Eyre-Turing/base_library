#include "eyre_settings_parse.h"

static std::map<char, char> genDescriptMethod()
{
	std::map<char, char> result;
	result['r'] = '\r';
	result['n'] = '\n';
	result['t'] = '\t';
	result['\"'] = '\"';
	result['\\'] = '\\';
	result['='] = '=';
	return result;
}

SettingsParseIni::SettingsParseIni(char sep) : SettingsParse(sep)
{

}

SettingsParseIni::SettingsParseIni(const SettingsParseIni &spi) : SettingsParse(spi)
{

}

SettingsParseIni::~SettingsParseIni()
{

}

bool SettingsParseIni::loadFromString(const String &text)
{
	static std::map<char, char> descript = genDescriptMethod();
	String _text = text;
	_text.replace("\r", "");
	int len = _text.size();
	int idx = _text.indexOf("[");
	while (idx >= 0 && idx < len)
	{
		String node = "";
		for (++idx; idx < len && _text[idx] != ']'; ++idx)
		{
			if (_text[idx] == '\n')	// 节点标识结束前遇到换行符，有问题
			{
				return false;
			}

			if (_text[idx] == '\\')	// 转义，需跳过
			{
				// 比如可能为 [aaa\[xxx\]bbb] ，这时，中间的 [ ] 要跳过
				node += descript[_text[++idx]];
			}
			else
			{
				node += _text[idx];
			}
		}
		// 此时 node 为节点名字
		m_data[node] = JsonNone;	// 创建节点

		Json &nodeJ = m_data[node];

		idx = _text.indexOf("\n", idx+1);
		int nodeEnd = _text.indexOf("\n[", idx+1);
		(nodeEnd == -1) && (nodeEnd = len);
		while (idx >= 0 && idx < nodeEnd)
		{
			String key = "";
			for (++idx; idx < len && _text[idx] != '='; ++idx)
			{
				if (_text[idx] == '\n')
				{
					return false;
				}

				if (_text[idx] == '\\')	// 转义
				{
					key += descript[_text[++idx]];
				}
				else
				{
					key += _text[idx];
				}
			}
			// 此时 key 为键名字
			
			int pos = _text.indexOf("\n", ++idx);
			(pos == -1) && (pos = len);
			
			String val = _text.mid(idx, pos-idx);

			nodeJ[key] = Json::descript(val);
			
			idx = pos;
		}

		// 寻找下一个节点
		idx = nodeEnd+1;
	}
	return false;
}

String SettingsParseIni::saveToString() const
{
	String ret = "";
	// 因为ini格式只有2层的情况，所以只考虑2层的情况，其余的一律不保存出去
	std::vector<String> keys = m_data.keys();
	size_t cnt = keys.size();
	for (size_t i = 0; i < cnt; ++i)
	{
		ret += "[" + Json::escape(keys[i]) + "]\n";
		const Json &data = m_data[keys[i]];
		std::vector<String> subkeys = data.keys();
		size_t num = subkeys.size();
		for (size_t j = 0; j < num; ++j)
		{
			ret += Json::escape(subkeys[j]) + "=" + data[subkeys[j]].toString() + "\n";
		}
	}
	return ret;
}
