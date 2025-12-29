#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
int main(){
pid_t childpid= fork();

if(childpid>0){
        printf("the parent process id = %d , and the zombie child id will be = %d \n ",getpid(),childpid);
        printf("parent alive, zombie child created . check with ps aux| grep (the daemon process) \n ");
        for(int i =0 ;i<=30;i++){
            printf("parent is alive for %d seconds \n ", 30-i);
            sleep(1);}
}
else{
        printf("child will now exit process and will continue to exist (as an entry) to be reaped by the parent using wait() (still a zombie)");
        exit(0);
}
return 0;
}
