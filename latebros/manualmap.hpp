#pragma once
#include "stdafx.h"
#include "process.hpp"
#include "portable_executable.hpp"

struct map_ctx
{
	std::string image_name;
	portable_executable pe;
	uintptr_t local_image;
	uintptr_t remote_image;
	uint8_t* get_pe_buffer();

	map_ctx() : pe(std::vector<uint8_t>()), remote_image(0) {}
	map_ctx(std::string new_image_name, std::vector<uint8_t> new_buffer) :
		image_name(new_image_name), pe(new_buffer) {}
};

using module_list = std::unordered_map<std::string, uintptr_t>;

namespace injection
{
	class manualmap
	{
	public:
		manualmap(process& proc) : process(proc) { }
		uintptr_t inject(const std::vector<uint8_t>& buffer);

	private:
		bool map_image(map_ctx& ctx);
		uintptr_t find_or_map_dependency(const std::string& image_name);
		void write_headers(map_ctx& ctx);
		void write_image_sections(map_ctx& ctx);
		void relocate_image_by_delta(map_ctx& ctx);
		void fix_import_table(map_ctx& ctx);

		module_list linked_modules;
		std::vector<map_ctx> mapped_modules;
		process& process;
	};
}