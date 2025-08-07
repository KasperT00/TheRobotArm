# TheRobotArm
This project includes the 3D-Models, code and other files to create and run a mostly 3D-Printed robot arm. This 6 axis arm is capable to lift at least 600grams and uses Nema 17 and Nema 23 motors to control.

The Robot uses slip ring planetary gearboxes between joints and has ratio of ~1:20. This repository also includes the spreadsheet file to fully calculate proper slip ring planteary gearbox
## Electrical components
Here are all the main electrical components that this project includes
- The motors are optimized based on their required lifting weight and position in the arm. Smaller motors are selected for joint further away from the base. 
    - Motors chosen for this build are Nema 17 and Nema 23 as these offer enough power for the arm as well as they are standardized.
- Motherboard was chosen based on what was available at the time. Other boards could be used as well such as Arduino Mega 2560 or similar by only changing the pin layout.

### Spesific components
- Motors
    - Base And Shoulder - Nema 23: E Series Bipolar 1.8deg 1.85Nm(261.98oz.in) 2.8A 57x57x76mm 4 Wires
    - Elbow - Nema 17: Bipolar 59Ncm(83.55oz.in) 2A 42x42x48mm 4 Wires w/ 1m Cable & Connector
    - Wrist - Nema 17: E Series Bipolar 42Ncm(59.49oz.in) 1.5A 42x42x38mm 4 Wires w/ 1m Cable & Connector
    - Wrist spin - Nema 17: E Series Bipolar 26Ncm(36.82oz.in) 1.2A 42X42x30mm 4 Wires w/ 1m Cable & Connector
- Control board
    - STM32F407VE 
    - Bluetooth HC05 or similar
- Gripper
    - 15KG 180angle 5v servo from Aliexpress
- Drivers
    - TB6600 Stepper Motor Drivers
- Power upply
    - 24v 10A Meanwell
    - 5V Power for motherboard and servo

## 3D-Parts and body
The Robot arm consists of multiple 3d printed parts as well as from 40mm pipe that connects the joints. Pipe was chosen to reduce the amount of 3d printing required but after designing it this way, 3d printing all parts and especially from joint to joint fully from PLA would be better decision.

For each motor there is also 3D-Printed Slip-Ring-Planetary-Gearbox that has a ratio of ~1:20. More about this in the next chapter

## Planetary gearbox
For this project i wanted to use planetary gearboxes, but traditional ones do not achieve large anough ration. Thus so non-common planetary gearbox design was used. This is not the most efficient gearbox as it requires a lot of printing as well as it has quite much friction, so getting the input shaft spinning by turning the output ring if nearly impossible but works well from input to output. 

The planetary gearbox consist of 2 stages with 4 palents where both planets for each stage is connected. Sun gears are separate and ring gears are separate. 

In the gearbox is also airsoft bnb's as bearings. These bearings are 5.95mm. 

There are two sizes: One for Nema 23 motors and one for Nema 17 motors. So total of 5 gearboxes required, which equals to rougly 1 to 1.5 rolls of filament. 

## Code 
The code is written using Arduino IDE with basic functionailities. It includes the pinlayout, homing, different commands for each motor and enabling or disabling the motors. User can send commands and unti 0 is received, the motors are moved. Once 0 is received, the motors stop. This way user can move multiple motors at the same time. To send commands, some basic android application is used and commands are sent through bluetooth. The code also includes two way communication to bluetooth and back, so it can send log informtion to the bluetooth using its serial. 


