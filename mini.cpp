#include<iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

using namespace std;

struct mesg_buffer { 
    long mesg_type; 
    char mesg_text[100]; 
}; 

int main(int argc, char *argv[]){
    cout<<"I'm Created"<<endl;
    //cout<<argv[0]<<argv[1]<<endl;
    //char *path = "./OS";
    int id = 'O';
    key_t key = ftok("OS",55);
    cout<<key<<endl;
    key_t msgqid;
    msgqid = msgget(key, 0644 | IPC_CREAT);

        mesg_buffer message; 
    message.mesg_type = 1; 
    char str[] = "\nMessage from process to kernal: I am drowning, help meeee...!!\n";
    strcpy(message.mesg_text, str);

    
  
    // msgsnd to send message 
    //msgsnd(msgqid, &message, sizeof(message), 0); 
  
    // display the message 
    //printf("Data send is : %s \n", message.mesg_text);




    return 0;
}

