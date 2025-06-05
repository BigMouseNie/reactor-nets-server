class RectorImpl
{
    
}

class SubReactor

// handler
class EventHandler
{
    virtual Handle get_handle() const = 0;
    virtual void handle_read() = 0;
    virtual void handle_write() = 0;
    virtual void handle_error() = 0;
}

class ListenHandler : public EventHandler
{
    Handle listenfd;
    bool listen_on(uint16_t port);
}

class SocketHandler : piclic EventHandler
{
    Handle sockfd;
    char* buf;
}


// Demultiplexer
class RectorImplementation
{}

class EventDemultiplexer
{}

class EpollDemultiplexer : public EventDemultiplexer
{}

class SelectDemultiplexer : public EventDemultiplexer
{}
