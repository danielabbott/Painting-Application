#include <cstdio>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <cassert>
#include <string>

using namespace std;

static bool soLoaded = false;


typedef bool (*f_WebPGetInfo)(const uint8_t* data, size_t data_size, int* width, int* height);
static f_WebPGetInfo getInfo;

typedef uint8_t * (*f_WebPDecodeRGBAInto)(const uint8_t* data, size_t data_size, uint8_t* output_buffer, int output_buffer_size, int output_stride);
f_WebPDecodeRGBAInto decodeRGBA;


#ifdef _WIN32
#include <Windows.h>

string webpLibraryFile = "webp.dll";

static inline void load_so()
{
	HINSTANCE dll = LoadLibrary(webpLibraryFile.c_str());
	if(!so) {
		dll = LoadLibrary("webp.dll", RTLD_LAZY);
		if(!dll) {
			throw runtime_error("'" + webpLibraryFile + "' could not be loaded"); 
		}
	}

	getInfo = (f_WebPGetInfo)GetProcAddress(dll, "WebPGetInfo");
	decodeRGBA = (f_WebPDecodeRGBAInto)GetProcAddress(dll, "WebPDecodeRGBAInto");

	if(!getInfo || !decodeRGBA) {
		throw runtime_error("WebP Functions(s) not found in webp.dll");
	}

	soLoaded = true;
}
#endif

#ifdef __linux__
#include <dlfcn.h>

string webpLibraryFile = "libwebp.so";

static inline void load_so()
{
	void * so = dlopen(webpLibraryFile.c_str(), RTLD_LAZY);
	if(!so) {
		so = dlopen("libwebp.so", RTLD_LAZY);
		if(!so) {
			throw runtime_error("'" + webpLibraryFile + "' could not be loaded"); 
		}
	}

	getInfo = (f_WebPGetInfo)dlsym(so, "WebPGetInfo");
	decodeRGBA = (f_WebPDecodeRGBAInto)dlsym(so, "WebPDecodeRGBAInto");

	if(!getInfo || !decodeRGBA) {
		throw runtime_error("WebP Functions(s) not found in libwebp.so");
	}

	soLoaded = true;
}
#endif

std::vector<unsigned char> load_webp(FILE * file, unsigned int & width, unsigned int & height, bool & wasWebp)
{
	assert(file);

	wasWebp = false;

	if(!soLoaded) {
		load_so();
	}

	fseek(file, 0, SEEK_END);
	long fsize = ftell(file);
	rewind(file);

	uint8_t * rawData = new uint8_t[fsize];
	int read = fread(rawData, fsize, 1, file);
	if(read != 1) {
		delete[] rawData;
		throw runtime_error("IO error reading .webp file");
	}


	if(!getInfo(rawData, fsize, (int *)&width, (int *)&height))
	{
		throw runtime_error("Not a .webp file");
	}

	wasWebp = true;

	vector<uint8_t> data(width*height*4);
	if(!decodeRGBA(rawData, fsize, data.data(), data.size(), width*4))
	{
		throw runtime_error("Error decoding .webp image");
	}


	return data;
}