#include "../common/version.h"
#include "fun_json.h"

void fun_json(void){
    char *json_string=NULL;
    const cJSON *root = NULL;
    void *ptr1 = NULL;
    char *p;
	int64_t *dest=NULL;
    MMFLOAT tempd;
    int i,j,k,mode,index;
    char field[32],num[6];
    getargs(&ep, 3, ",");
    char *a=GetTempStrMemory();
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if(vartbl[VarIndex].type & T_INT) {
    if(vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
    if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
        error("Argument 1 must be integer array");
    }
    dest = (long long int *)ptr1;
    json_string=(char *)&dest[1];
    } else error("Argument 1 must be integer array");
    cJSON * parse = cJSON_Parse(json_string);
    if(parse==NULL)error("Invalid JSON data");
    root=parse;
    p=getCstring(argv[2]);
    int len = strlen(p);
    memset(field,0,32);
    memset(num,0,6);
    i=0;j=0;k=0;mode=0;
    while(i<len){
        if(p[i]=='['){ //start of index
            mode=1;
            field[j]=0;
            root = cJSON_GetObjectItemCaseSensitive(root, field);
            memset(field,0,32);
            j=0;
        }
        if(p[i]==']'){
            num[k]=0;
            index=atoi(num);
            root = cJSON_GetArrayItem(root, index);
            memset(num,0,6);
            k=0;
        }
        if(p[i]=='.'){ //new field separator
            if(mode==0){
                field[j]=0;
                root = cJSON_GetObjectItemCaseSensitive(root, field);
             memset(field,0,32);
                j=0;
            } else { //step past the dot after a close bracket
                mode=0;
            }
        } else  {
            if(mode==0)field[j++]=p[i];
            else if(p[i]!='[')num[k++]=p[i];
        }
        i++;
    }
    root = cJSON_GetObjectItem(root, field);

    if (cJSON_IsObject(root)){
        cJSON_Delete(parse);
        error("Not an item");
        return;
    }
    if (cJSON_IsInvalid(root)){
        cJSON_Delete(parse);
        error("Not an item");
        return;
    }
    if (cJSON_IsNumber(root))
    {
        tempd = root->valuedouble;

        if((MMFLOAT)((int64_t)tempd)==tempd) IntToStr(a,(int64_t)tempd,10);
        else FloatToStr(a, tempd, 0, STR_AUTO_PRECISION, ' ');   // set the string value to be saved
        cJSON_Delete(parse);
        sret=a;
        sret=CtoM(sret);
        targ=T_STR;
        return;
    }
    if (cJSON_IsBool(root)){
        int64_t tempint;
        tempint=root->valueint;
        cJSON_Delete(parse);
        if(tempint)strcpy(sret,"true");
        else strcpy(sret,"false");
        sret=CtoM(sret);
        targ=T_STR;
        return;
    }
    if (cJSON_IsString(root)){
        strcpy(a,root->valuestring);
        cJSON_Delete(parse);
        sret=a;
        sret=CtoM(sret);
        targ=T_STR;
        return;
    }
}
