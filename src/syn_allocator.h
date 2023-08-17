
#ifndef __SYN_ALLOCATOR_H
#define __SYN_ALLOCATOR_H

#include <vector>
#include <list>
#include <map>
#include <unordered_map>

#include <memory>
#include <memory_resource>
#include <assert.h>
#include <cstddef>	// for std::max_align_t

#include <string>
#include <string.h>	// for memset
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <iomanip>

#ifdef __linux__
#include <malloc.h>
#endif
#ifdef WIN32
#include <Windows.h>
#endif


#define SYN_CORE_WARNING(x) std::cout << "WARNING: " << x << '\n';
#define SYN_CORE_TRACE(x) std::cout << x << '\n';

#define DEBUG_MEMORY_ALLOC
#define DEBUG_MEMORY_STL_ALLOC

#ifdef DEBUG_MEMORY_ALLOC
#define SYN_ASSERT(x) assert(x);
#else
#define SYN_ASSERT(x)
#endif



namespace Syn {

    // function pointer for malloc_size_func (linux) or _msize (windows)
    typedef size_t (*malloc_size_func_t)(void*);
    extern malloc_size_func_t malloc_size_func;


    // caller signature helper functions
    extern std::string pretty_func(const char* _fnc);
    extern std::string get_caller_signature(const char* _c_file, const char* _c_line, const char* _c_fnc, const char* _c_type="");
    extern std::string format_mem_addr(void* _mem_addr, uint8_t _width=16);


    //
    enum class AllocType
    {
	STL		 = 0,
	SHARED	 = 1,
	EXPLICIT = 2,
	NONE 	 = 3
    };

    static inline std::string AllocTypeStr(AllocType _alloc_type)
    {
	switch (_alloc_type)
	{
	case AllocType::STL:		return "AllocType::STL";
	case AllocType::SHARED:		return "AllocType::SHARED";
	case AllocType::EXPLICIT:	return "AllocType::EXPLICIT";
	case AllocType::NONE: 		return "AllocType::NONE";
	}
	return "AllocType::NONE";
    }


    /* 
     * Allocation info -- strictly for debugging.
     */
    struct memory_alloc_info
    {
	uint32_t m_allocBytes;		// raw bytes
	uint32_t m_allocBlock;		// block-aligned bytes
	uint32_t m_deallocBytes;
	uint32_t m_deallocBlock;
	AllocType m_allocType;
	std::string m_callerFnc;

	memory_alloc_info() : 
	    m_allocBytes(0), 
	    m_allocBlock(0),
	    m_deallocBytes(0), 
	    m_deallocBlock(0),
	    m_allocType(AllocType::NONE),
	    m_callerFnc("")
	{}

	memory_alloc_info(uint32_t _alloc_bytes,
			  uint32_t _alloc_block,
			  uint32_t _dealloc_bytes=0,
			  uint32_t _dealloc_block=0,
			  AllocType _alloc_type=AllocType::NONE,
			  const std::string& _caller_fnc="") :
	    m_allocBytes(_alloc_bytes), 
	    m_allocBlock(_alloc_block),
	    m_deallocBytes(_dealloc_bytes), 
	    m_deallocBlock(_dealloc_block),
	    m_allocType(_alloc_type),
	    m_callerFnc(_caller_fnc)
	{}
    };


    /*
     * Helper struct for the memory_log.
     */
    struct memory_usage
    {
	uint32_t m_physicalAlloc;
	uint32_t m_virtualAlloc;
	uint32_t m_physicalDealloc;
	uint32_t m_virtualDealloc;

	memory_usage() :
	    m_physicalAlloc(0), m_virtualAlloc(0),
	    m_physicalDealloc(0), m_virtualDealloc(0)
	{}

	inline void update_alloc(uint32_t _mem_bytes, uint32_t _mem_block) 
	{ 
	    m_physicalAlloc += _mem_bytes; 
	    m_virtualAlloc  += _mem_block;
	}
	inline void update_dealloc(uint32_t _mem_bytes, uint32_t _mem_block)
	{
	    m_physicalDealloc += _mem_bytes;
	    m_virtualDealloc  += _mem_block;
	}
    };


    /*
     * The memory tracking record.
     */
    class memory_log
    {
    public:
	// Insert a new allocation into record.
	static void insert(void* _mem_addr, uint32_t _alloc_bytes, uint32_t _alloc_block, AllocType _alloc_type, const std::string& _caller_fnc);
	// Remove (deallocation) an allocation from record.
	static void remove(void* _mem_addr, uint32_t _dealloc_bytes, uint32_t _dealloc_block, AllocType _alloc_type);

	// Heap usage accessors.
	static const memory_usage& get_usage_alloc_type(AllocType _alloc_type) { return s_usageType[(int)_alloc_type]; }
	static const memory_usage& get_usage_total() { return s_usageTotal; }		
	// Print memory allocations, sorted on AllocType.
	static const std::string& print_alloc_all(bool _omit_deallocated=true, bool _use_std_out=false);
	// Print memory allocations of AllocType _alloc_type.
	static std::string print_alloc_type(AllocType _alloc_type, bool _omit_deallocated);
	// Get allocated bytes at memory adress _mem_addr.
	static uint32_t get_alloc_bytes(void* _mem_addr);

	// Map of all allocated memory addresses and their size
	static const std::unordered_map<void*, memory_alloc_info>& get_memory() { return s_memory; } 

    private:
	// formatting of bytes into kb and mb
	static inline const std::string& _fmt_sz(uint32_t _bytes)
	{
	    static constexpr uint32_t mb = 1024 * 1024;
	    std::ostringstream ss;
	    if (_bytes <= 1024)	       ss << _bytes << " B";
	    else if (_bytes < mb)      ss << std::fixed << std::setprecision(2) << (float)_bytes / 1024.0f << " K";
	    else if (_bytes < 1024*mb) ss << std::fixed << std::setprecision(2) << (float)_bytes / (float)mb << " M";
	    else		       ss << std::fixed << std::setprecision(2) << (float)_bytes / ((float)mb * 1024.0f) << " G";
	    s_sz = ss.str();
	    return s_sz;
	}

    private:
	static std::unordered_map<void*, memory_alloc_info> s_memory;
	static memory_usage s_usageType[4];
	static memory_usage s_usageTotal;
	static std::string s_sz;
	static std::string s_lastLogEntry;
    };


    /* 
     * unordered_map memory insert and remove function pointers.
     */
    typedef void (*insert_func)(void*, uint32_t, uint32_t, AllocType, const std::string&);
    typedef void (*remove_func)(void*, uint32_t, uint32_t, AllocType);


    /*
     * Polymorfic resource override, using the global new/delete
     * memory resource (i.e. the global heap): new_delete_resource().
     */
    class MemoryResource : public std::pmr::memory_resource
    {
    public:
	MemoryResource() { m_memory = std::pmr::new_delete_resource(); }
	explicit MemoryResource(insert_func _insert_func,
				remove_func _remove_func,
				AllocType _malloc_type = AllocType::NONE,
				std::pmr::memory_resource* _memory=std::pmr::new_delete_resource())
	{
	    m_insertFunc = _insert_func; 
	    m_removeFunc = _remove_func;
	    m_allocType = _malloc_type;
	    m_memory = _memory;
	}
	// override the allocation and deallocation functions
	void* do_allocate(std::size_t _bytes, 
			  std::size_t _alignment=alignof(std::max_align_t)) override
	{
	    void* ptr = m_memory->allocate(_bytes, _alignment);
	    m_insertFunc(ptr, _bytes, malloc_size_func((void*)ptr), m_allocType, m_lastCaller);

	    // assert(m_allocType != AllocType::NONE);
	    return ptr;
	}

	void do_deallocate(void* _ptr, 
			   std::size_t _bytes, 
			   std::size_t _alignment=alignof(std::max_align_t)) override
	{
	    //assert(m_allocType != AllocType::NONE);
	    m_removeFunc(_ptr, _bytes, malloc_size_func(_ptr), m_allocType);
	    m_memory->deallocate(_ptr, _bytes, _alignment);
	}

	bool do_is_equal(const std::pmr::memory_resource& _other) const noexcept override { return this == &_other; }

	// set caller signature
	void set_caller_signature(const std::string& _caller_sig) { m_lastCaller = _caller_sig.c_str(); }


    private:
	// type of allocation, according to AllocType enum class.
	AllocType m_allocType;
	// function pointers to the insert and remove functions of the memory map
	insert_func m_insertFunc = nullptr;
	remove_func m_removeFunc = nullptr;
	// caller signature
	std::string m_lastCaller = "";
	// pointer to the global heap
	std::pmr::memory_resource* m_memory = nullptr;
    };

	
    template<typename T>
    using pmr_alloc = std::pmr::polymorphic_allocator<T>;
    using mem_rsrc = std::pmr::memory_resource;


    /* STL memory resource handler:
     * Everytime a Syn::vector<> is instantiated, a new MemoryResource is needed to
     * track reallocations for that vector. Since the memory is allocated before it's
     * copied when the vector is dynamically extended, the previuos address' signature
     * is unavailable. To circumvent this, a new MemoryResource is created everytime
     * a new vector is requested, a new MemoryResource is created and stored in this
     * handler. This will increase the memory footprint of the vector by 16 bytes;
     * 8 bytes for the MemoryResource pointer in the vector in the current class and 8
     * 8 bytes for the MemoryResource instance itself.
     * Upon application shutdown, the destructor handles deallocation of the heap-
     * allocated MemoryResource instances.
     */
    class STLMemoryResourceHandler
    {
    public:
	STLMemoryResourceHandler() {};
	~STLMemoryResourceHandler()
	{
	    for (auto i : m_rsrcs)
		delete i;
	}
	// return a new MemoryResource and out store address
	// for deallocation upon destruction of this.
	MemoryResource* getNewMemoryResource(insert_func _in_fnc=memory_log::insert, 
					     remove_func _rm_fnc=memory_log::remove, 
					     AllocType _alloc_type=AllocType::STL)
	{
	    MemoryResource* ptr = new MemoryResource(_in_fnc, _rm_fnc, _alloc_type);
	    m_rsrcs.push_back(ptr);
	    return ptr;
	}
	// return the memory footprint of this class and all
	// created pointers and the MemoryResource:s they point to.
	constexpr std::size_t getMemSize() const
	{ return sizeof(STLMemoryResourceHandler) + m_rsrcs.size() * (sizeof(MemoryResource*) + sizeof(MemoryResource)); }

    private:
	std::vector<MemoryResource*> m_rsrcs;

    };
    // global handler instance.
    extern STLMemoryResourceHandler s_STLMemRsrcHandler;


    /*
     * STL containers
     */
    extern MemoryResource* s_memorySTL;
	
#ifdef DEBUG_MEMORY_ALLOC
    /* 
     * templated aliases for STL container declaration e.g.
     * Syn::vector<T> = SYN_VECTOR(T);
     */
    template<typename T> 
    using vector = std::vector<T, pmr_alloc<T>>;
    template<typename T>
    using list = std::list<T, pmr_alloc<T>>;
    template<typename K, typename T>
    using map = std::map<K, T, std::less<K>, pmr_alloc<std::pair<const K, T>>>;
    template<typename K, typename T>
    using unordered_map = std::unordered_map<K, T, std::hash<K>, std::equal_to<K>, pmr_alloc<std::pair<const K, T>>>;

#ifdef DEBUG_MEMORY_STL_ALLOC
    // vector
    template<typename T>
    static inline vector<T> _syn_vector(const char* _c_file, const char* _c_line, const char* _c_fnc)
    {
	MemoryResource* rsrc = s_STLMemRsrcHandler.getNewMemoryResource();
	rsrc->set_caller_signature(get_caller_signature(_c_file, _c_line, _c_fnc, "Syn::vector<>"));
	vector<T> v(0, rsrc);
	return v;
    }
    // list
    template<typename T>
    static inline list<T> _syn_list(const char* _c_file, const char* _c_line, const char* _c_fnc)
    {
	MemoryResource* rsrc = s_STLMemRsrcHandler.getNewMemoryResource();
	rsrc->set_caller_signature(get_caller_signature(_c_file, _c_line, _c_fnc, "Syn::list<>"));
	list<T> l(rsrc);
	return l;
    }
    // map
    template<typename K, typename T>
    static inline map<K, T> _syn_map(const char* _c_file, const char* _c_line, const char* _c_fnc)
    {
	MemoryResource* rsrc = s_STLMemRsrcHandler.getNewMemoryResource();
	rsrc->set_caller_signature(get_caller_signature(_c_file, _c_line, _c_fnc, "Syn::map<>"));
	map<K, T> m(rsrc);
	return m;
    }
    // unordered_map
    template<typename K, typename T>
    static inline unordered_map<K, T> _syn_unordered_map(const char* _c_file, const char* _c_line, const char* _c_fnc)
    {
	MemoryResource* rsrc = s_STLMemRsrcHandler.getNewMemoryResource();
	rsrc->set_caller_signature(get_caller_signature(_c_file, _c_line, _c_fnc, "Syn::unordered_map<>"));
	unordered_map<K, T> um(rsrc);
	return um;
    }
#else // not DEBUG_MEMORY_STL_ALLOC
    template<typename T>
    static inline vector<T> _syn_vector()
    {
	vector<T> v(0, s_memorySTL);
	return v;
    }
    // list
    template<typename T>
    static inline list<T> _syn_list()
    {
	list<T> l(s_memorySTL);
	return l;
    }
    // map
    template<typename K, typename T>
    static inline map<K, T> _syn_map()
    {
	map<K, T> m(s_memorySTL);
	return m;
    }
    // unordered_map
    template<typename K, typename T>
    static inline unordered_map<K, T> _syn_unordered_map()
    {
	unordered_map<K, T> um(s_memorySTL);
	return um;
    }
#endif // DEBUG_MEMORY_STL_ALLOC
	
#else // not DEBUG_MEMORY -- Syn::container is just aliased for std::container
    template<typename T>
    using vector = std::vector<T>;
    template<typename T>
    using list = std::list<T>;
    template<typename K, typename T>
    using map = std::map<K, T>;
    template<typename K, typename T>
    using unordered_map = std::unordered_map<K, T>;
#endif


    /* Wrappers for std::allocate_shared<>.
     */

    // basic typedef used in Synapse
    template<typename T>
    using Ref = std::shared_ptr<T>;

    // the memory resource used for allocation
    extern MemoryResource* s_memoryAllocShared;

#ifdef DEBUG_MEMORY_ALLOC
    template<typename T>
    static inline std::shared_ptr<T> _shared_ptr(const char* _c_file, 
						 const char* _c_line, 
						 const char* _c_fnc)
    {
	s_memoryAllocShared->set_caller_signature(get_caller_signature(_c_file, _c_line, _c_fnc, "std::shared_ptr<>"));
	pmr_alloc<T> alloc(s_memoryAllocShared);
	std::shared_ptr<T> ptr = std::allocate_shared<T>(alloc);
	s_memoryAllocShared->set_caller_signature("");
	return ptr;
    }

    template<typename T, typename ...Args>
    static inline std::shared_ptr<T> _shared_ptr(const char* _c_file, 
						 const char* _c_line, 
						 const char* _c_fnc,
						 Args ...args)
    {
	s_memoryAllocShared->set_caller_signature(get_caller_signature(_c_file, _c_line, _c_fnc, "std::shared_ptr<>"));
	pmr_alloc<T> alloc(s_memoryAllocShared);
	std::shared_ptr<T> ptr = std::allocate_shared<T>(alloc, args...);
	s_memoryAllocShared->set_caller_signature("");
	return ptr;
    }
#endif

#ifndef DEBUG_MEMORY_ALLOC
    template<typename T, typename ...Args> static inline std::shared_ptr<T> shared_ptr(Args ...args) { return std::make_shared<T>(args...); }
    template<typename T> static inline std::shared_ptr<T> shared_ptr() { return std::make_shared<T>(); }
#endif


    /* Wrappers for explicit memory allocation.
     */
#ifdef DEBUG_MEMORY_ALLOC
	
    template<typename T, typename ...Args>
    static inline T* _allocate(const char* _c_file, const char* _c_line, const char* _c_fnc, Args ...args)
    { 
	T* ptr = new T(args...);
	void* void_ptr = reinterpret_cast<void*>(ptr);
	memory_log::insert(void_ptr, 
			   sizeof(T), 
			   malloc_size_func(void_ptr), 
			   AllocType::EXPLICIT, 
			   get_caller_signature(_c_file, _c_line, _c_fnc, "new(...)"));
	return ptr;
    }
	
    template<typename T>
    static inline T* _allocate(const char* _c_file, const char* _c_line, const char* _c_fnc)
    { 
	T* ptr = new T;
	void* void_ptr = reinterpret_cast<void*>(ptr);
	memory_log::insert(void_ptr, 
			   sizeof(T), 
			   malloc_size_func(void_ptr), 
			   AllocType::EXPLICIT,
			   get_caller_signature(_c_file, _c_line, _c_fnc, "new()"));
	return ptr;
    }

    template<typename T>
    static inline T* _allocate_n(const char* _c_file, const char* _c_line, const char* _c_fnc, const std::size_t& _n)
    { 
	T* ptr = new T[_n];
	void* void_ptr = reinterpret_cast<void*>(ptr);
	memory_log::insert(void_ptr, 
			   sizeof(T) * _n, 
			   malloc_size_func(void_ptr), 
			   AllocType::EXPLICIT,
			   get_caller_signature(_c_file, _c_line, _c_fnc, "new[]"));
	return ptr;
    }

    // template<typename T>
    // static inline void _deallocate_n(T* _ptr)
    static inline void _deallocate(void *_ptr)
    {
	// void* void_ptr = reinterpret_cast<void*>(_ptr);
	memory_log::remove(_ptr, memory_log::get_alloc_bytes(_ptr), malloc_size_func(_ptr), AllocType::EXPLICIT);
	// memory_log::remove(void_ptr, memory_log::get_alloc_bytes(void_ptr), malloc_size_func(void_ptr), AllocType::EXPLICIT);
	::operator delete(_ptr);
    }

    // template<typename T>
    // static inline void _deallocate_n(T* _ptr)
    static inline void _deallocate_n(void *_ptr)
    {
	// void* void_ptr = reinterpret_cast<void*>(_ptr);
	memory_log::remove(_ptr, memory_log::get_alloc_bytes(_ptr), malloc_size_func(_ptr), AllocType::EXPLICIT);
        // memory_log::remove(void_ptr, memory_log::get_alloc_bytes(void_ptr), malloc_size_func(void_ptr), AllocType::EXPLICIT);
	::operator delete[](_ptr); 
    }
#else
    template<typename T, typename ...Args> static inline T* allocate(Args ...args) { return new T(args...); }
    template<typename T> static inline T* allocate() { return new T; }
    template<typename T> static inline T* allocate_n(const std::size_t& _n) {  return new T[_n]; }
    // template<typename T> static inline void deallocate(T* _ptr) { delete _ptr; }
    // template<typename T> static inline void deallocate_n(T* _ptr) { delete[] _ptr; }
    static inline void deallocate(void *_ptr) { delete _ptr; }
    static inline void deallocate_n(void *_ptr) { delete[] _ptr; }
#endif


} // namespace Syn

#define STR(x) #x
#define TOSTR(x) STR(x)

#ifdef __linux__
#define FUNCSIG Syn::pretty_func(__PRETTY_FUNCTION__).c_str()
#elif defined(_WIN32)
#define FUNCSIG __FUNCSIG__
#endif

// macros for creating STL containers
//
#ifdef DEBUG_MEMORY_ALLOC
#ifdef DEBUG_MEMORY_STL_ALLOC
#define SYN_VECTOR(T) 			Syn::_syn_vector<T>(__FILE__, TOSTR(__LINE__), FUNCSIG)
#define SYN_LIST(T) 			Syn::_syn_list<T>(__FILE__, TOSTR(__LINE__), FUNCSIG)
#define SYN_MAP(K, T) 			Syn::_syn_map<K, T>(__FILE__, TOSTR(__LINE__), FUNCSIG)
#define SYN_UNORDERED_MAP(K, T)         Syn::_syn_unordered_map<K, T>(__FILE__, TOSTR(__LINE__), FUNCSIG)
#else
#define SYN_VECTOR(T) 			Syn::_syn_vector<T>()
#define SYN_LIST(T) 			Syn::_syn_list<T>()
#define SYN_MAP(K, T) 			Syn::_syn_map<K, T>()
#define SYN_UNORDERED_MAP(K, T) Syn::_syn_unordered_map<K, T>()
#endif
#else
#define SYN_VECTOR(T) 			std::vector<T>()
#define SYN_LIST(T)			std::list<T>()
#define SYN_MAP(K, T)			std::map<K, T>()
#define SYN_UNORDERED_MAP(K, T)		std::unordered_map<K, T>()
#endif

// explicit, global allocation macros
//
#ifdef DEBUG_MEMORY_ALLOC
#define SYN_ALLOCATE(T, ...)		 Syn::_allocate<T>(__FILE__, TOSTR(__LINE__), FUNCSIG, ##__VA_ARGS__)
#define SYN_ALLOCATE_N(T, ...) 		 Syn::_allocate_n<T>(__FILE__, TOSTR(__LINE__), FUNCSIG, ##__VA_ARGS__)
#define SYN_DEALLOCATE(mem_addr)         Syn::_deallocate(mem_addr);
#define SYN_DEALLOCATE_N(mem_addr)       Syn::_deallocate_n(mem_addr)
// #define SYN_DEALLOCATE(mem_addr)   Syn::_deallocate<decltype(mem_addr)>(mem_addr);
// #define SYN_DEALLOCATE_N(mem_addr) Syn::_deallocate_n<decltype(mem_addr)>(mem_addr)
#else
#define SYN_ALLOCATE(T, ...) 		  new T(__VA_ARGS__)
#define SYN_ALLOCATE_N(T, n) 		  new T[n]
#define SYN_DEALLOCATE(mem_addr)          ::operator delete(mem_addr);
#define SYN_DEALLOCATE_N(mem_addr)        ::operator delete[](mem_addr)
#endif

// macros for creating a alloc-tracked std::shared_ptr
//
#ifdef DEBUG_MEMORY_ALLOC
#define SYN_MAKE_REF(T, ...) Syn::_shared_ptr<T>(__FILE__, TOSTR(__LINE__), FUNCSIG, ##__VA_ARGS__)
#else
#define SYN_MAKE_REF(T, ...) std::make_shared<T>(##__VA_ARGS__)
#endif


#endif // __SYN_ALLOCATOR_H

