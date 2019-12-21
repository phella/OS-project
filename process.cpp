#include <istream>
#include <fstream>
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sstream>
#include <iostream>
#include <signal.h> 
#include <unistd.h>
#include <bits/stdc++.h>



using namespace std;

unsigned int clk = 1;
void string_copy(char* dest , string src);

struct operation{
	int time;
	bool oper ;
	string message;
    int slot_number = -1;
	operation(int t , bool o , string msg){
		time = t;
		oper = o;
		message = msg;
        if(o){
            stringstream temp(msg); 
            temp >> slot_number;

        }
	}
};

struct mesg_buf {
    mesg_buf(long mtype , string mtext , int snum){
        mesg_type = mtype;
        strcpy(mesg_text,mtext.c_str());
        // mesg_text = mtext;
        slot_number = snum;
		
    }
    long mesg_type; /* type of message */
    char mesg_text[64]; /* message text */
    int slot_number;
};


void inc_clk(int signum);
vector<operation> schedule;


int main(){
    signal (SIGUSR2 , inc_clk);
    key_t key = ftok("OS",53);
    key_t msgqid =  msgget ( key, IPC_CREAT );
    cout<<msgqid<<endl;
	ifstream myfile ("input.txt");
	int time;
        int counter = 0;
	string temp , msg ;
	if( myfile.is_open()){
		while(myfile>>time){
			string total_msg = "";
			myfile >> temp;
			myfile >> msg ;
			int l = int(msg[0]);
			if(msg[0] != '\"' || (msg[0] == '\"' && msg[msg.length()-1] == '\"')){
				total_msg = msg;
			} else {
				total_msg += msg;
				myfile>>msg;
				while(msg[msg.length()-1] != '\"'){
					total_msg += ' ' + msg;
					myfile>>msg;
				}
				total_msg += ' ' + msg;
			}
			bool op = true;
			if(temp == "ADD"){
				op = false;
			}
			operation entry(time , op , total_msg);
			schedule.push_back(entry);
			//cout<<"Process: "<<total_msg<<endl;
		}
	}

    while(counter < schedule.size() ){
        while(schedule[counter].time != clk ){
            // sleep(1);
        }
	mesg_buf new_message((long)schedule[counter].oper + 1 , schedule[counter].message , schedule[counter].slot_number );
    int x = msgsnd( msgqid , &new_message , sizeof(mesg_buf)-sizeof(long) , !IPC_NOWAIT );
	cout<<"Process : Message sent with type "<<new_message.mesg_type<<" , "<<new_message.mesg_text <<endl;
	counter++;
        sleep(1);
    }
	return 0;
}

void inc_clk(int signum){
	 cout<<"Procces: clk event"<<endl;
    	signal (SIGUSR2 , inc_clk);
    	clk++;
}

void string_copy(char* dest , string src) {
    for(int i =0 ;i<src.length() ; i++){
        dest[i] = src[i];
    }

}
