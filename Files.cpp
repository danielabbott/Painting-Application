#include <Files.h>
#include <fstream>
#include <streambuf>
#include <iterator>
#include <stdexcept>

using namespace std;

int load_file_to(string filepath, void * data, unsigned int dataSize)
{
	ifstream file(filepath, ios::binary);

	if(!file.is_open()) {
		/* File failed to open */
		return 0;
	}

	file.read((char *)data, dataSize);

	return (int)file.gcount();
}

vector<unsigned char> load_file(string filepath)
{
	/* Open File */

	ifstream file(filepath, ios::binary);


	if(!file.is_open()) {
		/* File failed to open */

		throw runtime_error("Error opening file");
	}

	/* Get size of file */

	streampos fileSize;

	file.seekg(0, ios::end);
	fileSize = file.tellg();
	file.seekg(0, ios::beg);
	
	/* Load file into a std::vector */

	vector<unsigned char> vec(fileSize);

	file.read((char *)vec.data(), fileSize);

	return move(vec);
}

void saveFile(string filepath, const void * data, unsigned int dataSize)
{
	ofstream file (filepath, ios::out | ios::binary);
	if(file.is_open()) {
		file.write ((const char *)data, dataSize);
		file.close();
	}
}