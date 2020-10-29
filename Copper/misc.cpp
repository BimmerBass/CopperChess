
#include "stdio.h"
#include "defs.h"

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>
#include <io.h>
#endif



// http://home.arcor.de/dreamlike/chess/
int InputWaiting()
{
#ifndef _WIN32
  fd_set readfds;
  struct timeval tv;
  FD_ZERO (&readfds);
  FD_SET (fileno(stdin), &readfds);
  tv.tv_sec=0; tv.tv_usec=0;
  select(16, &readfds, 0, 0, &tv);

  return (FD_ISSET(fileno(stdin), &readfds));
#else
   static int init = 0, pipe;
   static HANDLE inh;
   DWORD dw;

   if (!init) {
	 init = 1;
	 inh = GetStdHandle(STD_INPUT_HANDLE);
	 pipe = !GetConsoleMode(inh, &dw);
	 if (!pipe) {
		SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT));
		FlushConsoleInputBuffer(inh);
	  }
	}
	if (pipe) {
	  if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
	  return dw;
	} else {
	  GetNumberOfConsoleInputEvents(inh, &dw);
	  return dw <= 1 ? 0 : dw;
	}
#endif
}

void ReadInput(S_SEARCHINFO *info) {
  int             bytes;
  char            input[256] = "", *endc;

	if (InputWaiting()) {
		info->stopped = true;
		do {
#ifndef _WIN32
			bytes=read(fileno(stdin),input,256);
#else
			bytes = _read(_fileno(stdin), input, 256);
#endif
		} while (bytes<0);
		endc = strchr(input,'\n');
		if (endc) *endc=0;

		if (strlen(input) > 0) {
			if (!strncmp(input, "quit", 4))    {
			  info->quit = true;
			}
		}
		return;
	}
}