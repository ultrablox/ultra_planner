
#include "data_file.h"

void onWriteComplete(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
{
	cout << "Write completed" << std::endl;
}
