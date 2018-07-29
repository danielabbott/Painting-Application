#pragma once

#include <vector>
#include <string>

std::vector<unsigned char> load_file(std::string filepath);
int load_file_to(std::string filepath, void * data, unsigned int dataSize);

void saveFile(std::string filepath, const void * data, unsigned int dataSize);
