#include <stdio.h>
#include <windows.h>
#include "HashMap.h"

extern int errno;


/**
 * hash����
 * @param keyVoid
 * @return
 */
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


int cut(char *str, char *cut, char * newStr){
    char * add = strstr(str, cut);
    if(add == NULL){
        return 0;
    } else {
        unsigned long i = add - str;
        strncpy(newStr, str, i);
        *(newStr + i) = '\0';
        return 0;
    }
}


void find(char dir[]) {

    /*����·��*/
    char LookUpPath[MAX_PATH];
    strcpy(LookUpPath, dir);
    strcat(LookUpPath, "/*");

    /*�ҵ����ļ�*/
    WIN32_FIND_DATA FindData;
    WIN32_FIND_DATA * pFindData = &FindData;

    HANDLE hFind= FindFirstFile(LookUpPath, pFindData);

    if(INVALID_HANDLE_VALUE == hFind){
        return;
    } else {
        HashMap  hashMap = CreateHashMap(64, &hash, &equal);

        while(TRUE) {
            if(FindData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
            {
//                if(FindData.cFileName[0] != '.')
//                {
//                    strcpy(szFile,dir);
//                    strcat(szFile,"");
//                    strcat(szFile,FindData.cFileName);
//                    find(szFile);
//                }
            }
            else
            {

                //start with .
                if(FindData.cFileName[0] == '.'){
                    if(!FindNextFile(hFind,&FindData)){
                        break;
                    }
                }

				//��ȡ��������
                char KeyFileName0[MAX_PATH];
                cut(FindData.cFileName, ".", KeyFileName0);
                char KeyFileName[MAX_PATH];
                cut(KeyFileName0, "(", KeyFileName);

                WIN32_FIND_DATA *value = GetHashMap(hashMap, KeyFileName);

                if(value == NULL){
					//���Ƶ��µĽṹ�壬�Ž�map
                    WIN32_FIND_DATA *currentFindFileData = calloc(1, sizeof(WIN32_FIND_DATA));
                    currentFindFileData->nFileSizeLow = FindData.nFileSizeLow;
                    strcpy(currentFindFileData->cFileName, FindData.cFileName);
                    PutHashMap(hashMap, KeyFileName, currentFindFileData);

                    fprintf(stdout, "find %s,%u\n", currentFindFileData->cFileName, currentFindFileData->nFileSizeLow);
                } else {

                    WIN32_FIND_DATA more;
                    WIN32_FIND_DATA less;
                    //ɾ��С��
                    if((*value).nFileSizeLow > FindData.nFileSizeLow){
                        more = *value;
                        less = FindData;
                    } else {
                        less = *value;
                        more = FindData;
                    }

                    fprintf(stdout, "compare %s,%u greater than %s,%u\n", more.cFileName, more.nFileSizeLow, less.cFileName, less.nFileSizeLow);

                    char name2Delete[MAX_PATH];
                    strcpy(name2Delete, dir);
                    strcat(name2Delete, "/");
                    strcat(name2Delete, less.cFileName);
                    DeleteFile(name2Delete);

                    fprintf(stdout, "delete %s,%u\n", less.cFileName, less.nFileSizeLow);

					//��ĸ��Ƶ��µĽṹ�壬�Ž�map
                    WIN32_FIND_DATA *currentFindFileData = calloc(1, sizeof(WIN32_FIND_DATA));
                    currentFindFileData->nFileSizeLow = more.nFileSizeLow;
                    strcpy(currentFindFileData->cFileName, more.cFileName);
                    PutHashMap(hashMap, KeyFileName, currentFindFileData);

                    fprintf(stdout, "keep %s,%u\n", currentFindFileData->cFileName, currentFindFileData->nFileSizeLow);

                    //free(name2Delete);
                    free(value);
                }

                fflush(stdout);
            }

           
            if(!FindNextFile(hFind,&FindData)){
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

            fprintf(stdout, "rename before %s\n", more.cFileName);

			char * suffix = strchr(more.cFileName, '.');
            int    suffixLen;
            if(suffix == NULL){
                suffixLen = 0;
            } else {
                suffixLen = strlen(suffix);
            }

            //fprintf(stdout, "rename suffix %s\n", suffix);

			char renameb[MAX_PATH];
			strcpy(renameb, dir);
			strcat(renameb, "/");
			strcat(renameb, more.cFileName);

            fprintf(stdout, "rename from %s\n", renameb);

            //��ȡ��������
            char KeyFileName0[MAX_PATH];
            cut(FindData.cFileName, ".", KeyFileName0);
            char KeyFileName[MAX_PATH];
            cut(KeyFileName0, "(", KeyFileName);


			char rename2[MAX_PATH];

            strcpy(rename2, dir);
			strcat(rename2, "/");
			strcat(rename2, KeyFileName);
            if(suffixLen > 0){
                strcat(rename2, suffix);
            }

            fprintf(stdout, "rename to %s\n", rename2);

            if(equal(renameb, rename2)){
                fprintf(stdout, "rename ignore %s\n", renameb);
                continue;
            }

			MoveFile(renameb, rename2);

			fprintf(stdout, "rename success %s\n", rename2);
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