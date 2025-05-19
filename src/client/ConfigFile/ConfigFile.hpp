#pragma once

#include "../Options/Options.hpp"
#include "../../Chess/Utils/ChessHelperFunctions.hpp"
#include <iostream>
#include <fstream>	

enum CFG_FILE_RET{
	FOUND,
	NOTFOUND,
	TAMPERED,
	FOUND_BUT_EMPTY
};

void init_config_load();
void config_file_options();
