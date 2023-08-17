
#include "syn_allocator.h"

#include <iomanip> 		// std::setw(), std::right and std::hex.
#include <algorithm> 	// std::sort() and std::reverse().


namespace Syn {

    // function pointer for malloc_size_func (linux) or _msize (windows)
#ifdef __linux__
    malloc_size_func_t malloc_size_func = malloc_usable_size;
#elif defined(_WIN32)
    malloc_size_func_t malloc_size_func = _msize;
#endif
	
	
    // static member variable declarations
    std::unordered_map<void*, memory_alloc_info> memory_log::s_memory;
	
    memory_usage memory_log::s_usageType[4];
    memory_usage memory_log::s_usageTotal = memory_usage();
    std::string memory_log::s_sz;
    std::string memory_log::s_lastLogEntry;
	

    // initialization of global objects
    MemoryResource* s_memorySTL = new MemoryResource(memory_log::insert, memory_log::remove, AllocType::STL);
    MemoryResource* s_memoryAllocShared = new MemoryResource(memory_log::insert, memory_log::remove, AllocType::SHARED);
    //MemoryResource* s_memoryExplicit 	= new MemoryResource(memory_log::insert, memory_log::remove, AllocType::EXPLICIT);

    STLMemoryResourceHandler s_STLMemRsrcHandler;


    //-----------------------------------------------------------------------------------
    void memory_log::insert(void* _mem_addr, 
			    uint32_t _alloc_bytes, 
			    uint32_t _alloc_block, 
			    AllocType _alloc_type,
			    const std::string& _caller_fnc) 
    { 
	s_memory[_mem_addr] = memory_alloc_info(_alloc_bytes, _alloc_block, 0, 0, _alloc_type, _caller_fnc);
	// update memory usage
	s_usageType[(int)_alloc_type].update_alloc(_alloc_bytes, _alloc_block);
	s_usageTotal.update_alloc(_alloc_bytes, _alloc_block);
    }
	

    //-----------------------------------------------------------------------------------
    void memory_log::remove(void* _mem_addr, uint32_t _dealloc_bytes, uint32_t _dealloc_block, AllocType _alloc_type)
    {
	auto iterator = s_memory.find(_mem_addr);
	SYN_ASSERT(iterator != s_memory.end());
	SYN_ASSERT(iterator->second.m_allocType == _alloc_type);
	iterator->second.m_deallocBytes = _dealloc_bytes;
	iterator->second.m_deallocBlock = _dealloc_block;
	// update memory usage
	s_usageType[(int)_alloc_type].update_dealloc(_dealloc_bytes, _dealloc_block);
	s_usageTotal.update_dealloc(_dealloc_bytes, _dealloc_block);
    }


    //-----------------------------------------------------------------------------------
    const std::string& memory_log::print_alloc_all(bool _omit_deallocated, bool _use_std_out)
    {
	std::string exp = "MEMORY USAGE REPORT\n";
	for (auto i : {AllocType::STL, AllocType::SHARED, AllocType::EXPLICIT} )
	    exp += print_alloc_type(i, _omit_deallocated);

	// TODO: add 'overhead' of the memory_log class, the STLMemoryResourceHandler class
	// and the MemoryResourceShared instance to the total memory footprint of the program,
	// as a separate entry below.
	memory_usage total = get_usage_total();
	std::ostringstream ss;
	ss << "TOTAL MEMORY USAGE\n";
	ss << "Allocated:   " << std::right << std::setw(12) << _fmt_sz(total.m_physicalAlloc) << std::right << std::setw(14) << " (" + _fmt_sz(total.m_virtualAlloc) + ")" << "\n";
	ss << "Deallocated: " << std::right << std::setw(12) << _fmt_sz(total.m_physicalDealloc) << std::right << std::setw(14) << " (" + _fmt_sz(total.m_virtualDealloc) + ")" << "\n";
	ss << "Difference:  " << std::right << std::setw(12) << _fmt_sz(total.m_physicalAlloc - total.m_physicalDealloc) << std::right << std::setw(14) << 
	    " (" + _fmt_sz(total.m_virtualAlloc - total.m_virtualDealloc) + ")" <<  "\n";
	exp += ss.str();

	if (_use_std_out)
	    std::cout << exp;

	s_lastLogEntry = exp;
	return s_lastLogEntry;
    }


    //-----------------------------------------------------------------------------------
    std::string memory_log::print_alloc_type(AllocType _alloc_type, bool _omit_deallocated)
    {
	/* (not efficient) To be able to print from high to low 
	 * memory, all memory allocations of type _alloc_type must be 
	 * obtained in a first run, and stored in a vector and sorted.
	 */
	std::vector<void*> vec_mem;
	vec_mem.reserve(s_memory.size());
	for (auto& it : s_memory)
	{
	    // only store of type _alloc_type
	    if (it.second.m_allocType == _alloc_type)
	    {
		// store the memory address (i.e. the key in s_memory)
		vec_mem.push_back(it.first);
	    }
	}

	// sort the addresses in reverse order (from high to low)
	std::sort(vec_mem.begin(), vec_mem.end());
	std::reverse(vec_mem.begin(), vec_mem.end());
		
	std::vector<std::string> out;
	for (auto& key : vec_mem)
	{
	    auto map_entry = s_memory[key];
	    if ((_omit_deallocated) && 
		(map_entry.m_allocBytes == map_entry.m_deallocBytes) && 
		(map_entry.m_allocBlock == map_entry.m_deallocBlock))
		continue;
	    else
	    {
		std::ostringstream ss;
		ss << std::setw(4) << "";
		ss << std::right << std::setw(90) << map_entry.m_callerFnc + (map_entry.m_callerFnc == "" ? "(no caller function specified)" : "");
		ss << std::setw(4) << "" << format_mem_addr(key);
		ss << std::right << std::setw(12) << _fmt_sz(map_entry.m_allocBytes) <<  std::right << std::setw(14) << " (" + _fmt_sz(map_entry.m_allocBlock) + ")";
		if (!_omit_deallocated) ss << std::right << std::setw(12) << _fmt_sz(map_entry.m_deallocBytes) << std::right << std::setw(14) << " (" + _fmt_sz(map_entry.m_deallocBlock) + ")\n";
		else ss << "\n";

		out.push_back(ss.str());
	    }
	}
	// print
	std::string exp;
	if (out.size() > 0)
	{
	    std::ostringstream ss;
	    ss << std::setw(20) << std::left << AllocTypeStr(_alloc_type);
	    ss << std::setw(4) << "";
	    ss << std::setw(49) << std::right << "CALLING FUNCTION";
	    ss << std::setw(21) << std::right << "CALL";
	    ss << std::setw(27) << std::right << "MEMORY ADDRESS";
	    ss << std::setw(26) << std::right << "ALLOC (BLOCK)";
	    if (!_omit_deallocated) ss << std::setw(26) << std::right << "DEALLOC (BLOCK)\n";
	    else ss << "\n";
	    for (auto& o : out)
		ss << o;

	    memory_usage usage = get_usage_alloc_type(_alloc_type);
	    ss << "Allocated:   " << std::right << std::setw(12) << _fmt_sz(usage.m_physicalAlloc) << std::right << std::setw(14) << " (" + _fmt_sz(usage.m_virtualAlloc) + ")" << "\n";
	    ss << "Deallocated: " << std::right << std::setw(12) << _fmt_sz(usage.m_physicalDealloc) << std::right << std::setw(14) << " (" + _fmt_sz(usage.m_virtualDealloc) + ")" << "\n";
	    ss << "Difference:  " << std::right << std::setw(12) << _fmt_sz(usage.m_physicalAlloc - usage.m_physicalDealloc) << std::right << std::setw(14) << 
		" (" + _fmt_sz(usage.m_virtualAlloc - usage.m_virtualDealloc) + ")" << "\n\n";
	    exp = ss.str();
	}

	return exp;
    }


    //-----------------------------------------------------------------------------------
    uint32_t memory_log::get_alloc_bytes(void* _mem_addr)
    {
	auto iterator = s_memory.find(_mem_addr);
	SYN_ASSERT(iterator != s_memory.end());
	return iterator->second.m_allocBytes;
    }


    //-----------------------------------------------------------------------------------
    std::string pretty_func(const char* _fnc)
    {
	/* (lifted from Synapse/Debug/Log.hpp) */
	std::string fnc(_fnc);
	size_t firstParanthesis = fnc.find('(');
	size_t lastSpace = fnc.substr(0, firstParanthesis).rfind(' ');
	std::string fncName = lastSpace == std::string::npos ? fnc.substr(0, firstParanthesis) : fnc.substr(lastSpace+1, firstParanthesis-lastSpace-1);
	fnc = fncName.substr(0, firstParanthesis);
	return fnc;
    }


    //-----------------------------------------------------------------------------------
    std::string get_caller_signature(const char* _c_file, const char* _c_line, const char* _c_fnc, const char* _c_type)
    { 
	char buffer[512];
	memset(buffer, 0, 512);
	sprintf(buffer, "%s:%s: %s", _c_file, _c_line, _c_fnc);
	std::ostringstream ss;
	ss << std::right << std::string(buffer);
	if (strcmp(_c_type, ""))
	    ss << std::right << std::setw(21) << _c_type;
	//std::string ret = std::string(buffer);
	return ss.str();
    }


    //-----------------------------------------------------------------------------------
    std::string format_mem_addr(void* _mem_addr, uint8_t _width)
    {
	std::ostringstream ss;
	ss << std::setw(_width) << std::setfill(' ') << std::right << "0x" << std::hex << (uint64_t)_mem_addr;
	return ss.str();
    }


}
