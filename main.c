#include <stdio.h>
#include <windows.h>
#include "HashMap.h"




static int hash(void *keyVoid){
    char * key = keyVoid;
    int h = 5381;
    for (int i = 0; ; i++) {
        if(*(key + i) == '\0'){
            break;
        }

        h = (h << 5) + h +  *(key + i) ;
    }

    return h;
}

static bool equal(void *key1Void, void *key2Void) {
    char * key1 = key1Void;
    char * key2 = key2Void;

    return strcmp(key1, key2) == 0;
}


char* cut(char *str, char *cut){
    char * add = strstr(str, cut);
    if(add == NULL){
        return str;
    } else {
        unsigned long i = add - str;
        char * new_str = calloc(i + 1, sizeof(char));
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
        HashMap  hashMap = CreateHashMap(64, &hash, &equal);
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

				//��ȡ��������
                char *keyFileName = cut(cut(FindFileData.cFileName, "."), " (");

                WIN32_FIND_DATA *value = GetHashMap(hashMap, keyFileName);

                if(value == NULL){
					//���Ƶ��µĽṹ�壬�Ž�map
                    WIN32_FIND_DATA *currentFindFileData = calloc(1, sizeof(WIN32_FIND_DATA));
                    currentFindFileData->nFileSizeLow = FindFileData.nFileSizeLow;
                    strcpy(currentFindFileData->cFileName, FindFileData.cFileName);
                    PutHashMap(hashMap, keyFileName, currentFindFileData);

                    fprintf(stdout, "put %s size %u\n", currentFindFileData->cFileName, currentFindFileData->nFileSizeLow);
                } else {

                    WIN32_FIND_DATA more;
                    WIN32_FIND_DATA less;
                    //ɾ��С��
                    if((*value).nFileSizeLow > FindFileData.nFileSizeLow){
                        more = *value;
                        less = FindFileData;
                    } else {
                        less = *value;
                        more = FindFileData;
                    }

                    char * name2Delete = calloc(strlen(lpPath) + strlen(less.cFileName), sizeof(char));
                    strcpy(name2Delete, lpPath);
                    strcat(name2Delete, "/");
                    strcat(name2Delete, less.cFileName);
                    DeleteFile(name2Delete);

                    fprintf(stdout, "del %s size %u\n", less.cFileName, less.nFileSizeLow);

					//��ĸ��Ƶ��µĽṹ�壬�Ž�map
                    WIN32_FIND_DATA *currentFindFileData = calloc(1, sizeof(WIN32_FIND_DATA));
                    currentFindFileData->nFileSizeLow = more.nFileSizeLow;
                    strcpy(currentFindFileData->cFileName, more.cFileName);
                    PutHashMap(hashMap, keyFileName, currentFindFileData);

                    fprintf(stdout, "put %s size %u\n", currentFindFileData->cFileName, currentFindFileData->nFileSizeLow);

                    //free(name2Delete);
                    free(value);
                }

                fflush(stdout);
            }

           
            if(!FindNextFile(hFind,&FindFileData)){
                break;
            }
        }
		                    
		//������

		unsigned int size = SizeHashMap(hashMap);
        KvPairHashMap *p = PairListHashMap(hashMap);

		for (int i = 0; i < size; i++){
			WIN32_FIND_DATA more;
            WIN32_FIND_DATA * moreP = (p + i)->value;
            more = *moreP;

			char * suffix = strchr(more.cFileName, '.');
            int    suffixLen;
            if(suffix == NULL){
                suffixLen = 0;
            } else {
                suffixLen = strlen(suffix);
            }
			char * renameb = calloc(strlen(lpPath) + 1 + strlen(more.cFileName), sizeof(char));
			strcpy(renameb, lpPath);
			strcat(renameb, "/");
			strcat(renameb, more.cFileName);

			//��ȡ��������
			char *keyFileName = cut(cut(more.cFileName, "."), " (");

			char * rename2 = calloc(strlen(lpPath) + 1 + strlen(keyFileName) + suffixLen, sizeof(char));
			strcpy(rename2, lpPath);
			strcat(rename2, "/");
			strcat(rename2, keyFileName);
            if(suffixLen > 0){
                strcat(rename2, suffix);
            }


            if(equal(renameb, rename2)){
                continue;
            }

			MoveFile(renameb, rename2);

			fprintf(stdout, "rename %s >>> %s\n", renameb, rename2);
//free���쳣��ԭ����Ų�
//            free(renameb);
//            free(rename2);
            free(moreP);
		}
        fflush(stdout);
			

        free(p);
        DestroyHashMap(hashMap);
        FindClose(hFind);
    }


}
void main()
{
    fprintf(stdout, "����������Ŀ¼\n");
    fflush(stdout);
    char filepath[MAX_PATH];
    scanf("%s", filepath);
    find(filepath);
    system("PAUSE");
}