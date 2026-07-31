#include "Signal.hpp"

volatile sig_atomic_t g_stop = 0;

void signalHandler(int signum)
{
    (void)signum;
    g_stop = 1;
}

void setupSignals()
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGPIPE, SIG_IGN);
}