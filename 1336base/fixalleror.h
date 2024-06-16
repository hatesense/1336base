// obf
#include "hooker/typesforblacks.h"
#	define ASSERT_ONLY(...)
#	define Assert(x)
#	define Assertf(x,fmt,...)
#	define FastAssert(x)
#	define FatalAssert(x)
#	define FatalAssertf(x,fmt,...)
#	define FatalAssertMsg(x,msg)
#	define AssertMsg(x,msg)
#	define AssertVerify(x) (x)
#	define Verifyf(x,fmt,...) (x)
#	define DebugAssert(x)
#define rage_new					new
#define rage_aligned_new(a)			new((size_t)(a))
#define rage_alignof_new(a)			new(__alignof(a))
#define rage_heap_new(h)			new(h)
#define rage_heap_aligned_new(h,a)	new((h),(size_t)(a))
#define rage_placement_new(p)		::new(p)

// made by vali x cameron x johnathan
namespace rage {
	namespace sysObfuscatedTypes {
		// Obfuscated rand() function. Essentially the same as rand(), but inline, and generates
		// 32-bit values. Not thread safe, it doesn't need to be - we're not storing any meaningful
		// state here.
		__forceinline u32 obfRand()
		{
			static u32 next = 0xCB536E6A;
			next = next * 214013 + 2531011;
			return next;
		}
	}

	// This template class obfuscates the contained data in memory, by XORing it with a magic number
	// and the address of the data itself. Make sure you do NOT perform a bitwise copy of this type!
	// The type contained in a sysObfuscated does not need to be a POD type, with two caveats:
	// 1.	If external code tries to access the contained object through its 'this' pointer, the
	//		external code will access the obfuscated value and the code will likely crash.
	// 2.	The copy constructor and operator= will not be called in a standards compliant manner. In
	//		particular, the copy constructor may be called with a right-hand-side argument that is not
	//		an object that has had a constructor run on it, but is still a valid object, although it
	//		will only be aligned to 4 bytes (Default alignment for u32).
	//
	// Template arguments:
	// T:		The type being obfuscated, e.g. u32, float. The type must be a multiple of 32 bits in
	//			size.
	// TMutate:	If true, then the class will be 2x the size of the base type, and will be further
	//			mangled in memory. Every time the data is read, the class representation will change in
	//			memory, making it difficult to identify real changes with data breakpoints. This has a
	//			performance impact compared to using a non-mutated type.
#define CompileTimeAssert(data)
	template<class T, bool TMutate = true> class sysObfuscated
	{
	public:
		// PURPOSE:	Constructor
		// PARAMS:	rhs - Right-hand side argument for copy constructor
		// PARAMS:	data - Data to initialise with for copy constructor
		sysObfuscated() { CompileTimeAssert((sizeof(T) & 3) == 0); Init(); }
		sysObfuscated(const sysObfuscated<T, TMutate>& rhs) { Init(); Set(rhs.Get()); }
		template<bool TMutateOther> sysObfuscated(const sysObfuscated<T, TMutateOther>& rhs) { Init(); Set(rhs.Get()); }
		explicit sysObfuscated(const T& data) { Init(); Set(data); }

		// PURPOSE:	Destructor
		~sysObfuscated() {}

		// PURPOSE:	Access the unobfuscated value contained
		// RETURNS:	The unobfuscated data.
		__forceinline T Get() const;

		// PURPOSE:	Set the value contained
		// PARAMS:	data - Unobfuscated value to set
		__forceinline void Set(const T& data);

		// PURPOSE:	Implicit conversion operator
		__forceinline operator T() const { return Get(); }

		// PURPOSE:	Comparison against other sysObfuscated's
		// PARAMS:	rhs - Object to compare against
		// NOTES:	Templated to allow you to compare sysObfuscated's with different mutation values
		template<bool TMutateOther> bool operator==(const sysObfuscated<T, TMutateOther>& rhs) const { return Get() == rhs.Get(); }
		template<bool TMutateOther> bool operator!=(const sysObfuscated<T, TMutateOther>& rhs) const { return Get() != rhs.Get(); }
		template<bool TMutateOther> bool operator<(const sysObfuscated<T, TMutateOther>& rhs) const { return Get() < rhs.Get(); }
		template<bool TMutateOther> bool operator<=(const sysObfuscated<T, TMutateOther>& rhs) const { return Get() <= rhs.Get(); }
		template<bool TMutateOther> bool operator>(const sysObfuscated<T, TMutateOther>& rhs) const { return Get() > rhs.Get(); }
		template<bool TMutateOther> bool operator>=(const sysObfuscated<T, TMutateOther>& rhs) const { return Get() >= rhs.Get(); }

		// PURPOSE:	Assignment operator
		// PARAMS:	rhs - Object to assign
		// PARAMS:	data - Value to assign
		template<bool TMutateOther> sysObfuscated<T, TMutate>& operator=(const sysObfuscated<T, TMutateOther>& rhs) { Set(rhs.Get()); return *this; }
		sysObfuscated<T, TMutate>& operator=(const T& data) { Set(data); return *this; }

		// PURPOSE:	Misc. operators
		// PARAMS:	rhs - Object to assign
		// PARAMS:	data - Value to assign
		template<bool TMutateOther> sysObfuscated<T, TMutate>& operator+=(const sysObfuscated<T, TMutateOther>& rhs) { Set(Get() + rhs.Get()); return *this; }
		sysObfuscated<T, TMutate>& operator+=(const T& data) { Set(Get() + data); return *this; }
		template<bool TMutateOther> sysObfuscated<T, TMutate>& operator-=(const sysObfuscated<T, TMutateOther>& rhs) { Set(Get() - rhs.Get()); return *this; }
		sysObfuscated<T, TMutate>& operator-=(const T& data) { Set(Get() - data); return *this; }
		template<bool TMutateOther> sysObfuscated<T, TMutate>& operator*=(const sysObfuscated<T, TMutateOther>& rhs) { Set(Get() * rhs.Get()); return *this; }
		sysObfuscated<T, TMutate>& operator*=(const T& data) { Set(Get() * data); return *this; }
		template<bool TMutateOther> sysObfuscated<T, TMutate>& operator/=(const sysObfuscated<T, TMutateOther>& rhs) { Set(Get() / rhs.Get()); return *this; }
		sysObfuscated<T, TMutate>& operator/=(const T& data) { Set(Get() / data); return *this; }

	private:
		// Initialise m_xor and m_mutate to random values
		void Init();

		// The obfuscated data
		mutable u32 m_data[(TMutate ? sizeof(T) * 2 : sizeof(T)) / sizeof(u32)];

		// XOR and mutate keys for this type
		mutable u32 m_xor;
		mutable u32 m_mutate;
	};

	template<class T, bool TMutate> __forceinline void sysObfuscated<T, TMutate>::Init()
	{
		m_xor = sysObfuscatedTypes::obfRand();
		if (TMutate)
		{
			m_mutate = sysObfuscatedTypes::obfRand();
		}
	}

	// While these functions look inefficient, they optimise well
	template<class T, bool TMutate> __forceinline T sysObfuscated<T, TMutate>::Get() const
	{
		u32 xorVal = m_xor ^ (u32)(size_t)this;
		u32 ret[sizeof(T) / sizeof(u32)];
		u32* src = const_cast<u32*>(&m_data[0]);
		u32* dest = (u32*)&ret;
		for (size_t i = 0; i < sizeof(T) / 4; ++i)
		{
			if (TMutate)
			{
				// Extract valid data from two words of storage
				u32 a = *src & m_mutate;
				u32 b = src[sizeof(T) / 4] & (~m_mutate);

				// Apply entropy in the unused bits: Just flip the two u16's in the u32. We can't do a
				// huge amount more without knowledge of the mutation mask.
				u32 entropyA = ((*src & (~m_mutate)) << 16) | ((*src & (~m_mutate)) >> 16);
				u32 entropyB = ((src[sizeof(T) / 4] & m_mutate) << 16) | ((src[sizeof(T) / 4] & m_mutate) >> 16);
				*src = (*src & m_mutate) | entropyA;
				src[sizeof(T) / 4] = (src[sizeof(T) / 4] & (~m_mutate)) | entropyB;

				*dest++ = a | b;
				++src;
			}
			else
			{
				*dest++ = *src++ ^ xorVal;
			}
		}

		// Call Set() to reset the xor and mutate keys on every call to Get()
		if (TMutate)
		{
			const_cast<sysObfuscated<T, TMutate>*>(this)->Set(*(T*)&ret);
		}

		return *(T*)&ret;
	}

	template<class T, bool TMutate> __forceinline void sysObfuscated<T, TMutate>::Set(const T& data)
	{
		// Reset xor and mutate keys
		Init();

		u32 xorVal = m_xor ^ (u32)(size_t)this;
		u32* src = (u32*)&data;
		u32* dest = &m_data[0];
		for (size_t i = 0; i < sizeof(T) / 4; ++i)
		{
			if (TMutate)
			{
				u32 a = *src & m_mutate;
				u32 b = *src & (~m_mutate);
				++src;

				*dest = a;
				dest[sizeof(T) / 4] = b;
				++dest;
			}
			else
			{
				*dest++ = *src++ ^ xorVal;
			}
		}
	}

	//-------------------------------------------------------------------------------------------------

	// This template class keeps track of multiple copies of a variable, and detects if they go out of
	// sync, such as by someone modifying the value in memory. When this happens, the tamper callback
	// is called.
	// Template arguments:
	// T:			The type being obfuscated, e.g. u32, float, sysObfuscated<u32>.
	// TamperCB:	The tamper callback invoked when a mismatch is detected
	template<class T, void (*TamperCB)(u32, u32)> class sysLinkedData
	{
	public:
		// PURPOSE:	Constructor
		// PARAMS:	rhs - Right-hand side argument for copy constructor
		// PARAMS:	data - Data to initialise with for copy constructor
		sysLinkedData() { InitDefaultLinks(); }
		sysLinkedData(const sysLinkedData<T, TamperCB>& rhs) { InitDefaultLinks(); Set(rhs.Get()); }
		explicit sysLinkedData(const T& data) : m_data(data) { InitDefaultLinks(); }

		// PURPOSE:	Destructor
		~sysLinkedData();

		// PURPOSE:	Access the value contained and check all links for tampering
		// RETURNS:	The raw data.
		__forceinline const T& Get() const;

		// PURPOSE:	Set the value contained on all linked entries
		// PARAMS:	data - Value to set
		__forceinline void Set(const T& data);

		// PURPOSE:	Add a link to another sysLinkedData
		// PARAMS:	pOther - sysLinkedData to link
		// NOTES:	pOther's data will be set to the data contained by this instance
		__forceinline void AddLink(sysLinkedData<T, TamperCB>* pOther);

		// PURPOSE:	Remove a link to another sysLinkedData
		// PARAMS:	pOther - sysLinkedData to remove
		// NOTES:	Does nothing if pOther is not linked to this.
		__forceinline void RemoveLink(sysLinkedData<T, TamperCB>* pOther);

		// PURPOSE:	Walk the list of linked items and return the number of dummy links
		// NOTES: Should only really be used for debugging.
		int GetNumDummyLinks() const;

		// PURPOSE:	Walk the list of linked items and return the number of non-dummy links
		// NOTES: Should only really be used for debugging.
		int GetNumNonDummyLinks() const;

		// PURPOSE:	Walk the list of linked items and return the number of links
		// NOTES: Should only really be used for debugging.
		int GetNumLinks() const;

		// PURPOSE:	Implicit conversion operator
		operator T() const { return Get(); }

		// PURPOSE:	Comparison against other sysLinkedData's
		// PARAMS:	rhs - Object to compare against
		bool operator==(const sysLinkedData<T, TamperCB>& rhs) const { return Get() == rhs.Get(); }
		bool operator!=(const sysLinkedData<T, TamperCB>& rhs) const { return Get() != rhs.Get(); }
		bool operator<(const sysLinkedData<T, TamperCB>& rhs) const { return Get() < rhs.Get(); }
		bool operator<=(const sysLinkedData<T, TamperCB>& rhs) const { return Get() <= rhs.Get(); }
		bool operator>(const sysLinkedData<T, TamperCB>& rhs) const { return Get() > rhs.Get(); }
		bool operator>=(const sysLinkedData<T, TamperCB>& rhs) const { return Get() >= rhs.Get(); }

		bool operator==(const T& rhs) const { return Get() == rhs; }
		bool operator!=(const T& rhs) const { return Get() != rhs; }
		bool operator<(const T& rhs) const { return Get() < rhs; }
		bool operator<=(const T& rhs) const { return Get() <= rhs; }
		bool operator>(const T& rhs) const { return Get() > rhs; }
		bool operator>=(const T& rhs) const { return Get() >= rhs; }

		// PURPOSE:	Assignment operator
		// PARAMS:	rhs - Object to assign
		// PARAMS:	data - Value to assign
		sysLinkedData<T, TamperCB>& operator=(const sysLinkedData<T, TamperCB>& rhs) { Set(rhs.Get()); return *this; }
		sysLinkedData<T, TamperCB>& operator=(const T& data) { Set(data); return *this; }

		sysLinkedData<T, TamperCB>& operator-=(const float rhs) { Set(this->Get() - rhs); return *this; }
		sysLinkedData<T, TamperCB>& operator+=(const float rhs) { Set(this->Get() + rhs); return *this; }
	private:
		// Private c'tor and helper function for creating dummy links
		sysLinkedData(const T& data, u32 nonce) : m_data(data), m_isDummyLink(nonce) {}
		void InitDefaultLinks();

		// See if this is a dummy link
		u32 IsDummyLink() const { return m_isDummyLink & s_isDummyLinkMask; }

		T m_data;

		// Double-linked circular list
		sysLinkedData<T, TamperCB>* m_pPrev;
		sysLinkedData<T, TamperCB>* m_pNext;

		// If this mask is set, this is a dummy link and can be deleted when the last non-dummy link is destroyed
		static const u32 s_isDummyLinkMask = 3 << 3;
		u32 m_isDummyLink;
	};

	template<class T, void (*TamperCB)(u32, u32)>
	__forceinline sysLinkedData<T, TamperCB>::~sysLinkedData()
	{
		if (IsDummyLink())
		{
			// Dummy links don't run the full, unchaining destructor. They only get destroyed when the last
			// non-dummy link gets destroyed, so we don't care about properly unlinking everything, just
			// freeing memory.
			return;
		}

		// Is this the last non-dummy link in the chain?
		sysLinkedData<T, TamperCB>* p = m_pNext;
		bool allDummyLinks = true;
		while (p != this)
		{
			if (p->IsDummyLink() == 0)
			{
				allDummyLinks = false;
				break;
			}
			p = p->m_pNext;
		}

		// If every other link is a dummy link, delete the dummy links. NB: Walk through the prev pointer
		// rather than the next pointer as additional obfuscation, but also so memory is freed in the
		// reverse of the order it was acquired in, which might be easier on the allocator.
		if (allDummyLinks)
		{
			p = m_pPrev;
			while (p != this)
			{
				sysLinkedData<T, TamperCB>* pPrev = p->m_pPrev;
				delete p;
				p = pPrev;
			}
			return;
		}

		// There are non-dummy links still in the chain, just remove ourself from the chain
		m_pPrev->RemoveLink(this);
	}

	template<class T, void (*TamperCB)(u32, u32)>
	__forceinline const T& sysLinkedData<T, TamperCB>::Get() const
	{
		// Check integrity
		bool reported = false;
		sysLinkedData<T, TamperCB>* p = m_pNext;
		while (p != this)
		{
			if (p->m_data != m_data)
			{
				if (!reported)
				{
					// Now, let's pass the difference between the two
					// Cast it as a u32, and leave it up to the function
					// utilising it to cast it to the correct type
					TamperCB((u32)m_data, (u32)p->m_data);
					reported = true;
				}
				// Set all the links to the new value so that we only report new changes.
				p->m_data = m_data;
			}
			p = p->m_pNext;
		}
		return m_data;
	}

	template<class T, void (*TamperCB)(u32, u32)>
	__forceinline void sysLinkedData<T, TamperCB>::Set(const T& data)
	{
		sysLinkedData<T, TamperCB>* p = m_pNext;
		while (p != this)
		{
			p->m_data = data;
			p = p->m_pNext;
		}
		m_data = data;
	}

	template<class T, void (*TamperCB)(u32, u32)>
	__forceinline void sysLinkedData<T, TamperCB>::AddLink(sysLinkedData<T, TamperCB>* pOther)
	{
		// Check entry isn't already in list
		Assertf(pOther != this, "Can't add link to ourself");
		sysLinkedData<T, TamperCB>* p = m_pNext;
		while (p != this)
		{
			if (p == pOther)
				return;
			p = p->m_pNext;
		}

		// Set data in all new links
		pOther->Set(m_data);

		// Add link
		pOther->m_pPrev->m_pNext = m_pNext;
		m_pNext->m_pPrev = pOther->m_pPrev;
		m_pNext = pOther;
		pOther->m_pPrev = this;

		// Keep the number of dummy links in the list at a sensible number
		int maxLinksInSystem = 17 + (sysObfuscatedTypes::obfRand() & 15); // rand between 17..32
		int linksToCull = GetNumDummyLinks() - maxLinksInSystem;
		p = m_pNext;
		while (linksToCull > 0)
		{
			// Find next dummy link
			while (!p->IsDummyLink())
			{
				p = p->m_pNext;
			}

			// Unlink it
			p->m_pNext->m_pPrev = p->m_pPrev;
			p->m_pPrev->m_pNext = p->m_pNext;

			// Delete it. NB: It's safe to just delete it because dummy links don't do any unlinking
			// in their destructors.
			sysLinkedData<T, TamperCB>* pNext = p->m_pNext;
			delete p;
			p = pNext;
			--linksToCull;
		}
	}

	template<class T, void (*TamperCB)(u32, u32)>
	__forceinline void sysLinkedData<T, TamperCB>::RemoveLink(sysLinkedData<T, TamperCB>* pOther)
	{
		// Find entry in list
		Assertf(pOther != this, "Can't remove link to ourself");
		sysLinkedData<T, TamperCB>* p = m_pNext;
		while (p != pOther)
		{
			if (p == this)
				return; // Looped round entire list
			p = p->m_pNext;
		}

		// Remove link
		pOther->m_pNext->m_pPrev = pOther->m_pPrev;
		pOther->m_pPrev->m_pNext = pOther->m_pNext;
		pOther->m_pPrev = pOther;
		pOther->m_pNext = pOther;
	}

	template<class T, void (*TamperCB)(u32, u32)>
	__forceinline void sysLinkedData<T, TamperCB>::InitDefaultLinks()
	{
		// Initialise our m_isDummyLink member
		m_isDummyLink = ((u32)(size_t)this << 3) & (~s_isDummyLinkMask);

		// Number of links to create by default. Create more links for small types (pointer or smaller)
		static const u32 numLinks = sizeof(T) <= sizeof(void*) ? 12 : 4;
		sysLinkedData<T, TamperCB>* pLast = this;
		for (u32 i = 0; i < numLinks; ++i)
		{
			// Make a nonce value for the dummy link. Doesn't matter what it is, so long as the
			// s_isDummyLinkMask mask is set.
			u32 nonce = m_isDummyLink | s_isDummyLinkMask | 3;
			sysLinkedData<T, TamperCB>* pNewLink = rage_new sysLinkedData<T, TamperCB>(m_data, nonce);

			// Init forward / back pointers
			pNewLink->m_pPrev = pLast;
			pLast->m_pNext = pNewLink;
			pLast = pNewLink;
		}

		// Join the ends of the circular list
		m_pPrev = pLast;
		pLast->m_pNext = this;
	}

	template<class T, void (*TamperCB)(u32, u32)>
	__forceinline int sysLinkedData<T, TamperCB>::GetNumDummyLinks() const
	{
		sysLinkedData<T, TamperCB>* p = m_pNext;
		int ret = IsDummyLink() ? 1 : 0;
		while (p != this)
		{
			if (p->IsDummyLink())
				++ret;
			p = p->m_pNext;
		}
		return ret;
	}

	template<class T, void (*TamperCB)(u32, u32)>
	__forceinline int sysLinkedData<T, TamperCB>::GetNumNonDummyLinks() const
	{
		sysLinkedData<T, TamperCB>* p = m_pNext;
		int ret = IsDummyLink() ? 0 : 1;
		while (p != this)
		{
			if (!p->IsDummyLink())
				++ret;
			p = p->m_pNext;
		}
		return ret;
	}

	template<class T, void (*TamperCB)(u32, u32)>
	__forceinline int sysLinkedData<T, TamperCB>::GetNumLinks() const
	{
		sysLinkedData<T, TamperCB>* p = m_pNext;
		int ret = 1;
		while (p != this)
		{
			++ret;
			p = p->m_pNext;
		}
		return ret;
	}
	//------------------------------------------------------------------------------------------
	// Simple struct to hold obfuscated/non-obfuscated data along with node pointers
	// Serves as building blocks for the actual Linked List object
	template <class T> struct sysObfuscatedLinkedDataNode
	{
		sysObfuscated<T, false> m_data;
		T m_dummy;
		sysObfuscatedLinkedDataNode<T>* m_pPrev;
		sysObfuscatedLinkedDataNode<T>* m_pNext;
	};

	/* sysObfuscatedLinkedData Class
	 * Holds linked list that has synchronized data in both obfuscated and unobfuscated form
	 * The head can be considered the data being used, while the data in all the other nodes
	 * can be considered a shadow copy of the head.
	 * Currently, thread-safety is delegated to the function using this class instead of
	 * being implemented in the class itself. If we do decide to make this class independently
	 * thread-safe, consider adding a mutex member in this class along with Lock/Unlock methods.
	 */
	template <class T, void (*TamperCB)(u32, u32)> class sysObfuscatedLinkedData
	{
	public:
		// PURPOSE: Default Constructor
		// NOTES: Sets no initial link, length = 0
		sysObfuscatedLinkedData();

		// PURPOSE: Initialized Constructor
		// PARAMS: data - value of the variable to be protected
		// NOTES: Creates with 1 initial link, length = 1
		explicit sysObfuscatedLinkedData(const T& data)
		{
			length = 0;
			tripped = false;
			head = NULL;
			AddLink();
			Set(data);
		}

		// PURPOSE: Destructor
		// NOTES: Destroys and frees any remaining links
		~sysObfuscatedLinkedData();

		// PURPOSE: Retrieves protected value
		// OUTPUTS: The protected value
		// NOTES: The value comes from the obfuscated m_data member of the first node
		__forceinline T Get() const;

		// PURPOSE: Sets protected value to a new value
		// PARAMS: data - value to be written
		// NOTES:	If tripped flag = true, does not write value to
		//			unprotected m_dummy values in each link
		__forceinline void Set(const T& data);

		// PURPOSE: Checks the integrity of each link to the start of the list
		// NOTES:	If integrity is suspected to be compromised, sets the tripped
		//			flag to true
		__forceinline void Check();

		// PURPOSE:	Resets the linked list to a synchronized state
		// NOTES:	Also sets tripped = false
		__forceinline void Reset();

		// PURPOSE:	Resizes the linked list to desired length
		// PARAMS:	newLength - desired length of linked list
		// NOTES:	Use this instead of the AddLink/RemoveLink methods,
		//			since Resize automatically takes care of memory allocation
		//			of individual nodes.
		//			When removing excess nodes, it always removes the head
		//			so that the head is rotated along the linked list instead
		//			of being static.
		__forceinline void Resize(size_t newLength);

		// PURPOSE:	Returns the current length of the linked list
		// OUTPUTS:	The current length of the linked list
		__forceinline size_t GetLength() { return length; }

		// PURPOSE:	Implicit conversion operator
		// OUTPUTS:	The protected value
		operator T() const { return Get(); }

		// PURPOSE:	Comparison against other sysObfuscatedLinkedData's
		// PARAMS:	rhs - Object to compare against
		bool operator==(const sysObfuscatedLinkedData<T, TamperCB>& rhs) const { return Get() == rhs.Get(); }
		bool operator!=(const sysObfuscatedLinkedData<T, TamperCB>& rhs) const { return Get() != rhs.Get(); }
		bool operator<(const sysObfuscatedLinkedData<T, TamperCB>& rhs) const { return Get() < rhs.Get(); }
		bool operator<=(const sysObfuscatedLinkedData<T, TamperCB>& rhs) const { return Get() <= rhs.Get(); }
		bool operator>(const sysObfuscatedLinkedData<T, TamperCB>& rhs) const { return Get() > rhs.Get(); }
		bool operator>=(const sysObfuscatedLinkedData<T, TamperCB>& rhs) const { return Get() >= rhs.Get(); }

		bool operator==(const T& rhs) const { return Get() == rhs; }
		bool operator!=(const T& rhs) const { return Get() != rhs; }
		bool operator<(const T& rhs) const { return Get() < rhs; }
		bool operator<=(const T& rhs) const { return Get() <= rhs; }
		bool operator>(const T& rhs) const { return Get() > rhs; }
		bool operator>=(const T& rhs) const { return Get() >= rhs; }

		sysObfuscatedLinkedData<T, TamperCB>& operator=(const sysObfuscatedLinkedData<T, TamperCB>& rhs) { Set(rhs.Get()); return *this; }
		sysObfuscatedLinkedData<T, TamperCB>& operator=(const T& data) { Set(data); return *this; }

		sysObfuscatedLinkedData<T, TamperCB>& operator-=(const float rhs) { Set(this->Get() - rhs); return *this; }
		sysObfuscatedLinkedData<T, TamperCB>& operator+=(const float rhs) { Set(this->Get() + rhs); return *this; }

	private:
		size_t length;
		bool tripped;
		sysObfuscatedLinkedDataNode<T>* head;

		// PURPOSE:	Adds a link to the linked list
		// NOTES:	If there is an existing link, it initializes the 
		//			values in the new link to the existing one.
		//			Else, the new node becomes the head of the linked list
		//			but the values are not initialized.
		__forceinline void AddLink();

		// PURPOSE:	Removes the head node of the linked list
		// NOTES:	If this is the last link of the list, sets head to NULL
		__forceinline void RemoveLink();
	};

	template <class T, void (*TamperCB)(u32, u32)>
	sysObfuscatedLinkedData<T, TamperCB>::sysObfuscatedLinkedData()
	{
		length = 0;
		tripped = false;
		head = NULL;
	}

	template <class T, void (*TamperCB)(u32, u32)>
	sysObfuscatedLinkedData<T, TamperCB>::~sysObfuscatedLinkedData()
	{
		for (size_t i = 0; i < length; ++i)
		{
			RemoveLink();
		}
		Assertf(length == 0, "Error: sysObfuscatedLinkedData destructor failed to free every link");
	}

	template <class T, void (*TamperCB)(u32, u32)>
	__forceinline T sysObfuscatedLinkedData<T, TamperCB>::Get() const
	{
		return head->m_data.Get();
	}

	template <class T, void (*TamperCB)(u32, u32)>
	__forceinline void sysObfuscatedLinkedData<T, TamperCB>::Set(const T& data)
	{
		if (length == 0)
		{
			AddLink();
		}
		sysObfuscatedLinkedDataNode<T>* current = head;
		for (size_t i = 0; i < length; ++i)
		{
			current->m_data.Set(data);
			if (!tripped)
			{
				current->m_dummy = data;
			}
			current = current->m_pNext;
		}
	}

	template <class T, void (*TamperCB)(u32, u32)>
	__forceinline void sysObfuscatedLinkedData<T, TamperCB>::Check()
	{
		// Only report when linked data is first compromised
		if (tripped)
		{
			return;
		}

		sysObfuscatedLinkedDataNode<T>* current = head;
		for (size_t i = 0; i < length; ++i)
		{
			// The obfuscated data does not match the dummy data
			if (current->m_data.Get() != current->m_dummy)
			{
				tripped = true;
				TamperCB((u32)current->m_data.Get(), (u32)current->m_dummy);
				return;
			}

			// The obfuscated data of current node does not match that of the head
			if (current->m_data.Get() != head->m_data.Get())
			{
				tripped = true;
				TamperCB((u32)current->m_data.Get(), (u32)head->m_data.Get());
				Reset();
				return;
			}

			// The dummy data of the current node does not match that of the head
			if (current->m_dummy != head->m_dummy)
			{
				tripped = true;
				TamperCB((u32)current->m_dummy, (u32)head->m_dummy);
				return;
			}
		}
	}

	template <class T, void (*TamperCB)(u32, u32)>
	__forceinline void sysObfuscatedLinkedData<T, TamperCB>::Reset()
	{
		if (head == NULL)
		{
			return;
		}

		sysObfuscatedLinkedDataNode<T>* current = head;
		for (size_t i = 0; i < length; ++i)
		{
			current->m_data.Set(head->m_data.Get());
			current->m_dummy = head->m_dummy;
			current = current->m_pNext;
		}
		tripped = false;
	}

	template <class T, void (*TamperCB)(u32, u32)>
	__forceinline void sysObfuscatedLinkedData<T, TamperCB>::Resize(size_t newLength)
	{
		if (length < newLength)
		{
			while (length < newLength)
			{
				AddLink();
			}
		}
		else if (length > newLength)
		{
			while (length > newLength)
			{
				RemoveLink();
			}
		}
		else
		{
			return;
		}
	}

	template <class T, void (*TamperCB)(u32, u32)>
	__forceinline void sysObfuscatedLinkedData<T, TamperCB>::AddLink()
	{
		sysObfuscatedLinkedDataNode<T>* newNode = rage_new sysObfuscatedLinkedDataNode<T>();
		if (length == 0)
		{
			newNode->m_pPrev = newNode;
			newNode->m_pNext = newNode;
			head = newNode;
		}
		else
		{
			newNode->m_data.Set(head->m_data.Get());
			newNode->m_dummy = head->m_dummy;

			// Link the new node at end of list (directly behind current head)
			newNode->m_pPrev = head->m_pPrev;
			newNode->m_pNext = head;
			head->m_pPrev->m_pNext = newNode;
			head->m_pPrev = newNode;
		}

		++length;
	}

	template <class T, void (*TamperCB)(u32, u32)>
	__forceinline void sysObfuscatedLinkedData<T, TamperCB>::RemoveLink()
	{
		Assertf((head != NULL) && (length > 0), "Warning: Attempting to unlink an empty linked list");
		sysObfuscatedLinkedDataNode<T>* target = head;
		// If there is only 1 node, just set head to NULL
		if (length == 1)
		{
			head = NULL;
		}
		else
		{
			// Unlink target node and set head to next node in list
			target->m_pPrev->m_pNext = target->m_pNext;
			target->m_pNext->m_pPrev = target->m_pPrev;
			head = target->m_pNext;
		}
		delete(target);
		--length;
	}
}