#ifndef SEARCH_H
#define SEARCH_H
#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include <string>

class Search
{
private:
	/* data */
public:
	ProcessInstance<CBox<void>, CArc<void>> *process;
	std::vector<MemoryRange> memory_range_vec;
	std::vector<uint64_t> memory_hit_vec;

	void setprocess(ProcessInstance<CBox<void>, CArc<void>> *process);

	size_t getpagemap();
	size_t getpagemap(ProcessInstance<CBox<void>, CArc<void>> *proc);

	template <typename T>
	int writememory(uint64_t addr, T bytes);
	bool dumpmemory(uint64_t addr, uint64_t size);

	template <typename T>
	int checkarray(uint8_t *buf, int size, T bytes, uint64_t baseaddr);
	int checkarray(uint8_t *buf, int size, std::string bytes, uint64_t baseaddr);

	template <typename T>
	bool searchmemory(T message);

	template <typename T>
	bool searchrepetition(T message);
	bool searchrepetition(std::string message);

	template <typename T>
	int checkarrayrepetition(uint8_t *buf, int size, T bytes, uint64_t baseaddr, std::vector<uint64_t> &tempv);
	int checkarrayrepetition(uint8_t *buf, int size, std::string bytes, uint64_t baseaddr, std::vector<uint64_t> &tempv);

	void write_mem(uint64_t targetaddress, const char *data);

	bool printhits();
	bool clear();

	Search();
	~Search();
};

bool Search::dumpmemory(uint64_t addr, uint64_t size)
{

	CSliceMut<uint8_t> buf;
	buf.data = new u_int8_t[size];
	buf.len = size;

	this->process->read_raw_into(addr, buf);

	std::filesystem::create_directory("dumpm");
	std::ofstream of;
	of.open("dumpm/" + std::to_string(addr), std::ios_base::trunc | std::ios_base::binary);
	if (!of)
	{
		return 0;
	}
	of.write((const char *)buf.data, size);
	of.close();
	delete buf.data;
	return 1;
}

template <typename T>
int Search::writememory(uint64_t addr, T bytes)
{
	int length = sizeof(bytes); // 8

	unsigned char *uc_arg = (unsigned char *)&bytes;
	uint8_t *lbytes = new uint8_t[length];
	// little endian
	for (int i = 0; i < length; i++)
	{
		lbytes[i] = uc_arg[i];
	}

	CSliceRef<uint8_t> buf;
	buf.data = lbytes;
	buf.len = length;

	this->process->write_raw(addr, buf);
	return 1;
}

template <typename T>
int Search::checkarray(uint8_t *buf, int size, T bytes, uint64_t baseaddr)
{
	int length = sizeof(bytes);

	unsigned char *uc_arg = (unsigned char *)&bytes;
	uint8_t *lbytes = new uint8_t[length];
	// little endian
	for (int i = 0; i < length; i++)
	{
		lbytes[i] = uc_arg[i];
	}

	for (uint64_t i = 0; i < size; i++)
	{
		if (buf[i] == lbytes[0])
		{
			for (int j = 0; j < length; j++)
			{
				if (buf[i + j] != lbytes[j])
				{
					break;
				}
				if (j == length - 1)
				{
					this->memory_hit_vec.push_back(baseaddr + i);
					std::cout << "hits: "
							  << "0x" << std::uppercase << std::hex << baseaddr + i << std::endl;
					// std::cout << &buf[i] << std::endl;
				}
			}
		}
	}
	delete[] lbytes;
	return 0;
}

int Search::checkarray(uint8_t *buf, int size, std::string bytes, uint64_t baseaddr)
{
	for (uint64_t i = 0; i < size; i++)
	{
		if (buf[i] == bytes[0])
		{
			for (int j = 0; j < bytes.length(); j++)
			{
				if (buf[i + j] != bytes[j])
				{
					break;
				}
				if (j == bytes.length() - 1)
				{
					this->memory_hit_vec.push_back(baseaddr + i);
					std::cout << "hits: "
							  << "0x" << std::uppercase << std::hex << baseaddr + i << std::endl;
					std::cout << &buf[i] << std::endl;
				}
			}
		}
	}

	return 0;
}

int Search::checkarrayrepetition(uint8_t *buf, int size, std::string bytes, uint64_t baseaddr, std::vector<uint64_t> &tempv)
{
	for (uint64_t i = 0; i < size; i++)
	{
		if (buf[i] == bytes[0])
		{
			for (int j = 0; j < bytes.length(); j++)
			{
				if (buf[i + j] != bytes[j])
				{
					break;
				}
				if (j == bytes.length() - 1)
				{
					// this->memory_hit_vec.push_back(baseaddr + i);
					tempv.push_back(baseaddr);
					std::cout << "hits: "
							  << "0x" << std::uppercase << std::hex << baseaddr + i << std::endl;
					std::cout << &buf[i] << std::endl;
				}
			}
		}
	}

	return 0;
}

template <typename T>
int Search::checkarrayrepetition(uint8_t *buf, int size, T bytes, uint64_t baseaddr, std::vector<uint64_t> &tempv)
{
	int length = sizeof(bytes);

	unsigned char *uc_arg = (unsigned char *)&bytes;
	uint8_t *lbytes = new uint8_t[length];
	// little endian
	for (int i = 0; i < length; i++)
	{
		lbytes[i] = uc_arg[i];
	}

	for (uint64_t i = 0; i < size; i++)
	{
		if (buf[i] == lbytes[0])
		{
			for (int j = 0; j < length; j++)
			{
				if (buf[i + j] != lbytes[j])
				{
					break;
				}
				if (j == length - 1)
				{
					// this->memory_hit_vec.push_back(baseaddr + i);
					tempv.push_back(baseaddr);
					std::cout << "hits: "
							  << "0x" << std::uppercase << std::hex << baseaddr + i << std::endl;
					// std::cout << &buf[i] << std::endl;
				}
			}
		}
	}

	return 0;
}

template <typename T>
bool Search::searchmemory(T message)
{
	this->memory_hit_vec.clear();
	for (auto &&info : this->memory_range_vec)
	{
		CSliceMut<uint8_t> buf;
		buf.data = new u_int8_t[info._1];
		buf.len = info._1;
		this->process->read_raw_into(info._0, buf);

		this->checkarray(buf.data, info._1, message, info._0);

		delete buf.data;
	}
	return true;
}

template <typename T>
bool Search::searchrepetition(T message)
{
	int length = sizeof(message);
	std::vector<uint64_t> tempv;
	for (auto &&i : this->memory_hit_vec)
	{
		CSliceMut<uint8_t> buf;
		buf.data = new u_int8_t[length];
		buf.len = length;
		this->process->read_raw_into(i, buf);
		checkarrayrepetition(buf.data, length, message, i, tempv);
		delete buf.data;
	}
	this->memory_hit_vec = tempv;
	return true;
}

void Search::write_mem(uint64_t targetaddress, const char *data)
{
	std::cout << "datasize=" << strlen(data) << " data=" << data << std::endl;

	CSliceRef<uint8_t> temp;
	temp.data = (uint8_t *)data;
	temp.len = strlen(data);
	this->process->write_raw(targetaddress, temp);
}

bool Search::searchrepetition(std::string message)
{
	std::vector<uint64_t> tempv;
	for (auto &&i : this->memory_hit_vec)
	{
		CSliceMut<uint8_t> buf;
		buf.data = new u_int8_t[message.length()];
		buf.len = message.length();
		this->process->read_raw_into(i, buf);
		checkarrayrepetition(buf.data, message.length(), message, i, tempv);
		delete buf.data;
	}
	this->memory_hit_vec = tempv;
	return true;
}

size_t Search::getpagemap(ProcessInstance<CBox<void>, CArc<void>> *proc)
{
	this->memory_range_vec.clear();
	imem gap = 0x1000000;
	proc->virt_page_map_range(gap, 0x0, 0x800000000000, [proc, this](MemoryRange info) mutable
							  {
		this->memory_range_vec.push_back(info);
		return true; });
	return this->memory_range_vec.size();
}

size_t Search::getpagemap()
{
	this->memory_range_vec.clear();
	imem gap = 0x1000000;
	this->process->virt_page_map_range(gap, 0x0, 0x800000000000, [this](MemoryRange info) mutable
									   {
		this->memory_range_vec.push_back(info);
		return true; });
	return this->memory_range_vec.size();
}

void Search::setprocess(ProcessInstance<CBox<void>, CArc<void>> *process)
{
	this->process = process;
	return;
}

bool Search::printhits()
{
	std::cout << std::dec << this->memory_hit_vec.size() << std::hex << std::endl;
	return true;
}

bool Search::clear()
{
	this->process = nullptr;
	this->memory_range_vec.clear();
	this->memory_hit_vec.clear();
	return true;
}

Search::Search()
{
}

Search::~Search()
{
}

#endif // SEARCH_H