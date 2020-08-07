/***************************************************************************
* File:        main.c
* Author:      Venkata Bapanapalli
* Procedures:
*   handler  - handles the signal the parent process receives.
*   handler2 - handels the signal each child process receives.
*   main     - Creation of 5 child processes. Plays hot potato with all
               child processes. 
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <mqueue.h>
int count, max = 5;
pid_t pid[5];
int i, j, status = 5;
mqd_t msgq;
/***************************************************************************
* int handler
* Author:       Venkata Bapanapalli
* Date:         1 March 2020
* Description:  Handler that handles the signals that the parent processes
                receive. Sends a singal to all child process.
*
* Parameters:
* sig		I/P	int	Holds the SIGUSR1 value
**************************************************************************/
void handler(int sig)                                      //handles the signal the parent process receives.
{
    status--;                                              //decrement the number of child processes left in the game.
    for(j = 0; j < max; j++)                               //sends a signal to all child processes still alive.
    {
        kill(pid[j], SIGUSR2);
    }
}
/***************************************************************************
* int handler2
* Author:       Venkata Bapanapalli
* Date:         1 March 2020
* Description:  Handler that handles the signals that the child processes
                receive. Prints the child process Id and current count and
                resets the count of each process to 0.
*
* Parameters:
*  sig		I/P	int	Holds the SIGUSR2 value
**************************************************************************/
void handler2(int sig)                                      //handles the signal each child receives.
{
    printf("Process: %d Count: %i\n",getpid(), count);      //prints the child process id and count.
    count = 0;                                              //resets the count of this process to 0.
}
/***************************************************************************
* int main( int argc, char *argv[] )
* Author:       Venkata Bapanapalli
* Date:         1 March 2020
* Description:  The parent process creates 5 child processes using a for
                loop. The child processes will then play hot potato, the
                first process to receive the message 1000 times will be
                a loser and the game will continue with the remaning
                processes, until only one process remains. The last process
                is the winner. When a child process loses it will send a 
                signal to the parent and the parent will send a singal to 
                all children, asking them to print their current position
                and reset their counter to 0.
*
* Parameters:
* argc I/P int The number of arguments on the command line
* argv I/P char *[] The arguments on the command line
* main O/P int Status code (not currently used)
**************************************************************************/
int main(int argc, char *argv[])                          //the main program.
{
    char msg[] = "Potato";                                //the hot potato message.
    char rcvmsg[7];                                       //char array to hold the received message.
    for(i = 0; i < 5; i++)                                //using for loops to create 5 child processes.
    {
        if((pid[i] = fork()) < 0)                         //checking is fork failed.
        {
            perror("Fork fail\n");                        //prints an error in case process creation has failed.
        }
        if(pid[i] == 0)                                   //creates a child process.
        {
            signal(SIGUSR2, handler2);                    //signal interrupt.
            while(count != 1000)                          //as long as count is lower than 1000 the process will continue to receive and send messages from and to the queue, respectively.
            {
                mq_open("vtb160030", O_RDWR);             //opens the queue with read and write permissions.
                mq_receive(msgq, rcvmsg, 6 , NULL);       //receives message if it exists on the queue.
                mq_send(msgq, msg, 6, 0);                 //sends the message back to the queue for the next process to pick up.
                mq_close(msgq);                           //the queue is closed.
                count++;                                  //increment count.
            }
            printf("Process: %d finished.\n", getpid());  //print the process has finished statement.
            kill(getppid(), SIGUSR1);                     //send a signal to parent.
            wait(NULL);                                   //wait for process to end.
            exit(0);                                      //end process.
        }
    }
    signal(SIGUSR1, handler);                             //singal handler.
    printf("Parent ID: %d\n",getpid());                   //print parent ID.
    msgq = mq_open("vtb160030", O_CREAT | O_RDWR);        //opened message queue and creates it if it was not already created.
    mq_send(msgq, msg, 7, 0);                             //send a message on the queue (this is the first time that a message in put on the queue no child has access until this is sent).
    mq_close(msgq);                                       //closes the queue.
    mq_unlink("vtb160030");                               //unlinks the queue so parent no longer has access.
    while(status)                                         //waits for only one child to remain in the game.
    {
        if(status == 1)                                   //if only 1 process is in the game, end loop.
        {
            break;
        }
    }
    sleep(3);                                             //sleep for 3 seconds.
    printf("The last process is the winner!\n");          //declare winner.
    return 0;                                             //finish program.
}
