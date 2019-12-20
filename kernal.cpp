#include<iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
int my_time  = 1;
int count;
using namespace std;
struct mesg_buffer { 
    long mesg_type; 
    char mesg_text[64];
    int slot_number; 
};

void alarmHandler(int signum);
void sigChildHandler(int signum);
void sigUsr1Handler(int signum);
void sigUsr2Handler(int signum);


void readDataFromProceses(key_t msgqidUpProcesses,mesg_buffer &mgs );
//flag 1-> UP hard disk, 2->down hard disk
key_t createQueue(int flag);//flag values 0-> processes UP stream
int getHardDiskFreeSlots(key_t msgqidHardUP);

void sendRequestToHardDisk(key_t msgqidHardDown,mesg_buffer &msg);

int main(){
    
    pid_t pid = getpid();

    int n = 3;
    count = n;

    //create queuse
    key_t msgqidUpProcesses = createQueue(0);//proceses
    key_t msgqidHardUP = createQueue(1);//Up stream hard
    key_t msgqidHardDown = createQueue(2);//down stream Hard



    //create N processes and hard disk
    for (int i = 0; i < n+1; i++)
    {
        pid = fork();
        if (pid == 0 && i<n)
        {
	    cout<<"Kernal : new Process instantiated"<<endl;
            execl("process.out","process.out",NULL);

            
        }
        if (pid == 0 && i == n)
        {
            execl("Disk.out","Disk.out",NULL);
    
        }
        
    }
    //create msg queue
    mesg_buffer message;
    signal(SIGALRM,alarmHandler);
    signal(SIGCHLD,sigChildHandler);
   
signal(SIGUSR1,sigUsr1Handler);
signal(SIGUSR2,sigUsr2Handler);

    alarm(1);
    int deskTime = 0;
    while (count>0)
    {
	//cout<<deskTime<<endl;
        msgrcv(msgqidUpProcesses, &message, sizeof(mesg_buffer)-sizeof(long), 0, IPC_NOWAIT);
	
	//cout<<" out of read data " <<endl;
        if (deskTime <= 0)
        {
	    if(message.mesg_type < 3)
            	cout<<"Kernal : Message recieved Type = "<<message.mesg_type<<" , Message = "<<message.mesg_text<<endl;
            if (message.mesg_type == 1)//add request
            {
                kill(pid,SIGUSR1);
                int slots = getHardDiskFreeSlots(msgqidHardUP);
                if (slots > 0)//if free slots availabe send add request
                {
                    sendRequestToHardDisk(msgqidHardDown,message);
                    deskTime = 3;
                }
            }
            else if (message.mesg_type == 2)//delete request
            {
		cout<<"d5lna hna"<<endl;
                sendRequestToHardDisk(msgqidHardDown,message);
                deskTime = 1;
            }
            
        }
    }
    

  msgctl(msgqidUpProcesses, IPC_RMID, NULL); 
  

    
    cout<<"parent died"<<endl;


    return 0;
}


key_t createQueue(int flag){
    key_t msgqid, key;
    if(flag == 0){//processes up stream
    key = ftok("OS",53);
    msgqid = msgget(key, 0644 | IPC_CREAT);
    cout << "1st  " << msgqid << endl;
    }
    else if (flag == 1){//hard disk up stream
    key = ftok("UP",22);
    msgqid = msgget(key, 0644 | IPC_CREAT);
    cout << "2nd  " << msgqid << endl;
    }
    else
    {
    key = ftok("DOWN",66);
    msgqid = msgget(key, 0644 | IPC_CREAT);
    cout<< "3rd  "<< msgqid << endl;
    }
    
    return msgqid;

}



void readDataFromProceses(key_t msgqidUpProcesses,mesg_buffer &msg ){
    msgrcv(msgqidUpProcesses, &msg, sizeof(mesg_buffer)-sizeof(long), 0, !IPC_NOWAIT); 
 
}


int getHardDiskFreeSlots(key_t msgqidHardUP){
    mesg_buffer temp;
    msgrcv(msgqidHardUP, &temp, sizeof(mesg_buffer)-sizeof(long), 0, !IPC_NOWAIT);
	cout<<"Kernal : Hard message type ="<<temp.mesg_type<<" and number of free slots "<<temp.slot_number<<endl;
    return temp.slot_number;
}

void sendRequestToHardDisk(key_t msgqidHardDown,mesg_buffer &msg){
  int sendVal = msgsnd( msgqidHardDown , &msg , sizeof(mesg_buffer)-sizeof(long) , 0 );
  if(sendVal == -1)
  	perror("Errror in send");
  else 
    cout<<"Kernal : Sending mesg to hard with type = " << msg.mesg_type << " messgae text = " << msg.mesg_text<<endl;
}

void alarmHandler(int signum){
    	signal(SIGALRM,alarmHandler);
	my_time++;    
	cout<<"Kernal : Clock = "<<my_time<<endl;
    	alarm(1);
    	killpg(getpgrp(), SIGUSR2);
}

void sigChildHandler(int signum){
     int pid, stat_loc;

 printf("Child has sent a SIGCHLD signal #%d\n",signum);
    bool exit = false;
    while (! exit)
    {
        pid = waitpid(-1, &stat_loc, 0);
        if(!(stat_loc & 0x00FF)){
            exit = false;
            count --;
        }
        else
        {
            exit = true;
        }
        
    }
    signal(SIGCHLD,sigChildHandler);
}

void sigUsr1Handler(int signum){}
void sigUsr2Handler(int signum){}
