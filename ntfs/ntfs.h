// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the NTFS_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// NTFS_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef NTFS_EXPORTS
#define NTFS_API __declspec(dllexport)
#else
#define NTFS_API extern "C" __declspec(dllimport)
#endif

#define FIND_MASK_FILENAME 1
#define FIND_MASK_SIZEFROM 2
#define FIND_MASK_SIZETO 4
#define FIND_MASK_MODIFIEDFROM 8
#define FIND_MASK_MODIFIEDTO 16

#include <wchar.h>

struct FIND_PARAMS {
    int mask;
	char *device;
    wchar_t filename[256];
	wchar_t rootDir[256];
    unsigned long long sizeFrom;
    unsigned long long sizeTo;
    unsigned long long modifiedFrom;
    unsigned long long modifiedTo;
};

struct FIND_RESULT {
	wchar_t filename[256];
	unsigned long long lastModifiedTime;
	unsigned long long fileSize;
	unsigned long long fileIndex;
};

NTFS_API void FindStart(struct FIND_PARAMS *_findParams, long long *filesCount);
NTFS_API int FindNext(struct FIND_RESULT *findResult);
NTFS_API void FindGetPath(long long fileIndex, wchar_t *buffer);
NTFS_API int FindFilter(long long fileIndex);
NTFS_API void NtfsFindClose();
