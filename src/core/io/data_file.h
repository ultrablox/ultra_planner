
#if (defined(WIN32) || defined(WIN64))
	#include "data_file_win.h"
#else
	#include "data_file_unix.h"
#endif