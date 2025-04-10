# Dali led
 Implementation of a system control device using dali protocol with raspberry pi 5

 ## Proposed Architecture
 ![image](https://github.com/user-attachments/assets/164b5ef6-0ecb-4ff5-8abb-877b778b2a1a)
 - User will input address of the led and its dimming level using the GUI written in C++, then the code will be send to raspberry pi 5 using tcp protocol.
- At the User Space application that is server.c will take the command and write it to the character device at kernel space kernel module.
- after receive the message, the kernel model will handle the code, encode the message using Manchester encoding and controls the gpio to send out the encoded signal.
- Then the ESP32 will read the signal and then decode and control the lights.
- There is also a flask web app but that's not the main idea so i didn't include it in this.

 ## Test
 On your linux device
 ```bash
 git clone https://github.com/JohnnysGithubAccount/Dali-led.git
 cd RPi3-DALI
#  edit the kernel path in the Makefile file.
make # run this to create binary files 
 ```
 Then use scp to send the .ko file to your single board computer. Then you can insert the kernel module (.ko file) you just sent to the board
 
 Now you can run the server for listening controls code.

 ```bash
 cd client_app
 sudo ./a.out # run this command will run the server
 ```

For more details you can have a look how this project runs in video here.
[Watch the video here](https://youtu.be/I2O_JX2ZY2I)

