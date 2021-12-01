#ifndef EYRE_SETTINGS_PARSE_H
#define EYRE_SETTINGS_PARSE_H

#include "eyre_settings.h"

class SettingsParseIni : public SettingsParse
{
public:
	SettingsParseIni(char sep = '/');
	SettingsParseIni(const SettingsParseIni &spi);
	virtual ~SettingsParseIni();
	bool loadFromString(const String &text);
	String saveToString() const;
};

#endif
