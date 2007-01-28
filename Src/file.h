/*
 *
 * This file is taken from the ARAnyM project which builds a new and powerful
 * TOS/FreeMiNT compatible virtual machine running on almost any hardware.
 *
 * This file is distributed under the GNU Public License, version 2 or at
 * your option any later version. Read the file gpl.txt for details.
 *
 */

#ifndef HATARI_FILE_H
#define HATARI_FILE_H

#include "sysdeps.h"

#define MAX_FILENAME_LENGTH 260

/* File types */
enum {
  FILEFILTER_DISCFILES,
  FILEFILTER_ALLFILES,
  FILEFILTER_TOSROM,
  FILEFILTER_MAPFILE,
  FILEFILTER_YMFILE,
  FILEFILTER_MEMORYFILE,
};

#if defined(__BEOS__) || defined(OS_solaris) || defined(OS_mingw)
#include <dirent.h>
extern int alphasort(const void *d1, const void *d2);
extern int scandir(const char *dirname, struct dirent ***namelist, int(*select) (const struct dirent *), int (*dcomp) (const void *, const void *));
#endif  /* __BEOS__ */

extern void File_CleanFileName(char *pszFileName);
extern void File_AddSlashToEndFileName(char *pszFileName);
extern bool File_DoesFileExtensionMatch(char *pszFileName, char *pszExtension);
extern bool File_IsRootFileName(char *pszFileName);
extern char *File_RemoveFileNameDrive(char *pszFileName);
extern bool File_DoesFileNameEndWithSlash(char *pszFileName);
extern void File_RemoveFileNameTrailingSlashes(char *pszFileName);
extern bool File_FileNameIsMSA(char *pszFileName);
extern bool File_FileNameIsST(char *pszFileName);
extern void *File_Read(char *pszFileName, void *pAddress, long *pFileSize, char *ppszExts[]);
extern bool File_Save(char *pszFileName, void *pAddress,long Size,bool bQueryOverwrite);
extern int File_Length(char *pszFileName);
extern bool File_Exists(const char *pszFileName);
extern bool File_Delete(char *pszFileName);
extern bool File_QueryOverwrite(char *pszFileName);
extern bool File_FindPossibleExtFileName(char *pszFileName,char *ppszExts[]);
extern void File_splitpath(char *pSrcFileName, char *pDir, char *pName, char *Ext);
extern void File_makepath(char *pDestFileName, char *pDir, char *pName, char *pExt);
extern void File_ShrinkName(char *pDestFileName, char *pSrcFileName, int maxlen);

#endif /* HATARI_FILE_H */
