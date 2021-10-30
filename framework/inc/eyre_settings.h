#ifndef EYRE_SETTINGS_H
#define EYRE_SETTINGS_H

/*
 * 任意格式配置读写
 *
 * 作者: Eyre Turing (Eyre-Turing)
 * 最后编辑于: 2021-10-30 10:16
 */

#include "eyre_string.h"
#include "eyre_json.h"
#include "eyre_file.h"
#include <map>

class SettingsParse
{
public:
	SettingsParse(char sep = '/');	// sep是分隔符，键值对用sep分隔，数组用[下标]
	SettingsParse(const SettingsParse &sp);
	virtual ~SettingsParse();

	char getSep() const;
	void setSep(char sep);

	virtual bool loadFromString(const String &text) = 0;		// 必须由子类继承
	virtual String saveToString() const = 0;					// 必须由子类继承

	class Iterator
	{
	public:
		Iterator(SettingsParse *sp, const String &key);
		SettingsParse &obj();
		operator SettingsParse &();

		void remove();	// 删除当前指向的数据
		void setValue(const Json &v);
		Iterator &operator=(const Json &v);

	private:
		SettingsParse *m_sp;
		String m_key;
	};
	
	Iterator operator[](const String &key);
	const SettingsParse &operator[](const String &key) const;

	void remove();	// 删除当前数据
	void setValue(const Json &v);
	SettingsParse &operator=(const Json &v);

	Json value() const;	// 获取当前值，失败返回JsonNone
	operator String() const;

private:
	Json m_data;	// 储存配置数据
	Json &parseKey(const String &key);
	const Json &parseKeyConst(const String &key) const;
};

class SettingsParseJson : public SettingsParse
{
public:
	SettingsParseJson(char sep = '/');
	SettingsParseJson(const SettingsParseJson &spj);
	virtual ~SettingsParseJson();
	bool loadFromString(const String &text);
	String saveToString() const;
};

class Settings
{
public:
	/*
	 * 会改变 parse 所有权，parse 的所有权将归于该对象，当该对象析构时，会析构 parse
	 * parse 为 NULL 表示自动创建 SettingsParseJson 对象作为 parse
	 */
	Settings(const String &filename = "", SettingsParse *parse = NULL, char sep = '/', StringCodec codec = CODEC_AUTO);
	Settings(const Settings &settings);
	virtual ~Settings();

	void setFilename();		// 文件存在则加载文件；文件不存在则记录文件路径，等待有写操作时创建文件并写入数据

	char getSep() const;
	void setSep(char sep);

	SettingsParse::Iterator operator[](const String &key);
	const SettingsParse &operator[](const String &key) const;

private:
	File m_file;
	SettingsParse *m_parse;
	StringCodec m_codec;
};

#endif
