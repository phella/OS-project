#include<iostream>
#include<fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <signal.h>
#include <vector>
#define SLOTS_NUMBER 10
#define CHARS 64

using namespace std;

struct mesg_buffer { 
    long mesg_type; 
    char mesg_text[CHARS];
    int slot_number; 
};

char slots[SLOTS_NUMBER][CHARS];                              //disk slots
long long CLK = 1;                               //clock
int freeSlots = SLOTS_NUMBER;                    //free slots number initialized to 10
key_t upstream;
key_t downstream;


void user2Handler(int signum);                   //increments CLK
void user1Handler(int signum);                   //kernel sends a message
void Add(char*);                                 //Adds to disk
void Delete(int);                                //deletes from disk
void Send_Status();                              //sends number of freeslots to kernel

int main(void)
{
  ofstream disk_out;
  disk_out.open("disk_out.txt");
  for(int i = 0; i < SLOTS_NUMBER; i++)           //initialize the disk to be empty
  {
    strcpy(slots[i], " ");
  }

  //create upstream message queue
  key_t key_up = ftok("DOWN",66);
  upstream = msgget(key_up,  IPC_CREAT); // or msgget(12613, IPC_CREATE | 0644)

  //create downstream message queue
  key_t key_down = ftok("UP",22);
  downstream = msgget(key_down,  IPC_CREAT); // or msgget(12613, IPC_CREATE | 0644)

  //set signal handlers
  signal (SIGUSR2, user2Handler);
  signal (SIGUSR1, user1Handler);

  int rec_val;
  mesg_buffer message;
  while(1) 
  {
    rec_val = msgrcv(upstream, &message, sizeof(mesg_buffer)-sizeof(long), 0, IPC_NOWAIT); 
 
      if(message.mesg_type == 1){
        disk_out<<"Hard : Added new message with text" << message.mesg_text <<endl;
        Add(message.mesg_text);
        message.mesg_type = -1;
        for(int i = 0; i < SLOTS_NUMBER; i++) {
          disk_out<<"slot "<<i<<" : "<<slots[i]<<endl;
        }
        disk_out<<"free slots : "<<freeSlots<<endl;
        disk_out<<endl;
      }
      else if(message.mesg_type == 2){
        disk_out<<"Hard : delete request slot "<< message.slot_number << endl;
        Delete(message.slot_number);
        message.mesg_type = -1;
        for(int i = 0; i < SLOTS_NUMBER; i++) {
          disk_out<<"slot "<<i<<" : "<<slots[i]<<endl;
        }
        disk_out<<"free slots : "<<freeSlots<<endl;
        disk_out<<endl;
      }
    
  }
  disk_out.close();
  return 0;
}

void user2Handler(int signum)
{
  signal (SIGUSR2, user2Handler);
  cout<<"Disk : recieved SIGUSER 2"<<endl;
  CLK++;   
}

void user1Handler(int signum)
{
   signal (SIGUSR1, user1Handler);
   cout<<"Disk : recieved SIGUSER 1"<<endl;
   Send_Status();   
}

void Add(char* msg)
{
  cout<<"Disk: free slots before add:" <<freeSlots<<endl;
  for(int i = 0; i < SLOTS_NUMBER; i++) 
  {
    if(strcmp(slots[i]," ") == 0)
    {
      freeSlots--;
      strcpy(slots[i], msg);
      break;
    }
  }
cout<<"Disk: free slots after add:" <<freeSlots<<endl;
}

void Delete(int n)
{
  if(n >= 0 and n < 10)
  {
    if(strcmp(slots[n], " ") != 0) {
      freeSlots++;
      strcpy(slots[n], " ");
    }
  }
}

void Send_Status()
{
  int sendVal;

  mesg_buffer message;
  message.mesg_type = 1;
  message.slot_number = freeSlots;
  strcpy(message.mesg_text," philo ");
  sendVal = msgsnd(downstream, &message, sizeof(mesg_buffer)-sizeof(long), !IPC_NOWAIT);
  if(sendVal == -1)
  	perror("Errror in send");
  else 
	  cout<<"Disk: Sent Messgae of type "<< message.mesg_type <<" number of free slots = "<<message.slot_number<<endl;
}
