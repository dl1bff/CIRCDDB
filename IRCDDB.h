
#if !defined(_IRCDDB_H)
#define _IRCDDB_H



enum IRCDDB_RESPONSE_TYPE {
	IDRT_NONE,
	IDRT_USER,
	IDRT_GATEWAY
};

enum DSTAR_PROTOCOL {
	DP_UNKNOWN,
	DP_DEXTRA,
	DP_DPLUS
};

class CIRCDDBPrivate;

class CIRCDDB {
public:
	CIRCDDB(const wxString& hostName, unsigned int port, const wxString& callsign, const wxString& password);
	~CIRCDDB();

	// A false return implies a network error, or unable to log in
	bool open();

	// The following three functions don't block waiting for a reply, they just send the data

	// Send heard data, a false return implies a network error
	bool sendHeard(const wxString& userCallsign, const wxString& repeaterCallsign);

	// Send query for a gateway/reflector, a false return implies a network error
	bool findGateway(const wxString& gatewayCallsign);

	// Send query for a user, a false return implies a network error
	bool findUser(const wxString& userCallsign);

	// The following functions are for processing received messages
	
	// Get the waiting message type
	IRCDDB_RESPONSE_TYPE getMessageType();

	// Get a gateway message, as a result of IDRT_GATEWAY returned from getMessageType()
	// A false return implies a network error
	bool receiveGateway(wxString& gatewayCallsign, wxIPV4address& address, DSTAR_PROTOCOL& protocol);

	// Get a user message, as a result of IDRT_USER returned from getMessageType()
	// A false return implies a network error
	bool receiveUser(wxString& userCallsign, wxString& repeaterCallsign, wxIPV4address& address);

	void close();		// Implictely kills any threads in the IRC code

private:
	CIRCDDBPrivate * const d;

};

#endif  // _IRCDDB_H

