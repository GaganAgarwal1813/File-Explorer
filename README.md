# AOS Assignement-1

## Compilation and Running
To Compile- g++ main.cpp 
To Run- ./a.out

## Normal Mode

Vertical and Horizontal Scrolling is used inorder to ensure smooth functioning in small terminal sized 
Horizontal Scrolling can be handled by 'A' and 'S' where 'A' can be used to scroll leftwards and 'S' can be used to scroll rightwards
On dynamically changing the size the content may get distorted initially but on pressing either A or S the screeen will explorer will continue to work fine.
On Pressing Up and Down arrow key the position of the pointer changes accordingly
On Pressing Enter the respective directory and file will open.
On Pressing the left arrow key you will reach to the last visited directory
On pressing the Right arrow key you will reach to the to the last visited directory in the forward direction
On Pressing colon(:) command mode will start where respective commands can be entered
On pressing 'k' and 'l' vertical scrolling is done


## Command Mode

Previously entered commands can be seen as well as hidden depending upon the execution of few lines. By default I have made the command to hide as soon Enter key is pressed for the execution of the command.
In the create_file option some dummy test has already been added which can be removed in case user want to create a fresh file .
On pressing q + Enter the file explorer will quit.
