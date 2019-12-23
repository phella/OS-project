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
#include<fstream>
using namespace std;


int my_time  = 1;
int count;
int deskTime = 0;

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

    int n = 2;
    count = n;

    //create queuse
    key_t msgqidUpProcesses = createQueue(0);//proceses
    key_t msgqidHardUP = createQueue(1);//Up stream hard
    key_t msgqidHardDown = createQueue(2);//down stream Hard
    ofstream log;
    log.open("log.txt");


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
    
    signal(SIGALRM,alarmHandler);
    signal(SIGCHLD,sigChildHandler);
   
signal(SIGUSR1,sigUsr1Handler);
signal(SIGUSR2,sigUsr2Handler);
bool haveMsg = false;
    alarm(1);
    mesg_buffer message;
    bool allprocessDied; 
    allprocessDied= false;
    bool logged = false;
    while ((count > 0) || (!allprocessDied))
    {
        
        
        if (count <= 0 && !logged)
        {
            log<<endl;
            log<<"All Processes died at "<<my_time<<endl;
            logged = true;
        }
        
        if (! haveMsg)
        {
            int recv = msgrcv(msgqidUpProcesses, &message, sizeof(mesg_buffer)-sizeof(long), 0, IPC_NOWAIT);
            if (recv != -1)
            {
                haveMsg = true;
                //cout<<"Kernal: recieved valid msg"<<endl;
                //cout<<"Kernal : Message recieved Type = "<<message.mesg_type<<" , Message = "<<message.mesg_text<<endl;
            }
            else if(count <= 0){
                allprocessDied = true;
            }
        }
        
	//cout<<deskTime<<endl;
        	
	//cout<<" out of read data " <<endl;
        if (deskTime <= 0 && haveMsg)
        {
            

            if (message.mesg_type == 1)//add request
            {
                log<<endl;
                log<<"Processes Requet:  add msg "<<message.mesg_text <<"  at time: "<<my_time<<endl;
                kill(pid,SIGUSR1);
                log<<"Hard Request : number of free slots ? "<<"  at time: "<<my_time<<endl;
                int slots = getHardDiskFreeSlots(msgqidHardUP);
                log<<"Hard Response :  "<<slots<<"  at time: "<<my_time<<endl;;
                if (slots > 0)//if free slots availabe send add request
                {
                    sendRequestToHardDisk(msgqidHardDown,message);
                    deskTime = 3;
                    haveMsg = false;
                }
                else
                {
                    log<<"Kernal Out of space ignore last add Request at time : "<<my_time<<endl;
                    haveMsg = false;
                }
                
            }
            else if (message.mesg_type == 2)//delete request
            {
                log<<endl;
		        log<<"Process Request: delete msg with slot =  "<<message.slot_number<<" at time: "<<my_time<<endl;
                sendRequestToHardDisk(msgqidHardDown,message);
                deskTime = 1;
                haveMsg = false;
            }
            
        }
    }

   while (deskTime >0)
   {
       //do wala 7aga
   }
   log<<endl;
   log<<"All requests are done at "<<my_time<<endl;
   kill(pid,SIGKILL);
    

  msgctl(msgqidUpProcesses, IPC_RMID, NULL);
   msgctl(msgqidHardDown, IPC_RMID, NULL);
   msgctl(msgqidHardUP, IPC_RMID, NULL);
  
    log<<endl;
    log<<"kill hard, delete msg-queues and die at "<<my_time<<endl;
    //cout<<"parent died"<<endl;

    log.close();
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
	//cout<<"Kernal : Hard message type ="<<temp.mesg_type<<" and number of free slots "<<temp.slot_number<<endl;
    return temp.slot_number;
}

void sendRequestToHardDisk(key_t msgqidHardDown,mesg_buffer &msg){
  int sendVal = msgsnd( msgqidHardDown , &msg , sizeof(mesg_buffer)-sizeof(long) , 0 );
  if(sendVal == -1)
  	perror("Errror in send");
  //else 
    //cout<<"Kernal : Sending mesg to hard with type = " << msg.mesg_type << " messgae text = " << msg.mesg_text<<endl;
}

void alarmHandler(int signum){
    	signal(SIGALRM,alarmHandler);
	my_time++; 
    deskTime--;
    //cout<<endl;
    //cout<<"Kernal : Clock = "<<my_time<<endl<<endl;
    //cout<<"Kernal desktime: "<<deskTime<<endl;
    //cout<<"KERNAL: count=  "<<count<<endl;
    	alarm(1);
    	killpg(getpgrp(), SIGUSR2);

 }
    
    


void sigChildHandler(int signum){
     int pid, stat_loc;

 //cout<<"Child has sent a SIGCHLD signal"<<endl;
    int status;
    while (waitpid(-1,&status,WNOHANG) > 0)
    {
        count--;
    }
    

}

void sigUsr1Handler(int signum){}
void sigUsr2Handler(int signum){}
