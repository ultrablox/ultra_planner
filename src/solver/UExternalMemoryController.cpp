
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

#ifndef __APPLE__
	// get uninitialized config singleton
	stxxl::config * cfg = stxxl::config::get_instance();
	// create a disk_config structure.
	//std::string drive_letters[] = { "D:", "F:", "G:" };
	std::string drive_letters[] = { "D:"};

	for (auto drive : drive_letters)
	{
		for (int i = 0; i < 1; ++i)
		{
			size_t disc_space_size = 8ULL * 1024 * 1024 * 1024;
			stxxl::disk_config * p_disk1 = new stxxl::disk_config(drive + "/stxxl_" + to_string(i) + ".tmp", disc_space_size, "wincall direct=try");
			//p_disk1->direct = stxxl::disk_config::DIRECT_TRY; // force O_DIRECT
			//p_disk1->unlink_on_open = false;

			// add disk to config
			cfg->add_disk(*p_disk1);
		}
	}
#endif
}
