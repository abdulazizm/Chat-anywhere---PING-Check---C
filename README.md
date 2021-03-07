# udp_chat_between_two_devices_over_network
Author: Abdul Aziz
Mail ID: abdulaziz.m.eie@gmail.com
Country: India

### Directory structure:
- src ____ First_User.c
	- |__ Second_User.c
- release ____ first.exe
	- __ second.exe
		 
### Hints:<br />
---Run first.exe in one cmd terminal and run second.exe in other PC/Terminal------<br /> 
1. recv_message func runs in separate thread that prints the received message if any
2. main parent thread will be waiting for user input
3. if user is typing -> message received in between -> message displayed -> cursor moves -> user no need to type message again -> data can be sent correctly
4. User1 and User2 are the same program but with reversed send and recv ports - can have this as input from user but not now
5. I have tested the app by running two process (First and Second) in the same PC but with two different IP.
6. I used cygwin environment installed in windows 8 and compiled with gcc and run terminal by double clicking cygwin64.bat file
7. I have also attached a video with this souce code that may help you to rebuild if needed.
8. sending and receiving messages (ie, updating global buffer) are mutually exclusive and also running in separate threads
9. also works with inputs with spaces
10. when user provides more input than BUFFER_MAX_LEN (1024 char) -> fgets helps to strip the overflowed data

### Demo:
https://github.com/abdulazizm/udp_chat_between_two_devices_over_network/blob/master/Demo_Video.mp4

### Updates:
Check User Provided Input for IPv4 address format

### Video Demonstrated with IPs: 
- 192.168.1.5 - First
- 192.168.1.10 - Second

### netstat -a -b used to get ports binded by the executables.

## Kindly recompile source code if you want to try (recommended). Not sure about executables will work as it depends on cygwin environment.
