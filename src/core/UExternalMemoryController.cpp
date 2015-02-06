
#include "UExternalMemoryController.h"
#include <stxxl.h>

using namespace std;

UExternalMemoryController::UExternalMemoryController(const std::string & ignored_drives, const std::string & subdir)
{
	/*//Detect HDDs, create STXXL data in each
	vector<char> drive_letters;
	DWORD drives_bitmask  = GetLogicalDrives();
	for (int i=0; i < 32; i++)
	{
		DWORD mask_index = 1 << i;
		if ((drives_bitmask & mask_index) == mask_index)
		{
			char drive_letter = 'A' + i;
			drive_letters.push_back(drive_letter);
		}
	}
	*/


	// get uninitialized config singleton
	stxxl::config * cfg = stxxl::config::get_instance();
	// create a disk_config structure.
	//

#ifdef WIN32
	//std::string drive_letters[] = { "C:/temp"};
	std::string drive_letters[] = { "D:"};
	//std::string drive_letters[] = { "D:", "F:", "G:" };
#elif defined(__APPLE__)
	std::string drive_letters[] = {"/Volumes/HDD/Users/ultrablox/Projects/ultra_planner/bin/osx"};
#endif
	for (auto drive : drive_letters)
	{
		for (int i = 0; i < 4; ++i)
		{
			size_t disc_space_size = 8ULL * 1024 * 1024 * 1024;
		#ifdef WIN32
			stxxl::disk_config * p_disk1 = new stxxl::disk_config(drive + "/stxxl_" + to_string(i) + ".tmp", disc_space_size, "wincall direct=try");
		#elif defined(__APPLE__)
			stxxl::disk_config * p_disk1 = new stxxl::disk_config(drive + "/stxxl_" + to_string(i) + ".tmp", disc_space_size, "syscall direct=try");
		#endif	

			// add disk to config
			cfg->add_disk(*p_disk1);
		}
	}
}
