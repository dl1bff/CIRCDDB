
#if !defined(_IRCRECEIVER_H)
#define _IRCRECEIVER_H


class IRCReceiver : public wxThreadHelper
{
  public:

  IRCReceiver();

  ~IRCReceiver();


  bool startWork();

  void stopWork();


  protected:

  virtual wxThread::ExitCode Entry();



  private:


  bool terminateThread;

};


#endif 
