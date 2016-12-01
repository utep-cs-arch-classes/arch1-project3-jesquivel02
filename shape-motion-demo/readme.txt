Jonathan Esquivel
Project 3 Pong
Arch1 3432


  This program serves to create a game of Pong that is playable by two players. (Albeit with difficulty becaause the switches are close to each other) The program responds to the 4 switch inputs, switch 0 and 1 control the left paddle, and switches 2 and 3 control the right paddle. The ball will bounce off the paddles and increase it's x-axis speed after every hit. After every three hits off the paddles, the speed will increase further in the y axis. 

To compile program:
  $make load
  
Testing:
  Testing can be done by pressing the different switches on the msp430 board
  
To delete binaries:
  $make clean
  
Contributions:
  The demo code given to us was a great help in creating this project, whether the access to the shape layers, or the more intricate timing issues, the demo code was really useful. Another source of information was www-classes.usc.edu/engr/ee-s/477p/s00/pong.html which described what was neccessary for a game of Pong and gave me some ideas about how big the Pong paddles should be. 
  Jose Perez was a great help in getting the switches from p2swLib to be understood and work. Also helped with explaining how the switches should be grouped so the paddles could move correctly. 
  Abner Palomino was a great help in understanding how to get the ball to check if it is in contact with the paddles and reversing the directions. 
