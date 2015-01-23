#ifndef STINGRAYKIT_SERIALIZATION_SETTINGSVALUEFORWARD_H
#define STINGRAYKIT_SERIALIZATION_SETTINGSVALUEFORWARD_H

#include <map>
#include <vector>
#include <stingraykit/self_counter.h>

namespace stingray
{
	class SettingsValue;
	STINGRAYKIT_DECLARE_SELF_COUNT_PTR(SettingsValue);

	typedef std::map<const std::string, SettingsValueSelfCountPtr> SettingsValueMap;
	typedef std::vector<SettingsValueSelfCountPtr> SettingsValueList;

}

#endif