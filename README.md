# Dali led
 Implementation of a system control device using dali protocol with raspberry pi 5

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

For more details you can have a look how this project runs in "Quick demo - Made with Clipchamp.mp4" video I included in the reposistory.



