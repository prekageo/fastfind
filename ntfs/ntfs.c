// ntfs.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ntfs.h"

// TODO: faster search
// TODO: do search on a subdirectory
// TODO: efficient list management
// TODO: efficient path retrieval
// TODO: globbing operators on filename matching
// TODO: case insensitive filenames
// TODO: on/off to the spinner

#include <stdio.h>
#include <string.h>
#include "utils.h"
#include "dir.h"

struct FIND_PARAMS findParams;
unsigned long long rootDirInode;
struct mft_search_ctx *m_ctx = NULL;
struct FIND_DATA {
	long long parent;
	wchar_t filename[256];
} *findData;

#define ELEMENTS(x) (sizeof((x))/sizeof(*(x)))

int inode_get_name(ntfs_inode *inode, wchar_t *buffer, int bufsize, leMFT_REF *parent) {
	ntfs_attr_search_ctx *ctx;
	ATTR_RECORD *rec;
	FILE_NAME_ATTR *attr;
	int name_space;

	if (!inode || !buffer) {
		errno = EINVAL;
		return 0;
	}

	ctx = ntfs_attr_get_search_ctx(inode, NULL);
	if (!ctx) {
		ntfs_log_error("Couldn't create a search context.\n");
		return 0;
	}

	name_space = 4;
	while ((rec = find_attribute(AT_FILE_NAME, ctx))) {
		/* We know this will always be resident. */
		attr = (FILE_NAME_ATTR *) ((char *) rec + le16_to_cpu(rec->value_offset));

		if (attr->file_name_type > name_space) {
			continue;
		}

		name_space = attr->file_name_type;
		wcsncpy(buffer, attr->file_name, attr->file_name_length);
		buffer[attr->file_name_length]=0;
		*parent = attr->parent_directory;
	}
	ntfs_attr_put_search_ctx(ctx);

	return 1;
}

#define PATH_SEP '\\'
u64 pathname_to_inode_num(ntfs_volume *vol, ntfs_inode *parent,
		const wchar_t *pathname)
{
	u64 inum, result;
	int len, err = 0;
	wchar_t *p, *q;
	ntfs_inode *ni = NULL;
	ntfschar *unicode = NULL;
	wchar_t *ascii = NULL;

	inum = result = (u64)-1;
	if (!vol || !pathname) {
		err = EINVAL;
		goto close;
	}
	ntfs_log_trace("Path: '%s'\n", pathname);
	if (parent) {
		ni = parent;
	} else
		inum = FILE_root;
	//unicode = calloc(1, MAX_PATH);
	ascii = _wcsdup(pathname);
	if (/*!unicode ||*/ !ascii) {
		ntfs_log_error("Out of memory.\n");
		err = ENOMEM;
		goto close;
	}
	p = ascii;
	/* Remove leading /'s. */
	while (p && *p == PATH_SEP)
		p++;
	while (p && *p) {
		if (!ni) {
			ni = ntfs_inode_open(vol, inum);
			if (!ni) {
				ntfs_log_debug("Cannot open inode %llu.\n",
						(unsigned long long)inum);
				err = EIO;
				goto close;
			}
		}
		/* Find the end of the first token. */
		q = wcschr(p, PATH_SEP);
		if (q != NULL) {
			*q = 0;
			q++;
		}
		len = wcslen(p);
		unicode = p;
		//len = ntfs_mbstoucs(p, &unicode, MAX_PATH);
		//if (len < 0) {
		//	ntfs_log_debug("Couldn't convert name to Unicode: "
		//			"%s.\n", p);
		//	err = EILSEQ;
		//	goto close;
		//}
		inum = ntfs_inode_lookup_by_name(ni, unicode, len);
		if (inum == (u64)-1) {
			ntfs_log_debug("Couldn't find name '%s' in pathname "
					"'%s'.\n", p, pathname);
			err = ENOENT;
			goto close;
		}
		inum = MREF(inum);
		if (ni != parent)
			ntfs_inode_close(ni);
		ni = NULL;
		p = q;
		while (p && *p == PATH_SEP)
			p++;
	}
	result = inum;
close:
	if (ni && (ni != parent))
		ntfs_inode_close(ni);
	free(ascii);
	//free(unicode);
	if (err)
		errno = err;
	return result;
}

NTFS_API int FindNext(struct FIND_RESULT *findResult) {
	int count=(m_ctx->vol->mft_na->data_size >> m_ctx->vol->mft_record_size_bits)/100;
    unsigned long long data_size;
    STANDARD_INFORMATION *stdInfo;
    ATTR_RECORD *stdInfoAttr, *dataAttr;
	ntfs_volume *vol;
    
	while (--count && mft_next_record(m_ctx) == 0) {
		if (m_ctx->mft_num < 27) {
			continue;
		}
        if (!(m_ctx->flags_match & FEMR_BASE_RECORD)) {
            continue;
        }
        
        stdInfoAttr = find_first_attribute(AT_STANDARD_INFORMATION, m_ctx->inode->mrec);
        stdInfo = (STANDARD_INFORMATION*)((char*)stdInfoAttr + le16_to_cpu(stdInfoAttr->value_offset));
        dataAttr = find_first_attribute(AT_DATA, m_ctx->inode->mrec);
        inode_get_name(m_ctx->inode, findData[m_ctx->mft_num].filename, ELEMENTS(findData[m_ctx->mft_num].filename), &findData[m_ctx->mft_num].parent);

        if (dataAttr == NULL) {
            data_size = 0;
        } else if (dataAttr->non_resident) {
            data_size = dataAttr->data_size;
        } else {
            data_size = dataAttr->value_length;
        }

        if (findParams.mask & FIND_MASK_FILENAME) {
            if (wcsstr(findData[m_ctx->mft_num].filename, findParams.filename) == 0) continue;
        }
        if (findParams.mask & FIND_MASK_SIZEFROM) {
            if (data_size < findParams.sizeFrom) continue;
        }
        if (findParams.mask & FIND_MASK_SIZETO) {
            if (data_size > findParams.sizeTo) continue;
        }
        if (findParams.mask & FIND_MASK_MODIFIEDFROM) {
            if (stdInfo->last_data_change_time < findParams.modifiedFrom) continue;
        }
        if (findParams.mask & FIND_MASK_MODIFIEDTO) {
            if (stdInfo->last_data_change_time > findParams.modifiedTo) continue;
        }

		wcscpy(findResult->filename, findData[m_ctx->mft_num].filename);
		findResult->lastModifiedTime = stdInfo->last_data_change_time;
		findResult->fileSize = data_size;
		findResult->fileIndex = m_ctx->mft_num;
		return 1;
    }
	findResult->fileIndex = m_ctx->mft_num;
	if (count == 0) {
		return 2;
	}

	vol = m_ctx->vol;
    mft_put_search_ctx(m_ctx);
    ntfs_umount(vol, FALSE);
	return 0;
}

NTFS_API void FindStart(struct FIND_PARAMS *_findParams, long long *filesCount) {
    ntfs_volume *vol = NULL;
    
	memcpy(&findParams, _findParams, sizeof(findParams));
	vol = utils_mount_volume(findParams.device, NTFS_MNT_RDONLY | NTFS_MNT_FORENSIC);
    if (vol == NULL) {
        perror("utils_mount_volume");
        exit(1);
    }

	*filesCount = vol->mft_na->data_size >> vol->mft_record_size_bits;
	findData = malloc(*filesCount * sizeof(*findData));
		
    m_ctx = mft_get_search_ctx(vol);
    m_ctx->flags_search = FEMR_IN_USE | FEMR_BASE_RECORD;

	rootDirInode = pathname_to_inode_num(vol, NULL, findParams.rootDir);
}

NTFS_API void FindGetPath(long long fileIndex, wchar_t *buffer) {
	int stack_idx=0;
	wchar_t *stack[256];
	long long idx = MREF(findData[fileIndex].parent);

	while (idx != FILE_root) {
		stack[stack_idx++]=findData[idx].filename;
		idx = MREF(findData[idx].parent);
	}

	buffer[0]=findParams.device[4];
	buffer[1]=':';
	buffer[2]='\\';
	buffer[3]=0;
	for (stack_idx--;stack_idx>=0;stack_idx--) {
		wcscat(buffer, stack[stack_idx]);
		wcscat(buffer, L"\\");
	}
}

NTFS_API int FindFilter(long long fileIndex) {
	if (rootDirInode == FILE_root) {
		return 1;
	}

	while (fileIndex != FILE_root) {
		if (fileIndex == rootDirInode)
			return 1;
		fileIndex = MREF(findData[fileIndex].parent);
	}

	return 0;
}

NTFS_API void NtfsFindClose() {
	free(findData);
}