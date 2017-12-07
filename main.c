#include <stdio.h>
#include <windows.h>
#include "HashMap.h"




static int hash(char *key){
    int h = 5381;
    for (int i = 0; ; i++) {
        if(*(key + i) == '\0'){
            break;
        }

        h = (h << 5) + h +  *(key + i) ;
    }

    return h;
}

static bool equal(char *key1, char *key2) {
    return strcmp(key1, key2) == 0;
}


char* cut(char *str, char *cut){
    char * add = strstr(str, cut);
    if(add == NULL){
        return str;
    } else {
        unsigned int i = add - str;
        char * new_str = malloc(sizeof(char) * (i + 1));
        strncpy(new_str, str, i);
        *(new_str + i) = '\0';
        return new_str;
    }
}


void find(char * lpPath)
{
    char szFind[MAX_PATH], szFile[MAX_PATH];
    WIN32_FIND_DATA FindFileData;
    strcpy(szFind, lpPath);
    strcat(szFind, "/*");
    HANDLE hFind= FindFirstFile(szFind, &FindFileData);
    if(INVALID_HANDLE_VALUE == hFind){
        return;
    } else {
        HashMap  hashMap = createHashMap(64, &hash, &equal);
        while(TRUE) {
            if(FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
            {
                if(FindFileData.cFileName[0]!='.')
                {
                    strcpy(szFile,lpPath);
                    strcat(szFile,"");
                    strcat(szFile,FindFileData.cFileName);
                    find(szFile);
                }
            }
            else
            {

                fprintf(stdout, "%s\n", FindFileData.cFileName);

				//获取核心名称
                char *keyFileName = cut(cut(FindFileData.cFileName, "."), " (");

                WIN32_FIND_DATA *value = getFromHashMap(hashMap, keyFileName);

                if(value == NULL){
					//复制到新的结构体，放进map
                    WIN32_FIND_DATA currentFindFileData;
                    currentFindFileData.nFileSizeLow = FindFileData.nFileSizeLow;
                    strcpy(currentFindFileData.cFileName, FindFileData.cFileName);
                    putIntoHashMap(hashMap, keyFileName, &currentFindFileData);

                    fprintf(stdout, "put %s size %u\n", currentFindFileData.cFileName, currentFindFileData.nFileSizeLow);
                } else {

                    WIN32_FIND_DATA more;
                    WIN32_FIND_DATA less;
                    //删除小的
                    if((*value).nFileSizeLow > FindFileData.nFileSizeLow){
                        more = *value;
                        less = FindFileData;
                    } else {
                        less = *value;
                        more = FindFileData;
                    }

                    char * name2Delete = malloc(strlen(lpPath) + strlen(less.cFileName));
                    strcpy(name2Delete, lpPath);
                    strcat(name2Delete, "/");
                    strcat(name2Delete, less.cFileName);
                    DeleteFile(name2Delete);

                    fprintf(stdout, "del %s size %u\n", less.cFileName, less.nFileSizeLow);

					//大的复制到新的结构体，放进map
                    WIN32_FIND_DATA currentFindFileData;
                    currentFindFileData.nFileSizeLow = more.nFileSizeLow;
                    strcpy(currentFindFileData.cFileName, more.cFileName);
                    putIntoHashMap(hashMap, keyFileName, &currentFindFileData);

                    fprintf(stdout, "put %s size %u\n", currentFindFileData.cFileName, currentFindFileData.nFileSizeLow);
                }

                fflush(stdout);
            }

           
            if(!FindNextFile(hFind,&FindFileData)){
                break;
            }
        }
		                    
		//重命名

		unsigned int size = sizeOfHashMap(hashMap);
		HashMapKVPair p = listPairsOfHashMap(hashMap);

		for (unsigned int i = 0; i < size; i++){
			WIN32_FIND_DATA more;
			more = (WIN32_FIND_DATA*)*(*(p + i).value);

			char * suffix = strchr(more.cFileName, '.');
			char * renameb = malloc(strlen(lpPath) + 1 + strlen(more.cFileName));
			strcpy(renameb, lpPath);
			strcat(renameb, "/");
			strcat(renameb, more.cFileName);

			//获取核心名称
			char *keyFileName = cut(cut(FindFileData.cFileName, "."), " (");

			char * rename2 = malloc(strlen(lpPath) + 1 + strlen(keyFileName) + strlen(suffix));
			strcpy(rename2, lpPath);
			strcat(rename2, "/");
			strcat(rename2, keyFileName);
			strcat(rename2, suffix);

			MoveFile(renameb, rename2);

			fprintf(stdout, "ren %s size %u\n", more.cFileName, more.nFileSizeLow);
		}

			


        FindClose(hFind);
    }


}
void main()
{
    char filepath[MAX_PATH]="/var/MusicDR";
    find(filepath);
    system("PAUSE");
}