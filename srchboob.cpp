//#pragma comment (lib,"c:\\PROGRA~1\\Java\\jdk1.5.0_01\\lib\\jvm.lib")

//#include <jni.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
//#include "bookmain.h"       //20230125 bookhash
#include "bookhash.h"  	//20230125
//#define JNI_VERSION_1_2 0x00010002
//#define JNI_VERSION_1_4 0x00010004

/*
JNIEnv* create_vm() {
	JavaVM* jvm;
	JNIEnv* env;
	JavaVMInitArgs args;
	JavaVMOption options[1];

	// There is a new JNI_VERSION_1_4, but it doesn't add anything for the purposes of our example.
	args.version = JNI_VERSION_1_2;
	args.nOptions = 1;
	//options[0].optionString = "-Djava.class.path=c:\\projects\\local\\inonit\\classes";
	options[0].optionString = "-Djava.class.path=.";
	args.options = options;
	args.ignoreUnrecognized = JNI_FALSE;

	jint status = JNI_CreateJavaVM(&jvm, (void **)&env, &args);
	if (status < 0)
   	{  fprintf(stderr,"%s%d \n","Error creating VM, status=",status);
       		return 0;
   	}
	return env;
}
*/

#define CB_EMPTY 90
int srchboob(unsigned char eboardfrom[], int compMoveFirst)
//int srchboob(unsigned char eboardfrom[], int compMoveFirst)
{
    char bookstr[3];  // incl '\0'
    unsigned char cboard[32];
    char bookargstr[67]; // last 2 chr is compMoveFirst followed by '\0'
    int bookmove;

//    2,3, 4,5,6,7, 8,9,10,11, 12,13,14,15, 16,17,18,19, 20,21,22,23, 24,25,26,27, 28,29,30,31, 32,33
//    p p  p p p p  p p p  p   b  b  b  b   e  e  e  e   n  n  n  n   c  c  c  c   r  r  r  r   k  k
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5      16 7 8 9 20 1 2 3 4 5 6 7 8 9 0 31
//    B,E,H,C,R,P,P,K,P,P,P,R,C,H,E,B,    //B,E,H,C, R,P,P,P,K,P,P,R,C,H,E,B
    cboard[ 8]=eboardfrom[ 2];
    cboard[23]=eboardfrom[ 3];
    cboard[ 5]=eboardfrom[ 4];
    cboard[26]=eboardfrom[ 5];
    cboard[10]=eboardfrom[ 6];
    cboard[21]=eboardfrom[ 7];
    cboard[ 6]=eboardfrom[ 8];
    cboard[25]=eboardfrom[ 9];
    cboard[ 9]=eboardfrom[10];
    cboard[22]=eboardfrom[11];

    cboard[ 0]=eboardfrom[12];
    cboard[31]=eboardfrom[13];
    cboard[15]=eboardfrom[14];
    cboard[16]=eboardfrom[15];
    cboard[ 1]=eboardfrom[16];
    cboard[30]=eboardfrom[17];
    cboard[14]=eboardfrom[18];
    cboard[17]=eboardfrom[19];
    cboard[ 2]=eboardfrom[20];
    cboard[29]=eboardfrom[21];
    cboard[13]=eboardfrom[22];
    cboard[18]=eboardfrom[23];
    cboard[ 3]=eboardfrom[24];
    cboard[28]=eboardfrom[25];
    cboard[12]=eboardfrom[26];
    cboard[19]=eboardfrom[27];
    cboard[ 4]=eboardfrom[28];
    cboard[27]=eboardfrom[29];
    cboard[11]=eboardfrom[30];
    cboard[20]=eboardfrom[31];

    cboard[ 7]=eboardfrom[32];
    cboard[24]=eboardfrom[33];

    //sprintf (str, "%c%d%c%d",
    //(from % 9) + 'a', 9 - (from / 9),
    //(dest % 9) + 'a', 9 - (dest / 9)
    //);
//20190912    if (cboard[0] <0) cboard[0]=CB_EMPTY;
    	
    	
    //sprintf(bookargstr, "%02d", cboard[0]);
    sprintf(bookargstr, "%02d", cboard[0]);
    for (int i=1; i<32; i++)
    {
//20190912        if (cboard[i] <0) cboard[i]=CB_EMPTY;
        //sprintf(bookstr,    "%02d", cboard[i]);
        sprintf(bookstr, "%02d", cboard[i]);
        //strcat(bookargstr, bookstr);
        strcat(bookargstr, bookstr);
    }

    //sprintf(bookstr, "%02d", compMoveFirst);
    sprintf(bookstr, "%02d", compMoveFirst);
    //strcat(bookargstr, bookstr);
    strcat(bookargstr, bookstr);
    // debug
//    printf("bookargstr[64]=%d, [65]=%d, [66]=%d\n", bookargstr[64],bookargstr[65],bookargstr[66]);
//    printf("bookargstr=%s len=%d \n", bookargstr, strlen(bookargstr));

//20190912    std::string strbookargstr = bookargstr;
//    fprintf(stderr,"strbookargstr=%s len=%d \n", strbookargstr, strbookargstr.length() );	
//20230125 - bookhash for bookmain
    //bookmove = bookmain(bookargstr);  //argstr= 03....8901\0
		bookmove = bookhash(bookargstr);  //argstr= 03....8901\0
    return bookmove;

}



