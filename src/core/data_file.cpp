
#include "data_file_win.h"

void onWriteComplete(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
{
	cout << "Write completed" << std::endl;
}
