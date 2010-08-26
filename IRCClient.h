
#if !defined(_IRCCLIENT_H)
#define _IRCCLIENT_H

#include "IRCReceiver.h"


class IRCClient : public wxThreadHelper
{
  public:

  IRCClient( const wxString& hostName, unsigned int port, const wxString& callsign, const wxString& password );

  ~IRCClient();


  bool startWork();

  void stopWork();


  protected:

  virtual wxThread::ExitCode Entry();



  private:

  char host_name[100];
  unsigned int port;
  wxString callsign;
  wxString password;

  bool terminateThread;

  IRCReceiver * recv;

};







#endif 
