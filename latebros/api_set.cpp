#include "stdafx.h"
#include "api_set.hpp"

api_set::api_set()
{
	const auto peb = reinterpret_cast<uintptr_t>(NtCurrentTeb()->ProcessEnvironmentBlock);

	const auto api_set = *reinterpret_cast<API_SET_NAMESPACE_ARRAY**>(peb + 0x68);

	for (ULONG entry_index = 0; entry_index < api_set->count; ++entry_index)
	{
		const auto descriptor = api_set->entry(entry_index);

		auto dll_name = api_set->get_name(descriptor);
		std::for_each(dll_name.begin(), dll_name.end(), ::tolower);

		const auto host_data = api_set->get_host(descriptor);

		std::vector<std::wstring> hosts;
		for (ULONG j = 0; j < host_data->count; j++)
		{
			const auto host = host_data->entry(api_set, j);

			std::wstring host_name(reinterpret_cast<wchar_t*>(reinterpret_cast<uint8_t*>(api_set) + host->value_offset),
				host->value_length / sizeof(wchar_t));

			if (!host_name.empty())
			{
				//wprintf(L"%s - %s\n", dll_name, host_name.c_str());
				hosts.push_back(host_name);
			}
		}

		this->schema.emplace(std::move(dll_name), std::move(hosts));
	}
}

bool api_set::query(std::wstring& name) const
{
	// SEARCH FOR ANY ENTRIES OF OUR PROXY DLL
	const auto iter = std::find_if(this->schema.begin(), this->schema.end(), [&](const map_api_schema::value_type& val)
	{
		return std::search(name.begin(), name.end(), val.first.begin(), val.first.end()) != name.end();
	});

	if (iter != this->schema.end() && !iter->second.empty()) // FOUND
	{
		name = (iter->second.front() != name ? iter->second.front() : iter->second.back());
		return true;
	}

	return false;
}

bool api_set::query(std::string& name) const
{
	std::wstring wide_name(name.begin(), name.end());

	if (!query(wide_name))
		return false;

	name.assign(wide_name.begin(), wide_name.end());
	return true;
}