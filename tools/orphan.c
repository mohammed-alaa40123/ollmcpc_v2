#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>

int main(){

        pid_t child = fork();
        if(child > 0){
                printf("the parent exists , and the value is = %d", getpid());
                printf("parent exists and the child will become an orphan ");
                exit(0);}
        else if(child == 0){
                printf(" the child id =  %d , and the parent id is = %d" ,getpid(),getppid() );
                sleep(5);
                printf(" the child is an orphan with no parent and it is still running");
                sleep(5);
                printf(" still running ...");
                sleep(10);
                printf(" the child is done");

}
return 0;




}
