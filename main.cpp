#include "include/memflow.hpp"
#include "include/search.hpp"

#include <iostream>
#include <stdio.h>
#include <vector>

OsInstance<> os;
Search search;

void fmt_arch(char *arch, int n, ArchitectureIdent ident);

void get_process(){
	printf("Process List:\n");

	printf("%-4s | %-8s | %-10s | %-10s | %s\n", "Seq", "Pid", "Sys Arch", "Proc Arch", "Name");

	int i = 0;

	os.process_info_list_callback([i](ProcessInfo info) mutable {
		char sys_arch[11];
		char proc_arch[11];

		fmt_arch(sys_arch, sizeof(sys_arch), info.sys_arch);
		fmt_arch(proc_arch, sizeof(proc_arch), info.proc_arch);

		printf("%-4d | %-8d | %-10s | %-10s | %s\n", i++, info.pid, sys_arch, proc_arch, info.name);

		return true;
	});
}

int open_targetprocess(std::string &targetprocess, ProcessInstance<CBox<void>, CArc<void>> &proc){

	os.process_by_name(targetprocess, &proc);
	if (!proc.vtbl_process)
	{
		log_error(("unable to "+ targetprocess).c_str());
		return 0;
	}

	bool is_alive = proc.state().tag == ProcessState::Tag::ProcessState_Alive;
	if (!is_alive)
	{
		std::string logerrorstr = std::string("unable to find ") + targetprocess;
		log_error(logerrorstr.c_str());

		return 1;
	}

	std::cout << "found " << targetprocess << " running, path -> " << proc.info()->path << std::endl;

	search.setprocess(&proc);
	search.getpagemap();
	ModuleInfo moduleinfo;
	proc.primary_module(&moduleinfo);
	std::cout << "base 0x" << std::hex << moduleinfo.base << std::endl;	
	
	return 2;
}

constexpr unsigned int str2int(std::string in, int h = 0)
{
	const char* str = in.c_str();
	return !str[h] ? 5381 : (str2int(str, h+1) * 33) ^ str[h];
}

int main(int argc, char *argv[]) {
	log_init(LevelFilter::LevelFilter_Info);

	Inventory *inventory = inventory_scan();

	if (!inventory) {
		log_error("unable to create inventory");
		return 1;
	}

	printf("inventory initialized: %p\n", (void*)inventory);

	const char *conn_name = argc > 1? argv[1]: "qemu";
	const char *conn_arg = argc > 2? argv[2]: "";
	const char *os_name = argc > 3? argv[3]: "win32";
	const char *os_arg = argc > 4? argv[4]: "";

	ConnectorInstance<> connector, *conn = conn_name[0] ? &connector : nullptr;

	if (conn) {
		if (inventory_create_connector(inventory, conn_name, conn_arg, &connector)) {
			printf("unable to initialize connector\n");
			inventory_free(inventory);
			return 1;
		}

		printf("connector initialized: %p\n", connector.container.instance.instance);
	}

	if (inventory_create_os(inventory, os_name, os_arg, conn, &os)) {
		printf("unable to initialize OS\n");
		inventory_free(inventory);
		return 1;
	}

	inventory_free(inventory);

	printf("os initialized: %p\n", os.container.instance.instance);

	auto info = os.info();
	char arch[11];
	fmt_arch(arch, sizeof(arch), info->arch);

	printf("Kernel base: %llx\nKernel size: %llx\nArchitecture: %s\n", (unsigned long long int)info->base, (unsigned long long int)info->size, arch);

	get_process();

	ProcessInstance<CBox<void>, CArc<void>> proc;
	std::cout << "type target process name ex. main.exe" << std::endl;
	std::string targetprocess("main.exe");
	std::getline(std::cin >> std::ws, targetprocess);

	open_targetprocess(targetprocess, proc);

	std::string message = "There is nothing here";
	std::string replacemessage = "There was something :O";
	u_int64_t last = 0;
	while (true){
		std::string c("");
		std::string c2("");
		u_int8_t u8 = 0;
		u_int16_t u16 = 0;
		u_int32_t u32 = 0;
		u_int64_t u64 = 0;
		u_long addr = 0;
		long index = 0;

		std::cout << "{proc}: specify/change target process=>value:str" << std::endl;
		std::cout << "{s}: searchmemory=>{1|2|4|8|str} \\n value" << std::endl;
		std::cout << "{sr}: searchrepetition=>{1|2|4|8|str} \\n value" << std::endl;
		std::cout << "{wm}: writememory int =>addr \\n value" << std::endl;
		std::cout << "{wm str}: writememory str=>addr \\n value" << std::endl;
		std::cout << "{wm all}: writememory all hits=>value" << std::endl;
		std::cout << "{dump}: dumpmemory ???" << std::endl;
		std::cout << "{dump all}: dumpmemory ???" << std::endl;
		std::cout << "{v}: view hit" << std::endl;
		std::getline(std::cin >> std::ws, c);

		switch (str2int(c))
		{
		case str2int("proc"):
			get_process();
			std::cout << "type target process name ex. main.exe" << std::endl;
			std::getline(std::cin >> std::ws, targetprocess);
			open_targetprocess(targetprocess, proc);
			break;
		case str2int("s"):
			std::getline(std::cin >> std::ws, c2);
			switch (str2int(c2))
			{
			case str2int("1"):
			case str2int("u8"):
				std::cin >> u8 ;
				search.searchmemory(u8);
				break;
			case str2int("2"):
			case str2int("u16"):
				std::cin >> u16 ;
				search.searchmemory(u16);
				break;
			case str2int("4"):
			case str2int("u32"):
				std::cin >> u32 ;
				search.searchmemory(u32);
				break;
			case str2int("8"):
			case str2int("u64"):
				std::cin >> u64 ;
				search.searchmemory(u64);
				break;
			case str2int("s"):
			case str2int("str"):
				std::cin >> message ;
				search.searchmemory(message);
				break;
			default:
				break;
			}
			break;
		case str2int("sr"):
			std::cout << "last search: " << std::dec << last << std::endl;
			std::cin >> c2 ;
			switch (str2int(c2))
			{
			case str2int("1"):
			case str2int("u8"):
				std::cin >> u8 ;
				search.searchrepetition(u8);
				break;
			case str2int("2"):
			case str2int("u16"):
				std::cin >> u16 ;
				search.searchrepetition(u16);
				break;
			case str2int("4"):
			case str2int("u32"):
				std::cin >> u32 ;
				search.searchrepetition(u32);
				break;
			case str2int("8"):
			case str2int("u64"):
				std::cin >> u64 ;
				search.searchrepetition(u64);
				break;
			case str2int("s"):
			case str2int("str"):
				std::getline(std::cin >> std::ws, message);
				search.searchrepetition(message);
				break;
			default:
				break;
			}
			break;
		case str2int("wm"):
			std::cout << "type memory addr. ex 0x26FEA9460A0" << std::endl;
			std::cin >> std::hex >> addr;
			std::cout << "type value dec" << std::endl;
			std::cin >> std::dec >> u32 ;
			search.writememory(addr,u32);
			break;
		case str2int("wm str"):
			std::cout << "type memory addr. ex 0x26FEA9460A0" << std::endl;
			std::cin >> std::hex >> addr;
			std::cout << "type value" << std::endl;
			std::getline(std::cin >> std::ws, replacemessage);
			search.write_mem(addr, replacemessage.c_str());
			break;
		case str2int("wm all"):
			std::cin >> std::dec >> u32 ;
			for (auto &&i : search.memory_hit_vec){
				search.writememory(i,u32);
			}
			break;
		case str2int("d"):
		case str2int("dump"):
			std::cout << "type memory addr. ex 0x26FEA9460A0" << std::endl;
			std::cin >> std::hex >> addr;
			std::cout << "type value hex" << std::endl;
			std::cin >> std::hex >> u32 ;
			search.dumpmemory(addr,u32);
			system(("xxd -u -l 500 -g 1 dumpm/" + std::to_string(addr)).c_str());
			break;
		case str2int("da"):
		case str2int("dump all"):
			std::cout << "start dump" << std::endl;
			search.dumpmemoryall();
			std::cout << "dumped" << std::endl;
			break;
		case str2int("v"):
		case str2int("viewhit"):
			while (index>=0 && search.memory_hit_vec.size() != 0)
			{
				std::cout << "hits " << std::dec << search.memory_hit_vec.size() << std::endl;
				std::cout << "view index " << std::dec << index << std::endl;
				std::cout << "-, any, =, s, e, q " << search.memory_hit_vec.size() << std::endl;

				std::getline(std::cin >> std::ws, c2);
				switch (str2int(c2))
				{
					case str2int("q"):
						index = -1; 
						break;
					case str2int("-"):
						index--;
						index<0 ? index = 0:index = index;
						break;
					case str2int("="):
						break;
					case str2int("s"):
						index = 0;
						break;
					case str2int("e"):
						index = search.memory_hit_vec.size()-1;
						break;
					default:
						index++;
						index>=search.memory_hit_vec.size() ? index = search.memory_hit_vec.size()-1:index = index;
						break;
				}
				if (index < 0) break;
				search.dumpmemory(search.memory_hit_vec[index]-0x50,0x100);
				system(("xxd -u -g 1 dumpm/" + std::to_string(search.memory_hit_vec[index]-0x50)).c_str());
				std::cout << "addr 0x" << std::hex << search.memory_hit_vec[index] << std::endl;
			}
			break;
		default:
			break;
		}
		search.printhits();
		u8 != 0 ? last = u8: last = last; 
		u16 != 0 ? last = u16: last = last; 
		u32 != 0 ? last = u32: last = last; 
		u64 != 0 ? last = u64: last = last;
	}
	return 1;
}

void fmt_arch(char *arch, int n, ArchitectureIdent ident) {
	switch (ident.tag) {
		case ArchitectureIdent::Tag::ArchitectureIdent_X86:
			snprintf(arch, n, "X86_%d", ident.x86._0);
			break;
		case ArchitectureIdent::Tag::ArchitectureIdent_AArch64:
			snprintf(arch, n, "AArch64");
			break;
		default:
			snprintf(arch, n, "Unknown");
	}
}