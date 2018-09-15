#include <cstdint>
#include <stdexcept>
#include <cassert>
#include <string>
#include <iostream>
#include <ImageFile.h>

using namespace std;

static bool soLoaded = false;


typedef bool (*f_WebPGetInfo)(const uint8_t* data, size_t data_size, int* width, int* height);
static f_WebPGetInfo getInfo;

typedef uint8_t * (*f_WebPDecodeRGBAInto)(const uint8_t* data, size_t data_size, uint8_t* output_buffer, int output_buffer_size, int output_stride);
f_WebPDecodeRGBAInto decodeRGBAInto;


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
	decodeRGBAInto = (f_WebPDecodeRGBAInto)GetProcAddress(dll, "WebPDecodeRGBAInto");

	if(!getInfo || !decodeRGBAInto) {
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
	decodeRGBAInto = (f_WebPDecodeRGBAInto)dlsym(so, "WebPDecodeRGBAInto");

	if(!getInfo || !decodeRGBAInto) {
		throw runtime_error("WebP Functions(s) not found in libwebp.so");
	}

	soLoaded = true;
}
#endif

std::vector<unsigned char> load_webp(ifstream && file, unsigned int & width, unsigned int & height, bool & wasWebp)
{
	wasWebp = false;

	if(!soLoaded) {
		load_so();
	}

	file.seekg(0, file.end);
	long fsize = file.tellg();
	file.seekg(0);

	uint8_t * rawData = new uint8_t[fsize];
	file.read((char *)rawData, fsize);

	if(!getInfo(rawData, fsize, (int *)&width, (int *)&height))
	{
		throw runtime_error("Not a .webp file");
	}

	wasWebp = true;

	clog << "Loading .webp. Width: " << width << ", height: " << height << ", file size: " << fsize << endl;

	vector<uint8_t> data(width*height*4);
	if(!decodeRGBAInto(rawData, fsize, data.data(), data.size(), width*4))
	{
		throw runtime_error("Error decoding .webp image");
	}


	return data;
}