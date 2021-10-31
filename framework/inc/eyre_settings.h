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

class Settings;

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
		Iterator(SettingsParse *sp, Json *json, Json *parse, const String &key, const String &lastKey);

		bool removeKey();	// 删除当前键

		bool remove();	// 删除当前指向的数据
		bool setValue(const Json &v);	// 原数据存在则修改，原数据不存在则创建
		Iterator &operator=(const Json &v);

		Json value() const;
		operator Json() const;

	private:
		SettingsParse *m_sp;
		Json *m_json;
		Json *m_parse;
		String m_key;
		String m_lastKey;
	};
	
	bool keyExist(const String &key);
	Iterator operator[](const String &key);

	bool remove();	// 删除当前数据
	bool setValue(const Json &v);
	SettingsParse &operator=(const Json &v);

	Json value() const;	// 获取当前值，失败返回JsonNone
	operator Json() const;

	friend class Settings;

protected:
	Settings *m_s;
	Json m_data;	// 储存配置数据
	char m_sep;
	Json *parseKey(const String &key, String &lastKey);		// sep作为键值对分割符，[ 和 ] 作为数组分割符号，要使用sep 或 [ ] 作为key的内容时，用反斜杠转义
	static Json *parseKey(Json *json, const String &key, bool touch, String &lastKey);	// touch就是不存在时自动创建
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
	 * 会设置 parse 的 m_s 为该对象
	 */
	Settings(const String &filename = "", SettingsParse *parse = NULL, char sep = '/', StringCodec codec = CODEC_AUTO);
	Settings(const Settings &settings);
	virtual ~Settings();

	bool setFilename(const String &filename);		// 文件存在则加载文件；文件不存在则记录文件路径，等待有写操作时创建文件并写入数据

	char getSep() const;
	void setSep(char sep);

	SettingsParse::Iterator operator[](const String &key);

	bool save(const String &filename = "");	// 在 parse 做了任何赋值操作的时候，会自动调用这个方法，filename不为空则为另存为

private:
	File m_file;
	SettingsParse *m_parse;
	StringCodec m_codec;
};

#endif
