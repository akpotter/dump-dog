Includes:

1. Crash Dump generator process

2. Exception handling (You plug this into your existing project). When a crash occurs the watchdog process is spawned and analyzes your own process.
IPC is done via named pipes.

3. Minidump Reader - reads a crash dump and outputs a summary text file.
Uses the dbgeng.dll, so the console is basically a very basic windbg.

4. Crash Server (Python) - Receives zipped crash dumps in a specific directory.
Analyzes them and updates an HTML page with summary.
To be added: Crash clustering - offered grouping of crashes.

5. Crash client (Python) - Compresses client side crash dumps and sends them to the server.