#ifndef EYRE_JSON_H
#define EYRE_JSON_H

/*
 * json读写能力
 * 非线程安全
 * 
 * 作者: Eyre Turing (Eyre-Turing)
 * 最后编辑于: 2021-09-18 23:04
 */

#include <vector>
#include <map>
#include <iostream>
#include "eyre_string.h"

#define JSON_NONE		0		// JSON_NONE和JSON_OBJECT是可以互相转换的，当JSON_OBJECT没有数据时会变成JSON_NONE
#define JSON_BOOLEAN	1
#define JSON_NUMBER		2
#define JSON_STRING		3
#define JSON_ARRAY		4
#define JSON_OBJECT		5		// 当往JSON_NONE里添加数据时，JSON_NONE也会变成JSON_OBJECT

#define JSON_FAIL		0
#define JSON_APPEND		1
#define JSON_UPDATE		2

class JsonArray;

class Json
{
public:
	Json();
	Json(const Json &json);		// 深复制（包括ARRAY和OBJECT，都重新分配内存）

	Json(bool val);
	Json(double val);
	Json(const String &val);
	Json(const JsonArray &val);

	virtual ~Json();

	void asBoolean(bool val = false);					// 保存数据为布尔
	void asNumber(double val = 0);						// 保存数据为数字
	void asString(const String &val = "");				// 保存数据为文本
	void asArray();
	void asArray(const JsonArray &val);				// 保存数据为列表
	void asObject();
	void asObject(const Json &val);					// 保存数据为json对象

	Json &parent();
	const Json &parent() const;

	int type() const;

	bool boolean(bool def = false) const;	// 当该json保存的内容为布尔类型时可以用这个方法提取为布尔内容，修改返回值内容不会影响json
	double number(double def = 0) const;	// 提取为数字内容，修改返回值内容不会影响json
	String string(const String &def = "") const;	// 提取为文本内容，修改返回值内容不会影响json

	JsonArray toArray();		// 获取列表

	bool keyExist(const String &key) const;				// 判断键是否存在
	int set(const String &key, const Json &val);		// 加入键值对，返回操作状态，将以复制val的形式加入
	bool remove(const String &key);						// 删除键值对，this是json对象且key存在则删除并返回true，否则返回false
	
	static Json parseFromText(const String &text);	// 从文本解析json
	String toString(bool fold = false) const;		// 输出为字符串，参数fold控制输出文本是否折行美化

	std::vector<String> keys() const;

	class Iterator
	{
	public:
		Iterator();
		Iterator(Json *json, std::map<String, Json *>::iterator &it, const String &key);

		operator Json &();

		Iterator &operator=(const Json &json);
		Iterator &operator=(bool val);
		Iterator &operator=(double val);
		Iterator &operator=(const String &val);
		Iterator &operator=(const JsonArray &val);

	private:
		Json *m_j;
		std::map<String, Json *>::iterator m_it;
		String m_key;
	};

	Iterator operator[](const String &key);	// 返回key对应的子json对象
	const Json &operator[](const String &key) const;

	Json &operator=(const Json &json);
	Json &operator=(bool val);
	Json &operator=(double val);
	Json &operator=(const String &val);
	Json &operator=(const JsonArray &val);

	static String escape(const String &str);

	friend std::ostream &operator<<(std::ostream &out, const Json &json);

	friend class JsonArray;

private:
	Json *m_parent;

	int m_type;
	bool m_bolval;
	double m_numval;
	String m_strval;
	std::vector<Json *> m_arrval;
	std::map<String, Json *> m_objval;

	Json(const Json &json, Json *parent);
	void asObject(const Json &val, Json *parent);
	void cleanOldTypeData();
};

class JsonArray
{
public:
	JsonArray();
	JsonArray(const JsonArray &jsonArray);	// 深复制
	virtual ~JsonArray();

	size_t size() const;
	void append(const Json &json);	// 往列表里添加元素
	bool remove(size_t index);

	Json &toJson();					// 整个列表作为一个json对象

	Json &operator[](size_t index);
	const Json &operator[](size_t index) const;

	JsonArray &operator=(const JsonArray &jsonArray);

	friend std::ostream &operator<<(std::ostream &out, const JsonArray &jsonArray);

	friend class Json;

private:
	Json *m_json;
	bool needDeleteJson;
	JsonArray(Json *json);
};

extern Json JsonNone;	// 当操作失败时会返回此Json
extern Json::Iterator JsonIteratorNone;
extern JsonArray JsonArrayNone;

std::ostream &operator<<(std::ostream &out, const Json &json);
std::ostream &operator<<(std::ostream &out, const JsonArray &jsonArray);

#endif
