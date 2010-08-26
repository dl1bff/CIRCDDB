
#define	WSAAPI			FAR PASCAL
#define	WS2TCPIP_INLINE	__inline

typedef struct addrinfo {
	int                 ai_flags;       // AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST
	int                 ai_family;      // PF_xxx
	int                 ai_socktype;    // SOCK_xxx
	int                 ai_protocol;    // 0 or IPPROTO_xxx for IPv4 and IPv6
	size_t              ai_addrlen;     // Length of ai_addr
	char*               ai_canonname;   // Canonical name for nodename
	struct sockaddr*    ai_addr;        // Binary address
	struct addrinfo*    ai_next;        // Next structure in linked list
} ADDRINFOA, *PADDRINFOA;

extern "C" {
int WSAAPI getaddrinfo(
	__in_opt        PCSTR               pNodeName,
	__in_opt        PCSTR               pServiceName,
	__in_opt        const ADDRINFOA *   pHints,
	__deref_out     PADDRINFOA *        ppResult
);

void WSAAPI freeaddrinfo(__in struct addrinfo *ai);

#ifdef UNICODE
#define	gai_strerror	gai_strerrorW
#else
#define	gai_strerror	gai_strerrorA
#endif  /* UNICODE */

// WARNING: The gai_strerror inline functions below use static buffers,
// and hence are not thread-safe.  We'll use buffers long enough to hold
// 1k characters.  Any system error messages longer than this will be
// returned as empty strings.  However 1k should work for the error codes
// used by getaddrinfo().
#define GAI_STRERROR_BUFFER_SIZE 1024

WS2TCPIP_INLINE char* gai_strerrorA(IN int ecode)
{
	static char buff[GAI_STRERROR_BUFFER_SIZE + 1];

	DWORD dwMsgLen = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM
	                             |FORMAT_MESSAGE_IGNORE_INSERTS
	                             |FORMAT_MESSAGE_MAX_WIDTH_MASK,
	                              NULL,
	                              ecode,
	                              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	                              (LPSTR)buff,
	                              GAI_STRERROR_BUFFER_SIZE,
	                              NULL);

	return buff;
}

WS2TCPIP_INLINE WCHAR * gai_strerrorW(IN int ecode)
{
	static WCHAR buff[GAI_STRERROR_BUFFER_SIZE + 1];

	DWORD dwMsgLen = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM
	                             |FORMAT_MESSAGE_IGNORE_INSERTS
	                             |FORMAT_MESSAGE_MAX_WIDTH_MASK,
	                              NULL,
	                              ecode,
	                              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	                              (LPWSTR)buff,
	                              GAI_STRERROR_BUFFER_SIZE,
	                              NULL);

	return buff;
}
}

