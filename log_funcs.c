#include <stdio.h>

void CallFuncLog(char* calleeName, char* callerName, long int valID){
    printf("Function '%s' called '%s' {Func_id: %ld}\n", calleeName, callerName, valID);
}
